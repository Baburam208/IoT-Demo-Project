#include <SoftwareSerial.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

#define WiFi_SSID "KU Wireless Library Block"
#define WiFi_PASSWORD "ku987654321"

// Insert Firebase project API key.
#define API_KEY "AIzaSyBZmt_DZ1hzzIfMjwLRxB5ln2sZEUfbRDw"

// Insert RTDB URL define the RTDB URL
#define DATABASE_URL "https://crop-monitoring-system-5d67f-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data Object
FirebaseData fbdo;

FirebaseAuth auth;

FirebaseConfig config;

bool signupOK = false;

// Pin configurations
#define motionSensor D3
#define relay D5
#define led D6

String myString;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  WiFi.begin(WiFi_SSID, WiFi_PASSWORD);
  Serial.print("Connecting to Wi-Fi.");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Assign the api key (required) 
  config.api_key = API_KEY;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  // Signup
  if (Firebase.signUp(&config, &auth, "", "")) { // for anonymous two empty string
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task.
  config.token_status_callback = TokenStatusCallback(); // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Pin Configuration
  pinMode(motionSensor, INPUT);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  bool motion = digitalRead(motionSensor);
  if (motion) {
    Serial.println("Motion Detected. Perhaps! there is somebody.");
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }

  if (Serial.available() > 0) {
    // Read the incoming message.
    String message = Serial.readStringUntil('\n');

    // Parse the message to extract temperature, dryness and darkness values.
    float temperatureValue = parseValue(message, "Temp:");
    float drynessValue = parseValue(message, "Dryness:");
    float darknessValue = parseValue(message, "Darkness:");

    Serial.println(temperatureValue);
    Serial.println(drynessValue);
    Serial.println(darknessValue);

    if (Firebase.ready() && signupOK) {
      if (Firebase.RTDB.setFloat(&fbdo, "CropData/temperature", temperatureValue)) {
        Serial.print("Temp: ");
        Serial.println(temperatureValue);
      } else {
        Serial.println("FAILED!!!");
        Serial.println("Error: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.setFloat(&fbdo, "CropData/dryness", drynessValue)) {
        Serial.print("Dryness: ");
        Serial.println(drynessValue);
      } else {
        Serial.println("FAILED!!!");
        Serial.println("Error: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.setFloat(&fbdo, "CropData/darkness", darknessValue)) {
        Serial.print("Dark: ");
        Serial.println(darknessValue);
      } else {
        Serial.println("FAILED!!!");
        Serial.println("Error: " + fbdo.errorReason());
      }

    }

    String temperature_condition = categorize_temperature(temperatureValue);

    Serial.print("Temperature: ");
    Serial.print(temperature_condition);
    Serial.println();

    if (drynessValue > 65) {
      // Trigger water pump.
      Serial.println("Moisture level is low. Activate water pump.");
      digitalWrite(relay, HIGH);
    } else if (drynessValue >= 35 && drynessValue <= 65) {
      Serial.println("Moisture level is sufficient. For more moisture, you can turn on pump.");
    } else if (drynessValue < 34) {
      Serial.println("Enough(too much) Moisture Level.");
    }

    if (darknessValue > 80) {
      Serial.println("Low light intensity, Good for the mushroom.");
    } else if (darknessValue < 40) {
      Serial.println("Too high light intensity, Harsh condition for mushroom.");
    } else if (darknessValue >= 40 && darknessValue <= 80) {
      Serial.println("High light intensity, Unfavourable condition for mushroom.");
    }

  }

  Serial.println("___________________________________________________");
  delay(10000);
}

float parseValue(String data, String key) {
  float valueIndex = data.indexOf(key);
  if (valueIndex != -1) {
    return data.substring(valueIndex + key.length()).toFloat();
  }
  return -1;   // Return -1 if key is not found.
}


String categorize_temperature(float temperature) {
  if (temperature < 0) return "Very Cold.";
  else if (temperature >= 0 && temperature <= 10) return "Cold";
  else if (temperature > 10 && temperature <= 20) return "Cool";
  else if (temperature > 20 && temperature <= 25) return "Warn";
  else if (temperature > 25 && temperature <= 34) return "Hot";
  else return "Very Hot.";
}