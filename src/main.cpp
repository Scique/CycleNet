#include <Arduino.h>
#include <GY521.h>
#include <Sim800L.h>
#include <SoftwareSerial.h>
#include <string>
#include <map>
#include <cmath>
#include <array>

// Create variables for the sensors

// GY521
GY521 accel(0x68);
uint32_t counter = 0;
float ax, ay, az;
float gx, gy, gz;
float t;
const float g = 9.81;
bool fallen = false;
// Store the thresholds - orientation threshold is if not upright so + and - in that direction compared to upright
byte thresholds[] = {0.5 * g, 1.5 * g, 40};
// Whether the thresholds have been passed
bool testThreshold[] = {false, false, false};
// Counts the amount of time passed since the threshold of that number has been passed
byte thresholdBreakCount[] = {0, 0, 0};


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
    Serial.println("Serial has begun");
    Wire.begin();

    
    // Setup GY-521
    accel.wakeup();
    accel.setAccelSensitivity(0);  //  2g
    accel.setGyroSensitivity(0);   //  250 degrees/s
    accel.setThrottle(false);
    // Set all calibration errors to zero
	// Overwrites the values of axe aye aze gxe gye gze
	Serial.println("Accelerometer will begin calibration - please do not move until instructed that the accelerometer is calibrated. ");
    accel.calibrate(100);
	Serial.println("Accelerometer calibration complete. ");
    

    /*
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
    */

}

// Loop - self explanatory after the execution of the setup
void loop() {
    if (fallen) {
		// Do something about them falling
	}
	else {
		checkFall();
	}
	delay(100);

	// POST location to server
    
}




// Functions

// Fall detection
static void checkFall() {
    /* Algorithm: 
    If the magnitude exceeds the upper threshold within a short time frame after exceeding the lower threshold, a fall is likely detected.

    Collect data from the module:
    1. Calculate magnitude of acceleration.
    2. Check if the value of magnitude breaks the lower threshold(threshold 0) in 0.5 seconds. If false, return to Step 1.
    3. Check if the magnitude breaks the higher threshold(threshold 1) in 0.5 seconds. If false, return to Step 1.
    4. Check for change in orientation within 0.5 seconds(threshold 2). If false, return to Step 1.
    5. Check if orientation remains same for 1 second. If false, return to Step 1.
    6. Change the output pin as HIGH on the detection of fall (if all of the above steps are resulted as true).
    */

    // Define thresholds

    // Set all the acceleration values to 0
    ax = ay = az = 0;
    gx = gy = gz = 0;
    t = 0;

    // Get all the current acceleration and gyroscopic values
    accel.read();
    ax = accel.getAccelX();
    ay = accel.getAccelY();
    az = accel.getAccelZ();
    gx = accel.getGyroX();
    gy = accel.getGyroY();
    gz = accel.getGyroZ();

	// Calculate the overall acceleration of the object w/ pythagorean theorem 
	// acceleration total magnitude; all values should be within 0 to 1 - multiplied by 10 for easier use
	float am = (sqrt((ax*ax) + (ay*ay) + (az*az)))*10;
	// Calculate change in orientation w/ pythagorean theorem
	int orientationChange = sqrt((gx*gx) + (gy*gy) + (gz*gz));
	

	// Test for threshold 3 - if passed - fall has been detected
	if (testThreshold[2]) {
		thresholdBreakCount[2]++;
		// Check if orientation is held(change no more than 10 degrees) for longer than 1 seconds
		if (thresholdBreakCount[2] > 10 && orientationChange <= 10) {
			fallen = true;
			Serial.println("FALL DETECTED");
		}
		// User didn't fall
		else if (orientationChange > 10) {
			testThreshold[2] = false;
			thresholdBreakCount[2] = 0;
		}
	}
	

	// Check if 2nd threshold testing and check for 3rd threshold conditions
	if (testThreshold[1]){
		thresholdBreakCount[1]++;
		// If the orientation angle has changed more than 100 degrees and t2 has been longer than 0.5 - enable testing for t3
		if (orientationChange >= thresholds[2] && thresholdBreakCount[1] > 5) {
			testThreshold[2] = true;
			Serial.println("Threshold 3 testing begun");
			Serial.println(orientationChange);
		}
		// If t2 has not been broken longer than 0.5s and the required condiitons for t2 are not met - no fall
		else if (am <= thresholds[1]) {
			testThreshold[1] = false;
			thresholdBreakCount[1] = 0;
		}
	}

	// Check if 1st threshold testing and check for 2nd threshold conditions
	if (testThreshold[0]) {
		thresholdBreakCount[0]++;
		// If required conditions and if t1 has been broken for longer than 0.5s - enable t2
		if (am >= thresholds[1] && thresholdBreakCount[0] > 5){
			testThreshold[1] = true;
			Serial.println("Threshold 2 testing begun");
			Serial.println(am);
		}
		// If t1 has not been broken longer than 0.5s and the required condiitons for t2 are not met - no fall
		else if (am <= thresholds[0]) {
			testThreshold[0] = false;
			thresholdBreakCount[0] = 0;
		}
	}

	// Check for 1st threshold - skip checking if 2nd threshold alr passed
	if (am >= thresholds[0] && !testThreshold[1]) {
		testThreshold[0] = true;
		Serial.println("Threshold 1 testing begun");
		Serial.println(am);
	}
}
