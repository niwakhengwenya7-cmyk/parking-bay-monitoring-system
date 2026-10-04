#include <Servo.h>

// ==========================
// Parking Bay Monitoring System
// ==========================
// 3 IR sensors, 3 LEDs, 1 ultrasonic sensor, 1 servo motor, 1 buzzer

// IR sensor pins
const int IR_BAY_1 = 2;
const int IR_BAY_2 = 3;
const int IR_BAY_3 = 4;

// LED pins (one LED per bay)
const int LED_BAY_1 = 8;
const int LED_BAY_2 = 9;
const int LED_BAY_3 = 10;

// Ultrasonic sensor pins
const int TRIG_PIN = 5;
const int ECHO_PIN = 6;

// Servo and buzzer pins
const int SERVO_PIN = 11;
const int BUZZER_PIN = 12;

// Servo angles
const int SERVO_CLOSED_ANGLE = 90;
const int SERVO_OPEN_ANGLE = 0;

// Timing settings
const unsigned long GATE_OPEN_TIME = 4000;   // 4 seconds
const unsigned long BUZZER_BLIP_TIME = 300;  // 300 ms
const int ULTRASONIC_DISTANCE_THRESHOLD = 30; // cm

Servo gateServo;

bool isBayOccupied(int irPin) {
  // IR sensor usually LOW when object is detected
  return digitalRead(irPin) == LOW;
}

int readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25 ms timeout
  if (duration == 0) {
    return 999; // no reading
  }

  float distance = duration * 0.0343 / 2.0;
  return (int)distance;
}

void updateBayStatus(int irPin, int ledPin, const char* bayName) {
  bool occupied = isBayOccupied(irPin);
  digitalWrite(ledPin, occupied ? HIGH : LOW);

  if (occupied) {
    Serial.print(bayName);
    Serial.println(" is occupied");
  } else {
    Serial.print(bayName);
    Serial.println(" is free");
  }
}

void openGate() {
  gateServo.write(SERVO_OPEN_ANGLE);
  Serial.println("Gate opened");
}

void closeGate() {
  gateServo.write(SERVO_CLOSED_ANGLE);
  Serial.println("Gate closed");
}

void beepBuzzer(int durationMs) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(durationMs);
  digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
  Serial.begin(9600);

  pinMode(IR_BAY_1, INPUT);
  pinMode(IR_BAY_2, INPUT);
  pinMode(IR_BAY_3, INPUT);

  pinMode(LED_BAY_1, OUTPUT);
  pinMode(LED_BAY_2, OUTPUT);
  pinMode(LED_BAY_3, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  gateServo.attach(SERVO_PIN);
  closeGate();

  Serial.println("Parking Bay Monitoring System Started");
  Serial.println("-------------------------------");
}

void loop() {
  bool bay1Occupied = isBayOccupied(IR_BAY_1);
  bool bay2Occupied = isBayOccupied(IR_BAY_2);
  bool bay3Occupied = isBayOccupied(IR_BAY_3);

  // Update LED indicators
  digitalWrite(LED_BAY_1, bay1Occupied ? HIGH : LOW);
  digitalWrite(LED_BAY_2, bay2Occupied ? HIGH : LOW);
  digitalWrite(LED_BAY_3, bay3Occupied ? HIGH : LOW);

  // Check overall parking status
  bool parkingFull = bay1Occupied && bay2Occupied && bay3Occupied;

  Serial.print("Bay 1: "); Serial.println(bay1Occupied ? "Occupied" : "Free");
  Serial.print("Bay 2: "); Serial.println(bay2Occupied ? "Occupied" : "Free");
  Serial.print("Bay 3: "); Serial.println(bay3Occupied ? "Occupied" : "Free");

  if (parkingFull) {
    Serial.println("Parking is FULL");
    beepBuzzer(200);
    delay(200);
    beepBuzzer(200);
    Serial.println("Barrier remains closed");
    closeGate();
  } else {
    int distance = readDistanceCm();
    Serial.print("Distance to vehicle at gate: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance < ULTRASONIC_DISTANCE_THRESHOLD) {
      Serial.println("Vehicle detected at entry");
      openGate();
      delay(GATE_OPEN_TIME);
      closeGate();
    }
  }

  Serial.println("-------------------------------");
  delay(500);
}
