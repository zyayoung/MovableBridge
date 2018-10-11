#include <Servo.h>
#define DEBUG 2

#define OFF_BRIDGE 0
#define ENTER_FROM_LEFT 1
#define ENTER_FROM_RIGHT 2

#define SERVO_LEFT 9
#define SERVO_RIGHT 10
#define DISTANCE_LEFT 11
#define DISTANCE_RIGHT 8
#define HC_TRIG 7
#define HC_ECHO 6
#define MOTOR_LEFT_A 5
#define MOTOR_RIGHT_A 3
#define MOTOR_LEFT_B 4
#define MOTOR_RIGHT_B 2
#define SWITCH_LEFT A3
#define SWITCH_RIGHT A2
#define LIGHT_RED A7
#define LIGHT_GREEN A6

#define BLOCK_ON_DEGREE 90
#define BLOCK_OFF_DEGREE 0
#define CARC_DISTANCE_MIN 30
#define CARC_DISTANCE_MAX 70

#define DELAY_PER_LOOP 50


Servo s_left;
Servo s_right;

int carA_on_bridge = OFF_BRIDGE;
int carA_is_waiting_left = 0;
int carA_is_waiting_right = 0;
int carA_off_bridge_timer = 0;
int carC_off_bridge_timer = 0;
int carC_is_waiting = 0;

int distance_left_trigger_timer = 0;
int distance_right_trigger_timer = 0;
int hc_trigger_timer = 0;

bool bridge_raised = false;

long microsecondsToCentimeters(long microseconds) {
  // cite: https://www.arduino.cc/en/Tutorial/Ping?from=Tutorial.UltrasoundSensor

  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.

  return microseconds / 29 / 2;
}


long hc_read(){
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
  duration = pulseIn(HC_ECHO, HIGH);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);
  return cm;
}

bool detected_carc(){
  long distance = hc_read();
  return CARC_DISTANCE_MIN < distance && distance < CARC_DISTANCE_MAX;
}


void setup() {
  // initialize serial communication:

  #if DEBUG
  Serial.begin(9600);
  #endif

  s_left.attach(SERVO_LEFT);
  s_right.attach(SERVO_RIGHT);
  pinMode(SERVO_RIGHT, OUTPUT);
  pinMode(HC_TRIG, OUTPUT);
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

  digitalWrite(MOTOR_LEFT_A, LOW);
  digitalWrite(MOTOR_LEFT_B, LOW);
  digitalWrite(MOTOR_RIGHT_A, LOW);
  digitalWrite(MOTOR_RIGHT_B, LOW);
}


void loop() {
  // Update flags

  // Update sensor trigger time for robustness
  if(digitalRead(DISTANCE_LEFT))distance_left_trigger_timer += DELAY_PER_LOOP;
  else distance_left_trigger_timer = 0;
  if(digitalRead(DISTANCE_RIGHT))distance_right_trigger_timer += DELAY_PER_LOOP;
  else distance_right_trigger_timer = 0;
  if(detected_carc())hc_trigger_timer += DELAY_PER_LOOP;
  else hc_trigger_timer = 0;
  
  // Make accurate states of the sensors
  carA_is_waiting_left = (distance_left_trigger_timer > 500) ? 1 : 0;
  carA_is_waiting_right = (distance_right_trigger_timer > 500) ? 1 : 0;
  carC_is_waiting = (hc_trigger_timer > 1000) ? 1 : 0;

  // Predict whether carA is on the bridge
  if(carA_on_bridge==ENTER_FROM_LEFT && carA_is_waiting_right)carA_on_bridge = OFF_BRIDGE;
  if(carA_on_bridge==ENTER_FROM_RIGHT && carA_is_waiting_left)carA_on_bridge = OFF_BRIDGE;
  if(!carA_on_bridge && carA_is_waiting_left)carA_on_bridge = ENTER_FROM_LEFT;
  if(!carA_on_bridge && carA_is_waiting_right)carA_on_bridge = ENTER_FROM_RIGHT;

  // Update off bridge timer
  carA_off_bridge_timer = (carA_on_bridge) ? 0 : carA_off_bridge_timer + DELAY_PER_LOOP;
  carC_off_bridge_timer = (carC_is_waiting) ? 0 : carC_off_bridge_timer + DELAY_PER_LOOP;


  // Take action

  // Update Traffic Light
  digitalWrite(LIGHT_RED, carC_is_waiting);
  digitalWrite(LIGHT_GREEN, !carC_is_waiting);

  // Update Blocking System
  s_left.write((!carA_is_waiting_left && carC_is_waiting) ? BLOCK_ON_DEGREE : BLOCK_OFF_DEGREE);
  s_right.write((!carA_is_waiting_right && carC_is_waiting) ? BLOCK_ON_DEGREE : BLOCK_OFF_DEGREE);

  // Main Operation: Raise or Lower the bridge
  // maybe we should block here?
  if(!bridge_raised && carC_is_waiting && !carA_on_bridge){
    // Raise the bridge ( pray that compiler can optimize the code below
    digitalWrite(MOTOR_LEFT_A, !digitalRead(SWITCH_LEFT));
    digitalWrite(MOTOR_RIGHT_A, !digitalRead(SWITCH_RIGHT));
    if(digitalRead(SWITCH_LEFT) && digitalRead(SWITCH_RIGHT)){
      bridge_raised = true;
      digitalWrite(MOTOR_LEFT_A, LOW);
      digitalWrite(MOTOR_RIGHT_A, LOW);
    }
  }

  if(bridge_raised && carC_off_bridge_timer>2000){
    // Lower the bridge
    digitalWrite(MOTOR_LEFT_B, 1);
    digitalWrite(MOTOR_RIGHT_B, 1);
    delay(10000); // maybe we should not block here?
    digitalWrite(MOTOR_LEFT_B, 0);
    digitalWrite(MOTOR_RIGHT_B, 0);
    bridge_raised = false;
  }


  // Debug
  #if DEBUG
  char str[100];
  sprintf(
    str,
    "carA\t|on:%d\t|wl:%d\t|wr:%d\t|t:%d\t|carC\t|on:%d\t|t:%d\t|br:%d",
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

  #if DEBUG == 2
  sprintf(
    str,
    "distance_left_timer:%d\t|distance_right_timer:%d\t|hc_trigger_timer:%d",
    distance_left_trigger_timer,
    distance_right_trigger_timer,
    hc_trigger_timer
  );
  Serial.println(str);
  #endif

  
  // loop 1000/DELAY_PER_LOOP times per second
  delay(DELAY_PER_LOOP);
}
