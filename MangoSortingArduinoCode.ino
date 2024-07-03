#include <Servo.h>
#include <HX711_ADC.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define DOUT  D6
#define CLK  D7
 
HX711_ADC scale(DOUT, CLK); 

Servo s1;
Servo s2;
Servo s3;
Servo s4;
Servo s5;

// int previousMediumRawValue = -1;
// int previousLargeRawValue = -1;
// int previousMediumRipeValue = -1;
// int previousLargeRipeValue = -1;

String lastCategory = "";
unsigned long categoryStartTime = 0;
const unsigned long categoryStableTime = 1000;

#define WIFI_SSID "wifi"
#define WIFI_PASSWORD "password"
#define API_KEY "api key"
#define DATABASE_URL "database url" 


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool mangoStatus = false;
bool servo2Active = false;
bool servo3Active = false;
bool servo4Active = false;
bool servo5Active = false;

void setup() {
  Serial.begin(9600);
  s1.attach(D0);
  s2.attach(D1);
  s3.attach(D2);
  s4.attach(D3);
  s5.attach(D4);
  scale.begin(); 
  scale.start(2000); 
  scale.setCalFactor(1000.0);
  Serial.println("Scale is ready");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
   while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
   Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}

void resetServos() {
  s2.write(0);
  s3.write(0);
  s4.write(0);
  s5.write(0);
  servo2Active = false;
  servo3Active = false;
  servo4Active = false;
  servo5Active = false;
}



void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    scale.update();
    float weight = scale.getData(); 
    String currentCategory = "";
    Serial.println(weight);
    if (weight >= 180 && weight <= 249) {
      currentCategory = "small";
    } else if (weight >= 250 && weight <= 349) {
      currentCategory = "medium";
    } else if (weight >= 350 && weight <= 400) {
      currentCategory = "large";
    }
    if (currentCategory != lastCategory) {
      lastCategory = currentCategory;
      categoryStartTime = millis(); 
    } else if (currentCategory != "" && millis() - categoryStartTime >= categoryStableTime) {
      Serial.println(currentCategory);
      scale.tare();
    }
    
    if (Firebase.RTDB.getBool(&fbdo, "mango/1/maturity")) {
      if (fbdo.dataType() == "boolean") {
        mangoStatus = fbdo.boolData();
        Serial.println("Success: " + fbdo.dataPath() + ": " + mangoStatus + " (" + fbdo.dataType() + ")");
        if (mangoStatus == true) {
          if (currentCategory == "medium" && !servo2Active) {
            s2.write(180);
            Serial.println("Medium Ripe is Rotating");
            servo2Active = true;
          } else if (currentCategory == "large" && !servo3Active) {
            s3.write(180);
            Serial.println("Large Ripe is Rotating");
            servo3Active = true;
          }
        } else {
          if (currentCategory == "medium" && !servo4Active) {
            s4.write(180);
            Serial.println("Medium Raw is Rotating");
            servo4Active = true;
          } else if (currentCategory == "large" && !servo5Active) {
            s5.write(180);
            Serial.println("Large Raw is Rotating");
            servo5Active = true;
          }
        }
      }
    } else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }

  if (s1.read() == 0 && s2.read() == 0 && s3.read() == 0 && s4.read() == 0 && s5.read() == 0) {
    resetServos();
  }
}

// void mediumRawChecker() {
//     if (Firebase.RTDB.getInt(&fbdo, "raw/1/medium")) {
//         if (fbdo.dataType() == "int") {
//             int currentMediumRawValue = fbdo.intData();
//             if (currentMediumRawValue != previousMediumRawValue) {
//                 Serial.print("Small raw value changed to: ");
//                 Serial.println(currentMediumRawValue);
//                 s4.write(0);
//                 previousMediumRawValue = currentMediumRawValue;
//             }
//         }
//     } else {
//         Serial.println("Failed to get count from Firebase");
//         Serial.println("REASON: " + fbdo.errorReason());
//     }
// }

// void largeRawChecker() {
//     if (Firebase.RTDB.getInt(&fbdo, "raw/1/large")) {
//         if (fbdo.dataType() == "int") {
//             int currentLargeRawValue = fbdo.intData();
//             if (currentLargeRawValue != previousLargeRawValue) {
//                 Serial.print("Small raw value changed to: ");
//                 Serial.println(currentLargeRawValue);
//                 s5.write(0);
//                 previousLargeRawValue = currentLargeRawValue;
//             }
//         }
//     } else {
//         Serial.println("Failed to get count from Firebase");
//         Serial.println("REASON: " + fbdo.errorReason());
//     }
// }

// void mediumRipeChecker() {
//     if (Firebase.RTDB.getInt(&fbdo, "ripe/1/medium")) {
//         if (fbdo.dataType() == "int") {
//             int currentMediumRipeValue = fbdo.intData();
//             if (currentMediumRipeValue != previousMediumRipeValue) {
//                 Serial.print("Small raw value changed to: ");
//                 Serial.println(currentMediumRipeValue);
//                 s2.write(0);
//                 previousMediumRipeValue = currentMediumRipeValue;
//             }
//         }
//     } else {
//         Serial.println("Failed to get count from Firebase");
//         Serial.println("REASON: " + fbdo.errorReason());
//     }
// }

// void largeRipeChecker() {
//     if (Firebase.RTDB.getInt(&fbdo, "ripe/1/large")) {
//         if (fbdo.dataType() == "int") {
//             int currentLargeRipeValue = fbdo.intData();
//             if (currentLargeRipeValue != previousLargeRipeValue) {
//                 Serial.print("Small raw value changed to: ");
//                 Serial.println(currentLargeRipeValue);
//                 s3.write(0);
//                 previousLargeRipeValue = currentLargeRipeValue;
//             }
//         }
//     } else {
//         Serial.println("Failed to get count from Firebase");
//         Serial.println("REASON: " + fbdo.errorReason());
//     }
// }



