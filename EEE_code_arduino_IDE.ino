#include <DHT.h>

#define DHTPIN 4          // DHT11 sensor connected to GPIO 4
#define DHTTYPE DHT11     // Type of sensor is DHT11
#define MOSFET_PIN 14     // GPIO pin to control the MOSFET
#define STOP_BUTTON 12    // GPIO pin for the stop button

DHT dht(DHTPIN, DHTTYPE);

bool isPeltierOn = false; // State to track if the Peltier is ON

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW); // Start with the Peltier OFF

  pinMode(STOP_BUTTON, INPUT_PULLUP); // Set up stop button
}

void loop() {
  float temperature = dht.readTemperature(); // Read temperature in °C
  float humidity = dht.readHumidity(); // Read humidity (optional)

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // If temperature exceeds 50°C and Peltier is OFF → Turn ON
  if (temperature > 50 && !isPeltierOn) {
    Serial.println("Temperature too high! Turning ON Peltier...");
    digitalWrite(MOSFET_PIN, HIGH); // Turn ON the MOSFET
    isPeltierOn = true;
  }

  // If the stop button is pressed → Turn OFF manually
  if (digitalRead(STOP_BUTTON) == LOW) {
    Serial.println("Stop button pressed. Turning OFF Peltier...");
    digitalWrite(MOSFET_PIN, LOW); // Turn OFF the MOSFET
    isPeltierOn = false;
    delay(500); // Debounce delay
  }

  delay(2000); // Update every 2 seconds
}