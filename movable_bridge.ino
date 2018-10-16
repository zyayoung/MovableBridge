#include <Servo.h>
#define DEBUG_LEVEL 2

#define OFF_BRIDGE 0
#define ENTER_FROM_LEFT 1
#define ENTER_FROM_RIGHT 2
#define LEAVING_LEFT 3
#define LEAVING_RIGHT 4

#define SERVO_LEFT 9
#define SERVO_RIGHT 8
#define DISTANCE_LEFT A0
#define DISTANCE_RIGHT A1
#define HC_TRIG 7
#define HC_ECHO 6
#define HCMINOR_TRIG 13
#define HCMINOR_ECHO 12
#define MOTOR_LEFT_A 5
#define MOTOR_RIGHT_A 3
#define MOTOR_LEFT_B 4
#define MOTOR_RIGHT_B 2
#define SWITCH_LEFT A3
#define SWITCH_RIGHT A2
#define LIGHT_RED 11
#define LIGHT_GREEN 10

#define BLOCK_ON_DEGREE 140
#define BLOCK_OFF_DEGREE 40
#define CARC_DISTANCE_MIN 5
#define CARC_DISTANCE_MAX 70

#define DELAY_PER_LOOP 100


Servo s_left;
Servo s_right;

int carA_on_bridge = OFF_BRIDGE;
int carA_is_waiting_left = 0;
int carA_is_waiting_right = 0;
unsigned long carA_off_bridge_timer = 2001;
unsigned long carC_off_bridge_timer = 2001;
int carC_is_waiting = 0;

unsigned long distance_left_trigger_timer = 0;
unsigned long distance_right_trigger_timer = 0;
unsigned long hc_state_changed_timer = 0;
int hc_current_state = 0;

int bridge_raised = 0;

long microsecondsToCentimeters(long microseconds) {
  // cite: https://www.arduino.cc/en/Tutorial/Ping?from=Tutorial.UltrasoundSensor

  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.

  return microseconds / 29 / 2;
}


long hc_read() {
  // cite: https://www.arduino.cc/en/Tutorial/Ping?from=Tutorial.UltrasoundSensor

  // This function return the distance read from ultrasonic senser in cm.

  long duration, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(HC_TRIG, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH pulse
  // whose duration is the time (in microseconds) from the sending of the ping
  // to the reception of its echo off of an object.
  duration = pulseIn(HC_ECHO, HIGH, 10000);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);
  return cm;
}

