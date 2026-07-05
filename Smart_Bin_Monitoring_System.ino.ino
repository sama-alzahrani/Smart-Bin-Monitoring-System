#define BLYNK_TEMPLATE_ID "TMPL67yt6000Of"
#define BLYNK_TEMPLATE_NAME "ES"
#define BLYNK_AUTH_TOKEN    "nWXFHGoA8kCWcoiuZUQshaHSCZ3izTqm"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

#define TRIG_PIN 5
#define ECHO_PIN 18

const int BIN_DEPTH = 20;  
const int MIN_DIST  = 5;   

LiquidCrystal_I2C lcd(0x27, 20, 4); 

BlynkTimer timer;
bool notificationSent = false;

// رابط افتراضي مسبقاً (مثلاً موقع وسط الرياض) في حال لم يتم إدخال موقع بعد
String googleMapsUrl = "https://maps.google.com/?q=24.7136,46.6753";

// دالة تستقبل الإحداثيات النصية التي تكتبها في ودجت الـ Text Input أو Terminal (V3)
BLYNK_WRITE(V3) {
  String incomingCoords = param.asStr(); // يأخذ النص المكتوب (مثل: 24.7136,46.6753)
  
  if (incomingCoords.length() > 5) { // التأكد من أن النص ليس فارغاً
    googleMapsUrl = "https://maps.google.com/?q=" + incomingCoords;
    // تحديث الرابط في التطبيق فوراً على القناة V4
    Blynk.virtualWrite(V4, googleMapsUrl);
  }
}

void checkBinLevel() {
  // 1. قياس المسافة
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;
  
  if (distance > BIN_DEPTH) distance = BIN_DEPTH;
  if (distance < MIN_DIST) distance = MIN_DIST;
  
  int fillPercentage = map(distance, BIN_DEPTH, MIN_DIST, 0, 100);
  fillPercentage = constrain(fillPercentage, 0, 100);
  
  // 2. العرض على LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bin Level: ");
  lcd.print(fillPercentage);
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  int numBars = map(fillPercentage, 0, 100, 0, 16); 
  for (int i = 0; i < numBars; i++) {
    lcd.print((char)255); 
  }
  
  // 3. إرسال البيانات للـ Blynk
  if (Blynk.connected()) {
    Blynk.virtualWrite(V1, fillPercentage);
    Blynk.virtualWrite(V4, googleMapsUrl); // إرسال الرابط المحدث ليراه الزوار
    
    // 4. الإشعارات عند الامتلاء
    if (fillPercentage >= 95) {
      if (!notificationSent) {
        Blynk.logEvent("bin_full"); 
        notificationSent = true;
      }
    } else {
      notificationSent = false; 
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Wire.begin(21,22);
  Serial.println("I2C OK");

  lcd.init();
  Serial.println("LCD OK");

  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GHALA TEST");
  Serial.println("Text Printed");

  delay(1500);

  WiFi.begin(ssid, pass);
  Serial.println("WiFi Started");

  Blynk.config(auth);
  Serial.println("Blynk OK");

  timer.setInterval(2000L, checkBinLevel);
 Serial.println("Timer OK");
  lcd.clear();
  lcd.clear();
 lcd.setCursor(0,0);
  lcd.print("GHALA123");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  timer.run();
}