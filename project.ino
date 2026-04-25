// ================= LIBRARIES =================
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ---------------- MQ-2 ----------------
#define MQ2_PIN 34

// ---------------- VIBRATION ----------------
#define VIB_PIN 14

// ---------------- PIR ----------------
#define PIR_PIN 13

// ---------------- FLAME SENSOR ----------------
#define FLAME_PIN 27

// ---------------- BUZZER ----------------
#define BUZZER_PIN 12

// ---------------- PUMP ----------------
#define PUMP_PIN 26

// ---------------- WIFI ----------------
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ---------------- TELEGRAM ----------------
#define BOT_TOKEN "YOUR_BOT_TOKEN"
#define CHAT_ID "YOUR_CHAT_ID"


WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ---------------- THRESHOLDS ----------------
int smokeThreshold = 500;

// ---------------- TIMERS ----------------
unsigned long quakeAlarmUntil = 0;

// ---------------- MOTION LOCK ----------------
unsigned long motionLockUntil = 0;
bool motionLockActive = false;

#define QUAKE_DURATION  5000
#define MOTION_DURATION 5000

// ---------------- TRIGGER FLAGS ----------------
bool quakeTriggered = false;

// ---------------- VIBRATION ----------------
int vibCount = 0;
const int vibDebounceLimit = 3;

// ---------------- PIR FILTER ----------------
int motionHits = 0;
unsigned long lastMotionHitTime = 0;

const unsigned long MOTION_WINDOW = 1500;
const int MOTION_THRESHOLD = 3;

// ---------------- TELEGRAM CONTROL ----------------
unsigned long lastTelegramTime = 0;
const unsigned long TELEGRAM_COOLDOWN = 8000;

// ---------------- STATE FLAGS ----------------
bool fireSent = false;
bool quakeSent = false;
bool motionSent = false;

// ---------------- PUMP CONTROL ----------------
bool pumpState = false;
unsigned long buzzerOffTime = 0;

// ================= TELEGRAM FUNCTION =================
void sendTelegram(String msg) {

  if (millis() - lastTelegramTime < TELEGRAM_COOLDOWN) return;

  bool ok = bot.sendMessage(CHAT_ID, msg, "");

  if (!ok) {
    delay(500);
    bot.sendMessage(CHAT_ID, msg, "");
  }

  lastTelegramTime = millis();
}

// ... [LIBRARIES AND DEFINES STAY THE SAME] ...

void setup() {
  Serial.begin(115200);

  pinMode(MQ2_PIN, INPUT);
  pinMode(VIB_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  
  // NINJA START: Set to INPUT initially to keep the pump OFF
  pinMode(PUMP_PIN, INPUT); 

  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("SMART SAFETY SYSTEM STARTED");

  // ---------------- WIFI ----------------
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  client.setInsecure();
  client.setTimeout(5000);

  delay(2000);
}

void loop() {

  unsigned long now = millis();

  // ---------------- MOTION LOCK ----------------
  if (motionLockActive && now > motionLockUntil) {
    motionLockActive = false;
    motionSent = false;
  }

  if (motionLockActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    
    // NINJA: Ensure pump is OFF in lock mode
    pinMode(PUMP_PIN, INPUT); 
    
    Serial.println("[LOCK MODE]");
    delay(200);
    return;
  }

  // ---------------- MQ-2 + FLAME ----------------
  int smokeValue = analogRead(MQ2_PIN);
  int flameState = digitalRead(FLAME_PIN);

  bool smokeFire = (smokeValue > smokeThreshold);
  bool flameFire = (flameState == LOW);

  bool fireActive = (smokeFire || flameFire);

  bool buzzerOn = false;

  if (fireActive) {

    Serial.println("🔥 FIRE ACTIVE");
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerOn = true;

    // NINJA: To turn ON, set to OUTPUT and pull LOW
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW); 
    
    pumpState = true;

    if (!fireSent) {
      sendTelegram("🔥 FIRE ALERT! Smoke/Flame detected!");
      fireSent = true;
    }

  } else {
    fireSent = false;
  }

  // ---------------- VIBRATION ----------------
  int vibState = digitalRead(VIB_PIN);

  if (vibState == HIGH) vibCount++;
  else vibCount = 0;

  if (vibCount >= vibDebounceLimit && !quakeTriggered) {

    quakeAlarmUntil = now + QUAKE_DURATION;
    quakeTriggered = true;

    Serial.println("🚨 EARTHQUAKE ALERT!");
    sendTelegram("🚨 EARTHQUAKE DETECTED!");
  }

  if (vibCount == 0) {
    quakeTriggered = false;
  }

  bool quakeActive = (now < quakeAlarmUntil);

  if (!quakeActive) quakeSent = false;

  // ---------------- PIR ----------------
  int pirState = digitalRead(PIR_PIN);

  if (pirState == HIGH) {
    if (now - lastMotionHitTime < MOTION_WINDOW) {
      motionHits++;
    } else {
      motionHits = 1;
    }
    lastMotionHitTime = now;
  } else {
    motionHits = 0;
  }

  if (motionHits >= MOTION_THRESHOLD) {

    motionLockActive = true;
    motionLockUntil = now + MOTION_DURATION;

    Serial.println("🚶 HUMAN MOTION DETECTED");
    sendTelegram("🚶 Motion detected in restricted area!");

    motionHits = 0;
  }

  // ---------------- BUZZER CONTROL ----------------
  if (fireActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerOn = true;
  }
  else if (quakeActive || motionLockActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerOn = true;
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerOn = false;
  }

  // ---------------- PUMP DELAY LOGIC (NINJA) ----------------
  if (!fireActive && pumpState) {
    buzzerOffTime = now;
    pumpState = false;
  }

  if (!pumpState && (now - buzzerOffTime >= 1000)) {
    // NINJA: To turn OFF, set back to INPUT to disconnect
    pinMode(PUMP_PIN, INPUT); 
  }

  Serial.println("------------------------");
  delay(200);
}