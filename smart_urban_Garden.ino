#include <Wire.h>
#include <hd44780.h>                       // Main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <DHT.h>
#include <DHT_U.h>

// DHT sensor setup
#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Define variables for Temperature and Humidity
float temp;
float hum;
const int DryValue = 780;
const int WetValue = 382;

// Variables for soil moisture
int soilMoisturePin = A3;
int soilMoistureValue;
int soilMoisturePercent;

// Relay Ports
#define RELAY_OUT 3
#define RELAY_OUT2 11 // New relay for pump 2
String pump_status_text = "OFF";
String pump2_status_text = "OFF";  // New pump status text

// Sun Intensity
int sunIntensityPin = A1;
int sunIntensityValue;
int sunIntensityPercent;

#include "thingProperties.h"

// LCD setup
hd44780_I2Cexp lcd; // Declare lcd object

// Ultrasonic sensor setup
#define TRIG_PIN 9
#define ECHO_PIN 10
long duration;
int waterLevel;
int maxDistance = 100; // Max distance for water level (100cm)

// LED pins
#define RED_LED_PIN 4
#define GREEN_LED_PIN 5
#define BLUE_LED_PIN 6

// Timer variables
unsigned long previousMillis = 0;
const long interval = 5000; // Interval to change display (5 seconds)
bool showFirstSet = true;   // Flag to alternate between display sets

void setup() {
  Serial.begin(9600);
  delay(1500);

  // Initialize LCD
  lcd.begin(20, 4);  // Initialize the lcd for 20x4 display
  lcd.setBacklight(255);  // Turn on the backlight

  // Initialize cloud properties
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Initialize DHT22
  dht.begin();
  pinMode(RELAY_OUT, OUTPUT);
  pinMode(RELAY_OUT2, OUTPUT);  // Initialize the new relay pin
  digitalWrite(RELAY_OUT, LOW);
  digitalWrite(RELAY_OUT2, LOW);  // Set the new relay to OFF
  pump_status = false;
  pump2_status = false;  // Initialize new pump status

  // Initialize Ultrasonic Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize LED pins
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  // Display initial message on LCD
  lcd.setCursor(0, 0);
  lcd.print("project 2024");
  lcd.setCursor(0, 1);
  lcd.print("smart irrigation");
  delay(2000); // Wait for 2 seconds
  lcd.clear();
}

void loop() {
  ArduinoCloud.update();
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  // Update cloud variables
  temperaturepin = temp;
  humiditypin = hum;
  soilMoistureValue = analogRead(soilMoisturePin);
  soilMoisturePercent = map(soilMoistureValue, DryValue, WetValue, 0, 100);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  soilmoisturepin = soilMoisturePercent;

  sunIntensityValue = analogRead(sunIntensityPin);
  sunIntensityPercent = map(sunIntensityValue, 0, 2000, 0, 100);
  sunintensitypin = sunIntensityPercent;

  if (soilMoisturePercent <= triger_level) {
    pumpOn();
  } else {
    pumpOff();
  }

  // Measure water level
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  waterLevel = (duration / 2) / 29.1; // Convert the time into cm
  waterLevel = constrain(waterLevel, 1, 18); // Adjust for your tank measurements
  waterlevelpin = map(waterLevel, 1, 20, 100, 0); // Map 6cm to 100% and 18cm to 0%

  // Control LED indicators based on water level
  if (waterlevelpin <= 20) {
    digitalWrite(RED_LED_PIN, HIGH);    // Red LED on
    digitalWrite(GREEN_LED_PIN, LOW);   // Green LED off
    digitalWrite(BLUE_LED_PIN, LOW);    // Blue LED off
  } else if (waterlevelpin > 20 && waterlevelpin <= 90) {
    digitalWrite(RED_LED_PIN, LOW);     // Red LED off
    digitalWrite(GREEN_LED_PIN, HIGH);  // Green LED on
    digitalWrite(BLUE_LED_PIN, LOW);    // Blue LED off
  } else {
    digitalWrite(RED_LED_PIN, LOW);     // Red LED off
    digitalWrite(GREEN_LED_PIN, LOW);   // Green LED off
    digitalWrite(BLUE_LED_PIN, HIGH);   // Blue LED on
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    showFirstSet = !showFirstSet;
    lcd.clear();
  }

  // Update LCD display
  if (showFirstSet) {
    lcd.setCursor(0, 0);
    lcd.print("Temperature: ");
    lcd.print(temp);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Humidity:  ");
    lcd.print(hum);
    lcd.print("%");

    lcd.setCursor(0, 2);
    lcd.print("Soil Moisture: ");
    lcd.print(soilMoisturePercent);
    lcd.print("%");

    lcd.setCursor(0, 3);
    lcd.print("Pump Status: ");
    lcd.print(pump_status_text);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Sun Intensity: ");
    lcd.print(sunIntensityPercent);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("Water Level: ");
    lcd.print(waterlevelpin);
    lcd.print("%");

    lcd.setCursor(0, 2);
    lcd.print("Pump 2 Status: ");
    lcd.print(pump2_status_text);  // Display new pump status
  }

  delay(2000); // Update every 2 seconds
}

void pumpOn() {
  digitalWrite(RELAY_OUT, HIGH);
  pump_status_text = "ON";
  pump_status = true;
  Serial.println("Pump ON");
}

void pumpOff() {
  digitalWrite(RELAY_OUT, LOW);
  pump_status_text = "OFF";
  pump_status = false;
  Serial.println("Pump OFF");
}

void pump2On() {
  digitalWrite(RELAY_OUT2, HIGH);
  pump2_status_text = "ON";
  pump2_status = true;
  Serial.println("Pump 2 ON");
}

void pump2Off() {
  digitalWrite(RELAY_OUT2, LOW);
  pump2_status_text = "OFF";
  pump2_status = false;
  Serial.println("Pump 2 OFF");
}

void dht_sensor_getdata() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  temperaturepin = temp;
  humiditypin = hum;
  soilMoistureValue = analogRead(soilMoisturePin);
  soilMoisturePercent = map(soilMoistureValue, DryValue, WetValue, 0, 100);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  soilmoisturepin = soilMoisturePercent;
}

void onSoilmoisturepinChange() {
  // Add your code here to act upon Soilmoisturepin change
}

void onTrigerLevelChange() {
  // Add your code here to act upon TrigerLevel change
}

void onPumpStatusChange() {
  // Add your code here to act upon PumpStatus change
  if (pump_status) {
    pumpOn();
  } else {
    pumpOff();
  }
}

void onPump2StatusChange() {
  // Add your code here to act upon Pump2Status change
  if (pump2_status) {
    pump2On();
  } else {
    pump2Off();
  }
}

void onHumiditypinChange() {
  // Code to handle changes in humiditypin
}

void onTemperaturepinChange() {
  // Code to handle changes in temperaturepin
}

void onWaterlevelpinChange() {
  // Code to handle changes in waterlevelpin
}
