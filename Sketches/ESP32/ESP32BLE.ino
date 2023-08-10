#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <HardwareSerial.h>

//HardwareSerial Serial2(2); // Use hardware UART on ESP32 (Serial2)
const int relay = 26;
const int RSSI_THRESHOLD = -70; // Adjust this threshold as needed
bool stop;

// Bluetooth variables
BLEScan* pBLEScan;
bool deviceFound = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Print the MAC address and RSSI of the detected device
    Serial.print("Detected device MAC: ");
    Serial.println(advertisedDevice.getAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(advertisedDevice.getRSSI());

    // Check if the device's MAC address matches the target
    if (advertisedDevice.getAddress().equals(BLEAddress("dc:a8:44:df:41:f2")) &&
        advertisedDevice.getRSSI() >= RSSI_THRESHOLD) {
      Serial.println("Target device found within range!");
      deviceFound = true;
    }
  }
};

void setup() {
  Serial.begin(250000);
  Serial2.begin(250000, SERIAL_8N1, 16, 17); // Start UART communication on pins 16 (TX2) and 17 (RX2)
  //pinMode(relay, OUTPUT);

  // Initialize Bluetooth
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {
  // Check if there's any data available in the serial buffer
  while (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Read the incoming command

    if (command == "toggle") {
      // Toggle the deviceFound flag
      deviceFound = !deviceFound;
      Serial.println("Toggled deviceFound flag.");
    }
  }

  // Start scanning for Bluetooth devices
  pBLEScan->start(5, false); // Scan for 5 seconds

      Serial2.println("light_on"); // Sending "light_on" command to Arduino
    delay(5000);

    //digitalWrite(relay, HIGH); //mette in corto COM e NC
    Serial2.println("light_off"); // Sending "light_off" command to Arduino
    delay(5000);

  // If the target device is found within range, let the current flow
  if (deviceFound) {
    //digitalWrite(relay, LOW); //mette in corto COM e NO
    Serial2.println("light_on"); // Sending "light_on" command to Arduino
    delay(5000);
  } else {
    //digitalWrite(relay, HIGH); //mette in corto COM e NC
    Serial2.println("light_off"); // Sending "light_off" command to Arduino
    delay(5000);
  }

  // Reset deviceFound flag
  deviceFound = false;
}

