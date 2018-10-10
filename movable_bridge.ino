#include <Servo.h>
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

Servo s_left;
Servo s_right;

int carA_on_bridge = 0;
int carA_is_waiting_left = 0;
int carA_is_waiting_right = 0;
int carC_under_bridge = 0;
int carC_is_waiting = 0;


void setup() {
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

  // init

  s_left.write(BLOCK_OFF_DEGREE);
  s_right.write(BLOCK_OFF_DEGREE);
  digitalWrite(LIGHT_RED, LOW);
  digitalWrite(LIGHT_GREEN, HIGH);


}

void loop() {
  bool block_cara = carC_is_waiting || carC_under_bridge;
  digitalWrite(LIGHT_RED, block_cara);
  digitalWrite(LIGHT_GREEN, !block_cara);
  s_left.write((!carA_is_waiting_left && block_cara) ? BLOCK_ON_DEGREE : BLOCK_OFF_DEGREE);
  s_right.write((!carA_is_waiting_right && block_cara) ? BLOCK_ON_DEGREE : BLOCK_OFF_DEGREE);
}
