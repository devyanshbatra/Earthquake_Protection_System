#include <WiFi.h>
#include <HTTPClient.h>


const char* ssid = "your_SSID";        // Replace with your Wi-Fi SSID
const char* password = "your_PASSWORD"; // Replace with your Wi-Fi password

// Server endpoint
const char* serverName = "http://example.com/data"; // Replace with your server URL

// Define the analog pin where the LM35 is connected
const int lm35Pin = 34;  // GPIO34 (you can use any ADC-capable pin)

// Define the digital pin for controlling the relay or switch
const int switchPin = 23; // GPIO23 (you can use any available digital pin)
const int switchPin2 = 24; 
// Define the digital pin for the frequency sensor
const int freqSensorPin = 22; // GPIO22 (you can use any available digital pin)

// Define the analog pin for the seismometer sensor
const int seismoPin = 32; // GPIO32 (you can use any ADC-capable pin)

// Temperature threshold in Celsius
const float temperatureThreshold = 40.0;

// Earthquake magnitude threshold
const float earthquakeMagnitudeThreshold = 5.0;

// Variables to store the analog reading and calculated temperature
int sensorValue = 0;
float temperatureC = 0.0;

// Variables for frequency measurement
volatile unsigned long pulseCount = 0; // Pulse count for frequency calculation
unsigned long lastMeasurementTime = 0; // Last time measurement was done
const unsigned long measurementInterval = 1000; // Interval in milliseconds for frequency calculation
float frequency = 0.0; // Frequency in Hz

// Variables for seismometer data
int seismoValue = 0;
float earthquakeMagnitude = 0.0;

void IRAM_ATTR onPulse() {
  // Increment pulse count on each rising edge
  pulseCount++;
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Set the ADC width and the ADC resolution
  analogReadResolution(12); // ESP32 ADC resolution: 12 bits (0-4095)
  
  // Initialize the switch pin as an output
  pinMode(switchPin, OUTPUT);
  
  // Ensure the switch is turned on initially (assuming active HIGH)
  digitalWrite(switchPin, HIGH);
  
  // Initialize the frequency sensor pin as an input with interrupt
  pinMode(freqSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(freqSensorPin), onPulse, RISING);
  
  // Initialize the seismometer pin as an input
  pinMode(seismoPin, INPUT);
  
  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  
  // Initialize the measurement time
  lastMeasurementTime = millis();
}

void loop() {
  // Read the analog value from the LM35 sensor
  sensorValue = analogRead(lm35Pin);
  
  // Convert the analog reading to a voltage (0-3.3V)
  float voltage = sensorValue * (3.3 / 4095.0);
  
  // Convert the voltage to temperature in Celsius
  temperatureC = voltage * 100.0; // LM35 output is 10mV/°C
  
  // Print the temperature to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");
  
  // Check if the temperature exceeds the threshold
  if (temperatureC > temperatureThreshold) {
    // Turn off the switch (assuming active HIGH for the relay or transistor)
    digitalWrite(switchPin, LOW);
    Serial.println("Switch turned OFF due to high temperature.");
  } else {
    // Turn on the switch (assuming active HIGH for the relay or transistor)
    digitalWrite(switchPin, HIGH);
  }
  
  // Calculate frequency every measurementInterval
  unsigned long currentTime = millis();
  if (currentTime - lastMeasurementTime >= measurementInterval) {
    // Disable interrupts to safely read pulseCount
    noInterrupts();
    float elapsedTime = (currentTime - lastMeasurementTime) / 1000.0; // Convert milliseconds to seconds
    frequency = pulseCount / elapsedTime;
    pulseCount = 0; // Reset pulse count
    lastMeasurementTime = currentTime; // Update last measurement time
    interrupts();
    
    // Print the frequency to the Serial Monitor
    Serial.print("Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");
    
    // Send data to server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      String postData = "temperature=" + String(temperatureC) + "&frequency=" + String(frequency) + "&earthquakeMagnitude=" + String(earthquakeMagnitude);
      int httpResponseCode = http.POST(postData);
      
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Server Response: " + response);
      } else {
        Serial.println("Error sending POST request: " + String(httpResponseCode));
      }
      
      http.end();
    } else {
      Serial.println("WiFi not connected");
    }
  }
  
  // Read the analog value from the seismometer sensor
  seismoValue = analogRead(seismoPin);
  
  // Convert the seismometer reading to magnitude (you may need to adjust this conversion based on your sensor)
  earthquakeMagnitude = seismoValue * (3.3 / 4095.0); // Example conversion, adjust as needed
  
  // Print the earthquake magnitude to the Serial Monitor
  Serial.print("Earthquake Magnitude: ");
  Serial.println(earthquakeMagnitude);
  
  // Check if the earthquake magnitude exceeds the threshold
  if (earthquakeMagnitude > earthquakeMagnitudeThreshold) {
    // Activate the switch (assuming active HIGH to turn on the current)
    digitalWrite(switchPin2, HIGH);
    Serial.println("Switch activated due to high earthquake magnitude.");
  } else {
    // Deactivate the switch
    digitalWrite(switchPin2, LOW);
  }
  
  // Delay before the next reading
  delay(1000); // Short delay to prevent overwhelming the Serial Monitor
}
