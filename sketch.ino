#include <Stepper.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int stepsPerRevolution = 200;  // Number of steps per revolution for your motor
const int dhtPin = 2;               // Pin where DHT22 is connected
const int oneWireBus = 3;           // Pin where the Dallas Temperature sensor is connected
const int ldrPin = A0;              // Pin where LDR is connected (analog pin)
const int pirPin = 4;               // Pin where PIR sensor is connected
const int buzPin = 5;

DHT dht(dhtPin, DHT22);             // Initialize DHT22 sensor
OneWire oneWire(oneWireBus);        // Initialize one-wire for Dallas sensor
DallasTemperature sensors(&oneWire); // Initialize Dallas Temperature sensor

// Initialize the stepper library on pins 8 through 11
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

// Initialize I2C LCD display with address 0x27 (common I2C address for many LCDs)
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 LCD display

void setup() {
  // Set the speed at 60 rpm for the stepper motor
  myStepper.setSpeed(60);
  
  // Initialize serial communication for debugging
  Serial.begin(9600);
  
  // Initialize the DHT sensor
  dht.begin();
  
  // Start up the Dallas Temperature sensor
  sensors.begin();

  // Initialize the PIR sensor pin as input
  pinMode(pirPin, INPUT);

  pinMode(buzPin, OUTPUT);
  
  // Initialize the LCD display
  lcd.begin(16, 2);   // Set the LCD to 16 columns and 2 rows
  lcd.clear();        // Clear any previous data on the screen
  
  // Turn on the backlight continuously
  lcd.backlight();
}

void loop() {
  // Read temperature and humidity from DHT22 sensor
  float temperature = dht.readTemperature();  // Temperature in Celsius
  float humidity = dht.readHumidity();        // Humidity percentage

  // Read temperature from the Dallas Temperature sensor
  sensors.requestTemperatures();
  float dallasTemperature = sensors.getTempCByIndex(0); // Get the first sensor's temperature

  // Read the LDR value (lux)
  int ldrValue = analogRead(ldrPin);  // Read the raw analog value
  float luxValue = map(ldrValue, 0, 1023, 0, 1000); // Convert to lux (simple linear map)

  // Check if DHT22 readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Check for PIR motion detection
  int motionDetected = digitalRead(pirPin);  // Read the PIR sensor value (HIGH means motion detected)

  // Print the current values to the serial monitor
  Serial.print("DHT22 Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C\t");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Dallas Temperature: ");
  Serial.print(dallasTemperature);
  Serial.print(" °C\t");
  Serial.print("LDR Value (Lux): ");
  Serial.println(luxValue);

  // Check if the DHT22 and Dallas Temperature readings are within their acceptable ranges
  bool isInRange = (temperature >= 15 && temperature <= 30 && humidity >= 50 && humidity <= 70);
  bool isDallasInRange = (dallasTemperature >= 16 && dallasTemperature <= 30);
  bool isLdrInRange = (luxValue >= 200 && luxValue <= 1000);

  // Update the LCD display based on PIR sensor input
  lcd.clear();
  if (motionDetected == HIGH) {
    lcd.setCursor(0, 0);
    lcd.print("Human or Animal");
    lcd.setCursor(0, 1);
    lcd.print("are near the plant");
    tone(buzPin,1000);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Dallas Temp: ");
    lcd.print(dallasTemperature);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("LDR Lux: ");
    lcd.print(luxValue);
    noTone(buzPin);
  }

  // Step the motor if any sensor is out of range
  if (isInRange && isDallasInRange && isLdrInRange) {
    Serial.println("All values are within range, motor will not step.");
  } else {
    Serial.println("Out of range, motor will step.");
    
    // Step one revolution in one direction (clockwise)
    myStepper.step(stepsPerRevolution);
    delay(500);

    // Step one revolution in the other direction (counterclockwise)
    myStepper.step(-stepsPerRevolution);
    delay(500);
  }

  // Small delay before reading again
  delay(2000);  // Wait for 2 seconds before taking new readings
}
