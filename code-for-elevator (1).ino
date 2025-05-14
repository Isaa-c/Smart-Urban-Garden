/*
Elevator code
STEMNNOVATION 2023
CODE WRITTEN BY KUMACA STEM TEAM
*/
#include <NewPing.h>

//code for Motor control pins
int motorPin1 = 3;  // Input 1 of L298N
int motorPin2 = 2;  // Input 2 of L298N

// Buttons for each floor
int button1Pin = 4;  // Button for groundfloor
int button2Pin = 5;  // Button for floor 1
int button3Pin = 6;  // Button for floor 2
int button4Pin = 7;  // Button for floor 3 

// Motor speed control
int forwardSpeed = 70;  // Speed for forward movement (0-255)
int backwardSpeed = 60; // Speed for backward movement (0-255)

// Ultrasonic sensor pins
#define TRIGGER_PIN 8 
#define ECHO_PIN 10    
//floor distance
int floor1Distance = 44;  
int floor2Distance = 37;  
int floor3Distance = 20;  
int floor4Distance = 1;  

int tolerance = 3;  // +/- 3 cm

NewPing sonar(TRIGGER_PIN, ECHO_PIN, 50);  // Create a NewPing object

// Current floor variable
int currentFloor = 1;  // Initialize to ground floor (1)
int targetFloor = 0;   // Initialize to no target floor (0)

void setup() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  pinMode(button1Pin, INPUT_PULLUP);  
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(button4Pin, INPUT_PULLUP);

  pinMode(9, OUTPUT);  // Set pin 9 as an output for motor speed control (ENA)

  Serial.begin(9600);  
}

void loop() {
  if (digitalRead(button1Pin) == LOW) {
    targetFloor = 1;
  } else if (digitalRead(button2Pin) == LOW) {
    targetFloor = 2;
  } else if (digitalRead(button3Pin) == LOW) {
    targetFloor = 3;
  } else if (digitalRead(button4Pin) == LOW) {
    targetFloor = 4;
  }

  unsigned int distance = sonar.ping_cm();

  if (targetFloor != 0) {
    if (abs(distance - targetDistance(targetFloor)) <= tolerance) {
      stopMotor();
      currentFloor = targetFloor;
      targetFloor = 0;  // Reset target floor
    } else if (targetFloor > currentFloor) {
      // Move up with forward speed
      moveMotor(forwardSpeed, HIGH, LOW);
    } else if (targetFloor < currentFloor) {
      // Move down with backward speed
      moveMotor(backwardSpeed, LOW, HIGH);
    }
  }

  // Print the distance and current floor to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Current Floor: ");
  Serial.print(currentFloor);
  Serial.print(", Target Floor: ");
  Serial.println(targetFloor);
}

int targetDistance(int floor) {
  switch (floor) {
    case 1:
      return floor1Distance;
    case 2:
      return floor2Distance;
    case 3:
      return floor3Distance;
    case 4:
      return floor4Distance;
    default:
      return 0;
  }
}

void moveMotor(int speed, int dir1, int dir2) {
  analogWrite(9, speed);  // Use pin 9 for ENA
  digitalWrite(motorPin1, dir1);
  digitalWrite(motorPin2, dir2);
}

void stopMotor() {
  analogWrite(9, 0);  // pin 9 is use for ENA(on L298N shield) in order to control the motor speed  
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
}