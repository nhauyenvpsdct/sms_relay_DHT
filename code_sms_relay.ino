#include <SoftwareSerial.h>
#include <DHT.h>
#include "LiquidCrystal_I2C.h"

// Uncomment the following line if using standard LiquidCrystal library
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Make sure this address is correct
#define SIM800_TX 10
#define SIM800_RX 11
#define waterSensorPin A0
#define Relay_Pin 8
#define soilMoisturePin A1
#define VCC_water_sensor 7
#define DHTPin A3
#define DHTType DHT11

const int sensorMin = 0;
const int sensorMax = 700; 
const int soilMoistureThreshold = 15;
String Phone_num = "0364086620";

SoftwareSerial sim800l(SIM800_TX, SIM800_RX);

void sendSMS(const char* phoneNumber, const char* message);
int readWaterLevel();
void controlRelayBasedOnWaterLevel(int waterLevel);
void controlPumpBasedOnSoilMoisture();
DHT dht(DHTPin, DHTType);

struct DHTData {
  float temperature;
  float humidity;
};

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  dht.begin();
  lcd.begin(16, 2); // Initialize the LCD
  lcd.backlight();
  pinMode(Relay_Pin, OUTPUT);
  pinMode(VCC_water_sensor, OUTPUT);

  Serial.println("Khởi động SIM800L...");
  sim800l.println("AT");
  delay(1000);
  if (sim800l.available()) {
    Serial.println(sim800l.readString());
  }

  sim800l.println("AT+CMGF=1");
  delay(1000);
  if (sim800l.available()) {
    Serial.println(sim800l.readString());
  }
  // DHTData data = readDHT();
  // temp_to_sms(data);
}

void loop() {
  int waterLevel = readWaterLevel();
  Serial.println("Water Level: " + String(waterLevel) + "%");
  //controlPumpBasedOnSoilMoisture();
  //controlRelayBasedOnWaterLevel(waterLevel);

  DHTData data = readDHT();
  displayOnLCD(data);
  Serial.println("Nhiệt độ: " + String(data.temperature) + "C");
  Serial.println("Độ ẩm: " + String(data.humidity) + "%");
  delay(2000);
}

void sendSMS(const char* phoneNumber, const char* message) {
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
  sim800l.println("\"");
  delay(1000);
  if (sim800l.available()) {
    Serial.println(sim800l.readString());
  }
  sim800l.print(message);
  delay(1000);
  sim800l.write(26);
  delay(1000);
  if (sim800l.available()) {
    Serial.println(sim800l.readString());
  }
  Serial.println("Tin nhắn đã được gửi.");
}

int readWaterLevel() {
  digitalWrite(VCC_water_sensor, HIGH);
  int sensorValue = analogRead(waterSensorPin);
  digitalWrite(VCC_water_sensor, LOW);
  int waterLevel = map(sensorValue, sensorMin, sensorMax, 0, 100);
  return waterLevel;
}

void controlRelayBasedOnWaterLevel(int waterLevel) {
  if (waterLevel >= 70) {
    digitalWrite(Relay_Pin, HIGH);
    String message1 = "Canh bao muc nuoc > " + String(waterLevel) + "%. Da kich hoat may bom nuoc!";
    sendSMS(Phone_num.c_str(), message1.c_str());
    Serial.println(message1);
  } else if (waterLevel <= 40) {
    digitalWrite(Relay_Pin, LOW);
    String message2 = "Canh bao muc nuoc < " + String(waterLevel) + "%!";
    sendSMS(Phone_num.c_str(), message2.c_str());
    Serial.println(message2);
  }
}

void controlPumpBasedOnSoilMoisture() {
  int soilMoisture = analogRead(soilMoisturePin);
  int soilMoisturePercentage = map(soilMoisture, 0, 1023, 0, 100);
  Serial.println("Soil Moisture: " + String(soilMoisturePercentage) + "%");
  if (soilMoisturePercentage < soilMoistureThreshold) {
    digitalWrite(Relay_Pin, HIGH);
    sendSMS(Phone_num.c_str(), "Da BAT may bom");
    Serial.println("Máy bơm được kích hoạt.");
  } else {
    digitalWrite(Relay_Pin, LOW);
    sendSMS(Phone_num.c_str(), "Da NGAT may bom");
    Serial.println("Máy bơm được ngắt kích hoạt.");
  }
}

DHTData readDHT() {
  DHTData data;
  data.humidity = dht.readHumidity();
  data.temperature = dht.readTemperature(); // Đọc nhiệt độ theo độ C

  if (isnan(data.humidity) || isnan(data.temperature)) {
    Serial.println("Lỗi đọc cảm biến DHT!");
    data.humidity = data.temperature = -1; // Gán giá trị lỗi để nhận biết
  }
  
  return data;
}

void displayOnLCD(DHTData data) {
  lcd.clear(); // Xóa màn hình LCD để hiển thị mới
  if (data.temperature == -1 || data.humidity == -1) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor error!");
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(data.temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(data.humidity);
  lcd.print("%");
}

void temp_to_sms(DHTData data)
{
  String sendto = "Temperature: " + String(data.temperature) + " C\n Humidity: " + String(data.humidity) + "%";
  sendSMS("0364086620", sendto.c_str());
}
