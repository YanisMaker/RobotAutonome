#include <Servo.h>

Servo servoAvant;

const int SERVO_PIN = 3;

// Définition des broches
const int ENA = 5;
const int IN1 = 8;
const int IN2 = 9;

const int ENB = 6;
const int IN3 = 10;
const int IN4 = 11;

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  // ----- AVANCER -----
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0); // 50 % de 255
  analogWrite(ENB, 0);

  servoAvant.attach(SERVO_PIN);
 
}

void loop() {
  delay(10000);
  // Gauche -> Droite
  for (int angle = 130; angle >=90; angle -= 5) {
    servoAvant.write(angle);
    delay(100);  // vitesse du mouvement
  }

  delay(2500);

 

  
}
