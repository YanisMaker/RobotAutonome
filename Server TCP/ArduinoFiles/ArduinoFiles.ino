#include <Servo.h>

// ================= SERVO =================
Servo servoAvant;
const int SERVO_PIN = 3;

int servoCurrentAngle = 90;
int servoTargetAngle  = 90;

const int SERVO_MIN = 50;
const int SERVO_MAX = 130;
const int SERVO_STEP = 5;
const int SERVO_DELAY = 75; // ms

// ================= L298N =================
const int ENA = 5;
const int IN1 = 8;
const int IN2 = 9;

const int ENB = 6;
const int IN3 = 10;
const int IN4 = 11;

// ================= COMMANDE =================
char currentDir = 'S';
int currentSpeed = 0;

unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 500; // ms

// ================= SETUP =================
void setup() {
  Serial.begin(115200); // RX de l'Uno branché à TX de l'ESP32

  servoAvant.attach(SERVO_PIN);
  servoAvant.write(servoCurrentAngle);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();
}

// ================= LOOP =================
void loop() {
  readSerialCommand();
  updateServo();
  applyMotorCommand();
  safetyTimeout();
}

// ================= SERIAL =================
void readSerialCommand() {
  static String rxBuffer = "";
  
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      rxBuffer.trim();
      if (rxBuffer.length() >= 2) {
        char dir = rxBuffer.charAt(0);
        int speed = rxBuffer.substring(2).toInt();
        speed = constrain(speed, 0, 100);

        currentDir = dir;
        currentSpeed = speed;
        lastCommandTime = millis();

        // Définir angle cible
        if (dir == 'L') servoTargetAngle = SERVO_MIN;
        else if (dir == 'R') servoTargetAngle = SERVO_MAX;
        else servoTargetAngle = 90;
      }
      rxBuffer = "";
    } else {
      rxBuffer += c;
    }
  }
}

// ================= SERVO MOUVEMENT PROGRESSIF =================
void updateServo() {
  if (servoCurrentAngle == servoTargetAngle) return;

  if (servoCurrentAngle < servoTargetAngle)
    servoCurrentAngle += SERVO_STEP;
  else
    servoCurrentAngle -= SERVO_STEP;

  servoCurrentAngle = constrain(servoCurrentAngle, SERVO_MIN, SERVO_MAX);
  servoAvant.write(servoCurrentAngle);
  delay(SERVO_DELAY);
}

// ================= MOTEURS =================
void applyMotorCommand() {
  int pwm = map(currentSpeed, 0, 100, 0, 255);

  if (currentDir == 'F') {
    forward(pwm);
  } else if (currentDir == 'B') {
    backward(pwm);
  } else {
    stopMotors();
  }
}

void forward(int pwm) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);
}

void backward(int pwm) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ================= SÉCURITÉ =================
void safetyTimeout() {
  if (millis() - lastCommandTime > COMMAND_TIMEOUT) {
    currentSpeed = 0;
    currentDir = 'S';
    stopMotors();
    servoTargetAngle = 90;
  }
}
