#include <Arduino.h>
#include <GY521.h>
#include <Sim800L.h>
#include <SoftwareSerial.h>

// Create variables for the sensors

// GY521
GY521 accel(0x68);
uint32_t counter = 0;
float ax, ay, az;
float gx, gy, gz;
float t;


// SIM 800L - (RX, TX)
Sim800L GSM(3, 2);
int day,month,year,minute,second,hour;

/*
Wiring reference: 
- GY-521 Accelerometer and Gyro Module

- SIM800L GSM 
TX: 2
RX: 3(PWM)

- NEO 6M GPS Module

- Button


*/
// Setup - runs once when Arduino is booted
void setup() {
    // Begin serial data transmission
    Serial.begin(9600);
    Serial.println("niggers");
    Wire.begin();

    /*
    // Setup GY-521
    accel.wakeup();
    accel.setAccelSensitivity(0);  //  2g
    accel.setGyroSensitivity(0);   //  250 degrees/s
    accel.setThrottle(false);
    // Set all calibration errors to zero
    accel.axe = 0;
    accel.aye = 0;
    accel.aze = 0;
    accel.gxe = 0;
    accel.gye = 0;
    accel.gze = 0;
    */


    // Setup SIM800L GSM
    GSM.begin(9600);
    // Full functionality(mode 1)
    if (!GSM.setFunctionalityMode(1)) {
        Serial.println("Full functionality");
    }
    else {
        Serial.println("Error");
    }
    Serial.print("Functionality mode: ");
    Serial.println(GSM.getFunctionalityMode());
    delay(1000);

    Serial.println("GET PRODUCT INFO: ");
    Serial.println(GSM.getProductInfo());

    Serial.println("GET OPERATORS LIST: ");
    Serial.println(GSM.getOperatorsList());

    Serial.println("GET OPERATOR: ");
    Serial.println(GSM.getOperator());

}

// Loop - self explanatory after the execution of the setup
void loop() {
    // isFall();
    
}




// Functions