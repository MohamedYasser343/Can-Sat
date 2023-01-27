#include <string>
using namespace std;

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  float hum;
  float temp;
  float aX;
  float aY;
  float aZ;
  float aSqrt;
  float gX;
  float gY;
  float gZ;
  float mX;
  float mY;
  float mZ;
} struct_message;

// Create a struct_message called myData
struct_message myData;



void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  Wire.begin();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}



// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  // Print data on Serial monitor 
   Serial.println(myData.hum);
   Serial.println(myData.temp);
   Serial.println(myData.aX);
   Serial.println(myData.aY);
   Serial.println(myData.aZ);
   Serial.println(myData.aSqrt);
   Serial.println(myData.gX);
   Serial.println(myData.gY);
   Serial.println(myData.gZ);
   Serial.println(myData.mX);
   Serial.println(myData.mY);
   Serial.println(myData.mZ);
}
