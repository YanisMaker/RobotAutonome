#include <WiFi.h>
#include <ESP32Servo.h> 

// ================= CONFIGURATION WIFI =================
const char* WIFI_SSID = "Freebox-61B0E1";
const char* WIFI_PASS = "rhf2zq23hxqkrfmsqb2sb5";

// ================= PARAMÈTRES TCP =================
WiFiServer server(3333);
WiFiClient client;
String rxBuffer = "";

// ================= PARAMÈTRES SERVO =================
Servo servoAvant;
const int SERVO_PIN = 18;
int angleCurrent = 90;
int angleTarget = 90;
const int SERVO_MIN = 30;
const int SERVO_MAX = 150;
const int SERVO_STEP = 5;
unsigned long lastServoMove = 0;
const int SERVO_DELAY = 20; 

// ================= PINS L298N =================
const int ENA = 4;
const int IN1 = 5;
const int IN2 = 6;
const int ENB = 7;
const int IN3 = 15;
const int IN4 = 16;

// ================= ÉTAT GLOBAL =================
char dir = 'S'; 
int pwmPct = 50;  // Vitesse par défaut à 50%
int pwmValue = 127;

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  ESP32PWM::allocateTimer(0);
  servoAvant.setPeriodHertz(50); 
  servoAvant.attach(SERVO_PIN, 500, 2400); 
  servoAvant.write(angleCurrent);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK ! IP : " + WiFi.localIP().toString());

  server.begin();
}

void loop() {
  handleTCP();     
  updateServo();   
  applyMotor();    
}

void handleTCP() {
  // 1. Vérifier si un nouveau client arrive
  if (!client || !client.connected()) {
    // Si on était en train de rouler et que le client vient de se déconnecter
    if (dir != 'S') {
        Serial.println("!!! DECONNEXION : Arrêt de sécurité !!!");
        stopEverything();
    }
    
    client = server.available();
    if (client) {
      Serial.println("Client Python connecté");
      rxBuffer = "";
    }
  }

  // 2. Lire les données si connecté
  if (client && client.connected()) {
    while (client.available()) {
      char c = client.read();
      if (c == '\n') {
        processCommand(rxBuffer);
        rxBuffer = "";
      } else {
        rxBuffer += c;
      }
    }
  }
}

void processCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;
  Serial.println("Reçu : " + cmd);

  if (cmd == "F") dir = 'F';
  else if (cmd == "B") dir = 'B';
  else if (cmd == "S") dir = 'S';
  else if (cmd == "L") angleTarget -= 20;
  else if (cmd == "R") angleTarget += 20;
  else if (cmd == "A") pwmPct += 10;
  else if (cmd == "Z") pwmPct -= 10;
  else if (cmd == "SPACE") {
    stopEverything();
    angleTarget = 90;
  }

  angleTarget = constrain(angleTarget, SERVO_MIN, SERVO_MAX);
  pwmPct = constrain(pwmPct, 0, 100);
  pwmValue = map(pwmPct, 0, 100, 0, 255);
}

void stopEverything() {
    dir = 'S';
    // On ne met pas pwmPct à 0 pour pouvoir redémarrer direct 
    // mais on coupe la puissance immédiate
    pwmValue = 0; 
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0); analogWrite(ENB, 0);
}

void updateServo() {
  if (millis() - lastServoMove > SERVO_DELAY) {
    if (angleCurrent != angleTarget) {
      if (angleCurrent < angleTarget) angleCurrent += SERVO_STEP;
      else angleCurrent -= SERVO_STEP;
      angleCurrent = constrain(angleCurrent, SERVO_MIN, SERVO_MAX);
      servoAvant.write(angleCurrent);
    }
    lastServoMove = millis();
  }
}

void applyMotor() {
  if (dir == 'F') {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, pwmValue); analogWrite(ENB, pwmValue);
  } 
  else if (dir == 'B') {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENA, pwmValue); analogWrite(ENB, pwmValue);
  } 
  else {
    stopEverything();
  }
}