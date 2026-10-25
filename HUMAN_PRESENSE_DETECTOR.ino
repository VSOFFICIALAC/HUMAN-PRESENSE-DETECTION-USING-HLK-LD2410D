/*
 * FIXED VERSION - Uses the same detection as debug
 * Blue = Person detected → Map to Red/Yellow/Green
 */

#include <FastLED.h>
#include <SoftwareSerial.h>

// ===== CONFIGURATION =====
#define LED_PIN         6
#define NUM_LEDS        16
#define LED_BRIGHTNESS  50

#define SENSOR_RX_PIN   4
#define SENSOR_TX_PIN   3

#define NEAR_THRESHOLD  150   // 1.5 meters = Red
#define FAR_THRESHOLD   250   // 2.5 meters = Yellow

// ===== GLOBAL =====
SoftwareSerial sensorSerial(SENSOR_RX_PIN, SENSOR_TX_PIN);
CRGB leds[NUM_LEDS];

bool personDetected = false;
int personDistance = 0;
String serialBuffer = "";

enum LEDState {
  STATE_GREEN,
  STATE_RED,
  STATE_YELLOW
};

LEDState currentState = STATE_GREEN;

int animationStep = 0;
unsigned long lastAnimationTime = 0;
unsigned long lastBlinkTime = 0;
bool blinkState = false;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  sensorSerial.begin(115200);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  
  startupAnimation();
  Serial.println("\n=== SYSTEM READY ===");
  Serial.println("Red   = Person < 1.5m");
  Serial.println("Yellow = Person < 2.5m");
  Serial.println("Green  = No person");
  Serial.println("======================");
}

// ===== MAIN LOOP =====
void loop() {
  readSensorData();
  updateState();
  updateLEDAnimation();
  delay(10);
}

// ===== SENSOR READING (SAME AS DEBUG) =====
void readSensorData() {
  while (sensorSerial.available()) {
    char c = sensorSerial.read();
    serialBuffer += c;
    
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 1) {
        parseData(serialBuffer);
        serialBuffer = "";
      }
    }
  }
}

void parseData(String data) {
  data.trim();
  if (data.length() == 0) return;
  
  // Reset
  bool detected = false;
  int distance = 0;
  
  // Look for D: pattern
  int dIdx = data.indexOf("D:");
  if (dIdx != -1) {
    String after = data.substring(dIdx + 2);
    String num = "";
    for (int i = 0; i < after.length(); i++) {
      char ch = after.charAt(i);
      if (ch >= '0' && ch <= '9') num += ch;
      else break;
    }
    if (num.length() > 0) {
      distance = num.toInt();
      if (distance > 0 && distance < 600) {
        detected = true;
      }
    }
  }
  
  // Look for M: pattern
  if (!detected) {
    int mIdx = data.indexOf("M:");
    if (mIdx != -1) {
      String after = data.substring(mIdx + 2);
      if (after.indexOf("1") == 0) {
        detected = true;
      }
    }
  }
  
  // Look for P: pattern
  if (!detected) {
    int pIdx = data.indexOf("P:");
    if (pIdx != -1) {
      String after = data.substring(pIdx + 2);
      if (after.indexOf("1") == 0) {
        detected = true;
      }
    }
  }
  
  // Fallback: ANY number between 10-600
  if (!detected) {
    String allNums = "";
    for (int i = 0; i < data.length(); i++) {
      char ch = data.charAt(i);
      if (ch >= '0' && ch <= '9') {
        allNums += ch;
      } else if (allNums.length() > 0) {
        break;
      }
    }
    if (allNums.length() > 0) {
      int num = allNums.toInt();
      if (num > 10 && num < 600) {
        distance = num;
        detected = true;
      }
    }
  }
  
  // Update globals
  personDetected = detected;
  personDistance = distance;
  
  // Debug print
  if (detected) {
    Serial.print("✅ Person at ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("❌ No person");
  }
}

// ===== STATE MANAGEMENT =====
void updateState() {
  LEDState newState = STATE_GREEN;
  
  if (personDetected) {
    if (personDistance > 0 && personDistance < NEAR_THRESHOLD) {
      newState = STATE_RED;
      Serial.print("🔴 RED: ");
    } else if (personDistance > 0 && personDistance < FAR_THRESHOLD) {
      newState = STATE_YELLOW;
      Serial.print("🟡 YELLOW: ");
    } else if (personDistance > 0) {
      newState = STATE_GREEN;
      Serial.print("🟢 GREEN (too far): ");
    } else {
      newState = STATE_RED;
      Serial.print("🔴 RED (unknown distance): ");
    }
    Serial.print(personDistance);
    Serial.println(" cm");
  } else {
    newState = STATE_GREEN;
    Serial.println("🟢 GREEN (no person)");
  }
  
  if (newState != currentState) {
    currentState = newState;
    animationStep = 0;
    Serial.println("--- State changed! ---");
  }
}

// ===== LED ANIMATIONS =====
void updateLEDAnimation() {
  switch(currentState) {
    case STATE_GREEN:
      showGreenBlink();
      break;
    case STATE_RED:
      showLoadingAnimation(CRGB::Red);
      break;
    case STATE_YELLOW:
      showLoadingAnimation(CRGB::Yellow);
      break;
  }
}

void showGreenBlink() {
  if (millis() - lastBlinkTime > 500) {
    lastBlinkTime = millis();
    blinkState = !blinkState;
  }
  fill_solid(leds, NUM_LEDS, blinkState ? CRGB::Green : CRGB::Black);
  FastLED.show();
}

void showLoadingAnimation(CRGB color) {
  if (millis() - lastAnimationTime > 80) {
    lastAnimationTime = millis();
    animationStep = (animationStep + 1) % (NUM_LEDS + 4);
  }
  
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  int litCount;
  if (animationStep < NUM_LEDS) {
    litCount = animationStep + 1;
  } else if (animationStep < NUM_LEDS + 2) {
    litCount = NUM_LEDS;
  } else {
    litCount = NUM_LEDS - (animationStep - NUM_LEDS - 2);
    if (litCount < 0) litCount = 0;
  }
  
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < litCount) {
      float brightness = 1.0;
      int positionFromFront = litCount - i;
      if (positionFromFront > 3) {
        brightness = 0.3 + 0.7 * (3.0 / positionFromFront);
      }
      leds[i] = color;
      leds[i].fadeLightBy(255 * (1 - brightness));
    }
  }
  FastLED.show();
}

void startupAnimation() {
  for (int i = 0; i < 3; i++) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();
    delay(200);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(100);
  }
  
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Yellow);
  FastLED.show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(500);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}