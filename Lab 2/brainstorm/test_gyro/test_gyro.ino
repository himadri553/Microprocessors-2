#include <Wire.h>

const int MPU_addr = 0x68;  // I2C address (0x69 if AD0 pulled high)
int16_t GyX, GyY, GyZ;

void setup() {
  Wire.begin();               // join I2C bus
  Serial.begin(9600);         // start serial for output
  while (!Serial);            // wait for Serial Monitor (for Mega/Leonardo)

  // Wake up MPU6050 (clear sleep bit)
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Serial.println("MPU6050 test started...");
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true);  // 3 axes * 2 bytes each

  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  // Print results
  Serial.print(" | GyX="); Serial.print(GyX);
  Serial.print(" | GyY="); Serial.print(GyY);
  Serial.print(" | GyZ="); Serial.println(GyZ);

  delay(500);
}