long hcminor_read() {
  long duration, cm;
  digitalWrite(HCMINOR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(HCMINOR_TRIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(HCMINOR_TRIG, LOW);
  duration = pulseIn(HCMINOR_ECHO, HIGH, 10000);
  cm = microsecondsToCentimeters(duration);
  return cm;
}

bool detected_carc() {
  long distance = hc_read();
  delay(20);
  long distanceminor = hcminor_read();
  delay(20);
  return (CARC_DISTANCE_MIN < distance && distance < CARC_DISTANCE_MAX) || (CARC_DISTANCE_MIN < distanceminor && distanceminor < CARC_DISTANCE_MAX);
}


void setup() {
  // initialize serial communication:

#if DEBUG_LEVEL
  Serial.begin(115200);
#endif

  s_left.attach(SERVO_LEFT);
  s_right.attach(SERVO_RIGHT);
  pinMode(SERVO_RIGHT, OUTPUT);
  pinMode(HC_TRIG, OUTPUT);
  pinMode(HCMINOR_TRIG, OUTPUT);
  pinMode(MOTOR_LEFT_A, OUTPUT);
  pinMode(MOTOR_RIGHT_A, OUTPUT);
  pinMode(MOTOR_LEFT_B, OUTPUT);
  pinMode(MOTOR_RIGHT_B, OUTPUT);
  pinMode(LIGHT_RED, OUTPUT);
  pinMode(LIGHT_GREEN, OUTPUT);
  pinMode(DISTANCE_LEFT, INPUT);
  pinMode(DISTANCE_RIGHT, INPUT);
  pinMode(SWITCH_LEFT, INPUT);
  pinMode(SWITCH_RIGHT, INPUT);
  pinMode(HC_ECHO, INPUT);
  pinMode(HCMINOR_ECHO, INPUT);

  digitalWrite(MOTOR_LEFT_A, LOW);
  digitalWrite(MOTOR_LEFT_B, LOW);
  digitalWrite(MOTOR_RIGHT_A, LOW);
  digitalWrite(MOTOR_RIGHT_B, LOW);
}


void loop() {
  // Update flags

  // Update sensor trigger time for robustness
  if (!digitalRead(DISTANCE_LEFT))distance_left_trigger_timer += DELAY_PER_LOOP;
  else distance_left_trigger_timer = 0;
  if (!digitalRead(DISTANCE_RIGHT))distance_right_trigger_timer += DELAY_PER_LOOP;
  else distance_right_trigger_timer = 0;
  if (detected_carc() ^ hc_current_state)hc_state_changed_timer += DELAY_PER_LOOP;
  else hc_state_changed_timer = 0;

  // Make accurate states of the sensors
  carA_is_waiting_left = (distance_left_trigger_timer > 500) ? 1 : 0;
  carA_is_waiting_right = (distance_right_trigger_timer > 500) ? 1 : 0;
  if (hc_state_changed_timer > 1000) {
    hc_current_state = ! hc_current_state;
  }
  carC_is_waiting = hc_current_state;

  // Predict whether carA is on the bridge
  if (carA_on_bridge == ENTER_FROM_LEFT && carA_is_waiting_right)carA_on_bridge = LEAVING_RIGHT;
  else if (carA_on_bridge == ENTER_FROM_RIGHT && carA_is_waiting_left)carA_on_bridge = LEAVING_LEFT;
  else if (carA_on_bridge == LEAVING_LEFT && !carA_is_waiting_left) {
    carA_on_bridge = OFF_BRIDGE;
    delay(2000);  // easy but not elegant
  }
  else if (carA_on_bridge == LEAVING_RIGHT && !carA_is_waiting_right) {
    carA_on_bridge = OFF_BRIDGE;
    delay(2000);  // easy but not elegant
  }
  else if (carA_on_bridge == OFF_BRIDGE && carA_is_waiting_left && carA_off_bridge_timer > 5000)carA_on_bridge = ENTER_FROM_LEFT;
  else if (carA_on_bridge == OFF_BRIDGE && carA_is_waiting_right && carA_off_bridge_timer > 5000)carA_on_bridge = ENTER_FROM_RIGHT;

  // Update off bridge timer
  carA_off_bridge_timer = (carA_on_bridge) ? 0 : carA_off_bridge_timer + DELAY_PER_LOOP;
  carC_off_bridge_timer = (carC_is_waiting) ? 0 : carC_off_bridge_timer + DELAY_PER_LOOP;


  // Take action

  // Update Traffic Light
  bool redlight = bridge_raised || carC_off_bridge_timer < 2000;
  digitalWrite(LIGHT_RED, redlight);
  digitalWrite(LIGHT_GREEN, !redlight);

  // Update Blocking System
  s_left.write((!carA_is_waiting_left && redlight) ? BLOCK_ON_DEGREE : BLOCK_OFF_DEGREE);
  s_right.write((!carA_is_waiting_right && redlight) ? BLOCK_ON_DEGREE : BLOCK_OFF_DEGREE);

  // Main Operation: Raise or Lower the bridge
  // Notice that we block the loop here to prevent some strange errors
  if (!bridge_raised && carC_is_waiting && carA_off_bridge_timer > 2000) {
    // Raise the bridge ( pray that compiler can optimize the code below

#if DEBUG_LEVEL
    Serial.println("Raiseing the bridge.");
#endif

    // Switch is not used here because we are lazy.

    digitalWrite(MOTOR_LEFT_B, 1);
    digitalWrite(MOTOR_RIGHT_B, 1);
    delay(5200);
    digitalWrite(MOTOR_LEFT_B, 0);
    digitalWrite(MOTOR_RIGHT_B, 0);
    bridge_raised = 1;
  }

  if (bridge_raised && carC_off_bridge_timer > 2000) {
    // Lower the bridge

#if DEBUG_LEVEL
    Serial.println("Lowering the bridge.");
#endif

    digitalWrite(MOTOR_LEFT_A, 1);
    digitalWrite(MOTOR_RIGHT_A, 1);
    delay(4800);
    digitalWrite(MOTOR_LEFT_A, 0);
    digitalWrite(MOTOR_RIGHT_A, 0);
    bridge_raised = 0;
  }


  // Debug
#if DEBUG_LEVEL
  char str[100];
  sprintf(
    str,
    "carA\t|on:%d\t|wl:%d\t|wr:%d\t|t:%lu\t|carC\t|on:%d\t|t:%lu\t|bridgeraised:%d",
    carA_on_bridge,
    carA_is_waiting_left,
    carA_is_waiting_right,
    carA_off_bridge_timer,
    carC_is_waiting,
    carC_off_bridge_timer,
    bridge_raised
  );
  Serial.println(str);
#endif

#if DEBUG_LEVEL == 2
  sprintf(
    str,
    "distance_left_timer:%lu\t|distance_right_timer:%lu\t|hc_state_changed_timer:%lu|hc:%d\t",
    distance_left_trigger_timer,
    distance_right_trigger_timer,
    hc_state_changed_timer,
    hcminor_read()
  );
  Serial.println(str);
#endif


  // loop 1000/DELAY_PER_LOOP times per second
  delay(DELAY_PER_LOOP);
}
