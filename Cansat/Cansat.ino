//SD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

typedef enum{
  Write, Append
} Action;

//--------------------------------------------------------------------------------------------//

#include <string>
using namespace std;

//Pair-to-Pair
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
uint8_t broadcastAddress[] = {0xE0, 0xE2, 0xE6, 0xAC, 0x90, 0x30};

// Structure example to send data
// Must match the receiver structure
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
  esp_now_peer_info_t peerInfo;
// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//DHT
#include <DHT.h>;

#define DHTPIN 15
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE); 

int chk;
float hum; 
float temp;
//--------------------------------------------------------------------------------------------//
//BMP
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>

#define SDA 21
#define SCL 22

Adafruit_BMP280 bme; // I2C
MPU9250_asukiaaa bmp;

float aX, aY, aZ, aSqrt, gX, gY, gZ, mX, mY, mZ;

//--------------------------------------------------------------------------------------------//

unsigned long previousMillis = 0;        
const long interval = 40000;

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13,HIGH);
  
  Serial.begin(9600);

  //Pair-to-Pair
  Serial.println("Reading Values from sensors");

  Wire.begin();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer

  memcpy(peerInfo.peer_addr,broadcastAddress,6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  //SD
  if(!SD.begin()){
      Serial.println("Card Mount Failed");
      return;
  }
  Serial.println("done");
  writeFile(SD, "/humidity.csv", "Time,Humidity", Write);
  writeFile(SD, "/temperature.csv", "Time,Temperature", Write);
  writeFile(SD, "/acceleration.csv", "Time,X,Y,Z,Total", Write);
  writeFile(SD, "/gyro.csv", "Time,X,Y,Z", Write);
  writeFile(SD, "/magnetic.csv", "Time,X,Y,Z", Write);


  //Sensors
  dht.begin();

  Wire.begin(SDA, SCL);
  bmp.setWire(&Wire);

  bme.begin();
  bmp.beginAccel();
  
  bmp.beginGyro();
  bmp.beginMag();
}

void loop() {
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(13, LOW);
    delay(1);
    digitalWrite(13, HIGH);
  }
  CheckCard();
  ReadDHT();
  ReadBMP();
  
  WriteData();

  
  //Pair-to-Pair
  
  myData.hum = hum;
  myData.temp = temp;
  myData.aX = aX;
  myData.aY = aY;
  myData.aZ = aZ;
  myData.aSqrt = aSqrt;
  myData.gX = gX;
  myData.gY = gY;
  myData.gZ = gZ;
  myData.mX = mX;
  myData.mY = mY;
  myData.mZ = mZ;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));


  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  PrintVals();
  
  delay(1000);
}

/*-----------------------------------------------------------------------------------------------------*/


string readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
      Serial.println("Failed to open file for reading");
      return "Failed";
  }

  Serial.print("Read from file: ");
  string result = "";
  while(file.available()){
      char c = file.read();
    +   Serial.write(c);
      result.push_back(c);
  }
  return result;
  file.close();
}


void writeFile(fs::FS &fs, const char * path, const char * message, Action action){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, (action == Write)? FILE_WRITE : FILE_APPEND);
  //File file = fs.open(path, FILE_WRITE);
  if(!file){
      Serial.println("Failed to open file for writing");
      return;
  }
  if(file.print(message)){
      Serial.println("File written");
  } else {
      Serial.println("Write failed");
  }
  file.close();
}

void WriteData(){
  string DATA;

  //Humidity
  DATA = "\n";
  DATA.append(to_string(millis()));
  DATA.append(", ");
  DATA.append(to_string(hum));
  writeFile(SD, "/humidity.csv", DATA.c_str(), Append);

  //Temperature
  DATA = "\n";
  DATA.append(to_string(millis()));
  DATA.append(", ");
  DATA.append(to_string(temp));
  writeFile(SD, "/temperature.csv", DATA.c_str(), Append);

  //Acceleration
  DATA = "\n";
  DATA.append(to_string(millis()));
  DATA.append(", ");
  DATA.append(to_string(aX));
  DATA.append(", ");
  DATA.append(to_string(aY));
  DATA.append(", ");
  DATA.append(to_string(aZ));
  DATA.append(", ");
  DATA.append(to_string(aSqrt));
  writeFile(SD, "/acceleration.csv", DATA.c_str(), Append);

  //Gyro
  DATA = "\n";
  DATA.append(to_string(millis()));
  DATA.append(", ");
  DATA.append(to_string(gX));
  DATA.append(", ");
  DATA.append(to_string(gY));
  DATA.append(", ");
  DATA.append(to_string(gZ));
  writeFile(SD, "/gyro.csv", DATA.c_str(), Append);

  //Magnetic Field
  DATA = "\n";
  DATA.append(to_string(millis()));
  DATA.append(", ");
  DATA.append(to_string(mX));
  DATA.append(", ");
  DATA.append(to_string(mY));
  DATA.append(", ");
  DATA.append(to_string(mZ));
  writeFile(SD, "/magnetic.csv", DATA.c_str(), Append);

  DATA.pop_back();
  DATA.append(",}");
}


void CheckCard(){  
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
  }
}

void ReadDHT(){
  hum = dht.readHumidity();
  temp= dht.readTemperature();
}

void ReadBMP(){
  if (bmp.accelUpdate() == 0) {
    aX = bmp.accelX();
    aY = bmp.accelY();
    aZ = bmp.accelZ();
    aSqrt = bmp.accelSqrt();
  }
  if (bmp.gyroUpdate() == 0) {
    gX = bmp.gyroX();
    gY = bmp.gyroY();
    gZ = bmp.gyroZ();
  }
  if (bmp.magUpdate() == 0) {
    mX = bmp.magX();
    mY = bmp.magY();
    mZ = bmp.magZ();
  }
}


void PrintVals(){
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Temprature: ");
  Serial.println(temp);
 

  Serial.print("aX: ");
  Serial.print(aX);
  Serial.print("  aY: ");
  Serial.print(aY);
  Serial.print("  az: ");
  Serial.print(aZ);
  Serial.print("  aSqrt: ");
  Serial.println(aSqrt);

  Serial.print("gX: ");
  Serial.print(gX);
  Serial.print("  gY: ");
  Serial.print(gY);
  Serial.print("  gz: ");
  Serial.println(gZ);
  
  Serial.print("mX: ");
  Serial.print(mX);
  Serial.print("  mY: ");
  Serial.print(mY);
  Serial.print("  mz: ");
  Serial.println(mZ);
}
/*---------------------------------------------------------------------------------------------------*/
