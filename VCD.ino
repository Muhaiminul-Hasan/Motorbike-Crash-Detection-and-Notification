#include <Wire.h>
#include <SoftwareSerial.h>

#define MPU6050_ADDR 0x68  // I2C address of MPU6050
#define GSM_TX_PIN 7       // Connect TX from GSM shield to pin 7
#define GSM_RX_PIN 8       // Connect RX from GSM shield to pin 8

SoftwareSerial gsmSerial(GSM_TX_PIN, GSM_RX_PIN);

int16_t ax, ay, az;  // Accelerometer readings
int crashThreshold = 2000;  // Adjust this threshold based on your requirements

void setup() {
  Serial.begin(9600);
  gsmSerial.begin(9600);
  
  Wire.begin();
  MPU6050_Init();
}

void loop() {
  Read_Accelerometer();

  // Check for crash condition
  if (DetectCrash()) {
    SendCrashNotification();
    delay(5000); // Wait for 5 seconds to avoid sending multiple notifications
  }
}

void MPU6050_Init() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void Read_Accelerometer() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true);
  
  // Read accelerometer values
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
}

bool DetectCrash() {
  // Calculate total acceleration
  int totalAcc = abs(ax) + abs(ay) + abs(az);

  // Check if the total acceleration exceeds the threshold
  return totalAcc > crashThreshold;
}

void SendCrashNotification() {
  // Enable GPS
  gsmSerial.println("AT+CGPSPWR=1");
  delay(1000);

  // Get GPS data
  gsmSerial.println("AT+CGPSINFO");
  delay(2000);

  // Read GPS data
  while (gsmSerial.available()) {
    char c = gsmSerial.read();
    Serial.write(c);
  }

  // Send crash notification SMS
  gsmSerial.println("AT+CMGF=1");  // Set SMS to text mode
  delay(1000);
  gsmSerial.println("AT+CMGS=\"+1234567890\"");  // Replace with the recipient's phone number
  delay(1000);
  gsmSerial.print("Vehicle crash detected at location: ");
  
  // You can customize the SMS message to include GPS coordinates or any other relevant information
  
  // Send Ctrl+Z to indicate the end of the message
  gsmSerial.write(26);
  delay(1000);
}
