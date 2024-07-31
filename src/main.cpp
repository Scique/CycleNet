#include <Arduino.h>
#include <GY521.h>
#include <Sim800L.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <cmath>
#include <array>
#include "arduino_secrets.h"
#include "WiFiS3.h"

// Create variables to use

// GY521
GY521 accel(0x68);
uint32_t counter = 0;
float ax, ay, az;
float gx, gy, gz;
float t;
const float g = 9.81;

// Global variables
bool fallen = false;
bool assistance = false;
bool buttonPressed = false;
byte assistanceTimer = 0;
// In the case that the individual has fallen over
bool emergencyMode = false;
// Store the thresholds - orientation threshold is if not upright so + and - in that direction compared to upright
byte thresholds[] = {0.5 * g, g, 40};
// Whether the thresholds have been passed
bool testThreshold[] = {false, false, false};
// Counts the amount of time passed since the threshold of that number has been passed
byte thresholdBreakCount[] = {0, 0, 0};
// Location information
float location[] = {0.0, 0.0};
// To send data to web server
const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;
// the WiFi radio's status
int status = WL_IDLE_STATUS;
const char server[] = "https://backend-nekansppvq-an.a.run.app";
WiFiClient client;

// NEO 6M GPS
// The serial connection to the GPS module
SoftwareSerial ss(5, 6);
TinyGPSPlus gps;

/*
// SIM 800L - (RX, TX)
Sim800L GSM(3, 2);
int day,month,year,minute,second,hour;
*/

/*
Wiring reference:
- GY-521 Accelerometer and Gyro Module

- SIM800L GSM
TX: 2(A)
RX: 3(A)

- NEO 6M GPS Module
RX: 4(PWM)
TX: 3(PWM)


- Button
PWM: 2
*/

// Functions

// Connects to the wifi
static void connectToNetwork()
{
	// check for the WiFi module:
	if (WiFi.status() == WL_NO_MODULE)
	{
		Serial.println("Communication with WiFi module failed!");
	}

	// Check for firmware
	String fv = WiFi.firmwareVersion();
	if (fv < WIFI_FIRMWARE_LATEST_VERSION)
	{
		Serial.println("Please upgrade the firmware");
	}

	// attempt to connect to WiFi network:
	while (status != WL_CONNECTED)
	{
		Serial.print("Attempting to connect to WPA SSID: ");
		Serial.println(ssid);
		// Connect to WPA/WPA2 network:
		status = WiFi.begin(ssid, pass);

		// wait 10 seconds for connection:
		delay(10000);
	}
	Serial.println("Network connection successful");
}

static void post_data()
{
	Serial.println("Posting data");
	// if you get a connection, report back via serial:
	if (client.connect(server, 8080))
	{
		Serial.println("connected to server");
		// Make a HTTP request:
		// Because C is such a terrible language, we have to do all this just to get the link - string concatenation somehow requires the importing of a funciton and a library
		client.print("POST /post_data/?bikeID=0&latitude=");
		client.print(location[0]);
		client.print("&longitude=");
		client.print(location[1]);
		client.print("&fallen=");
		client.println(emergencyMode);
		client.println("Host: https://backend-nekansppvq-as.a.run.app");
		client.println("Connection: close");
		client.println();
		client.println();
	}
}

// Fall detection
static void checkFall()
{
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
	float am = (sqrt((ax * ax) + (ay * ay) + (az * az))) * 10;
	// Calculate change in orientation w/ pythagorean theorem
	int orientationChange = sqrt((gx * gx) + (gy * gy) + (gz * gz));

	// Test for threshold 3 - if passed - fall has been detected
	if (testThreshold[2])
	{
		thresholdBreakCount[2]++;
		// Check if orientation is held(change no more than 10 degrees) for longer than 1 seconds
		if (thresholdBreakCount[2] > 10 && orientationChange <= 10)
		{
			fallen = true;
			Serial.println("FALL DETECTED");
		}
		// User didn't fall
		else if (orientationChange > 10)
		{
			testThreshold[2] = false;
			thresholdBreakCount[2] = 0;
		}
	}

	// Check if 2nd threshold testing and check for 3rd threshold conditions
	if (testThreshold[1])
	{
		thresholdBreakCount[1]++;
		// If the orientation angle has changed more than 100 degrees and t2 has been longer than 1 - enable testing for t3
		if (orientationChange >= thresholds[2] && thresholdBreakCount[1] > 10)
		{
			testThreshold[2] = true;
			Serial.println("Threshold 3 testing begun");
			Serial.println(orientationChange);
			testThreshold[1] = false;
			thresholdBreakCount[1] = 0;
		}
		// If t2 has not been broken longer than 1s and the required condiitons for t2 are not met - no fall
		else if (am <= thresholds[1])
		{
			Serial.println("Threshold 3 not passed - not a fall");
			testThreshold[1] = false;
			thresholdBreakCount[1] = 0;
		}
	}

	// Check if 1st threshold testing and check for 2nd threshold conditions
	if (testThreshold[0])
	{
		thresholdBreakCount[0]++;
		// If required conditions and if t1 has been broken for longer than 1s - enable t2
		if (am >= thresholds[1] && thresholdBreakCount[0] > 10)
		{
			testThreshold[1] = true;
			Serial.println("Threshold 2 testing begun");
			testThreshold[0] = false;
			thresholdBreakCount[0] = 0;
		}
		// If t1 has not been broken longer than 1s and the required condiitons for t2 are not met - no fall
		else if (am <= thresholds[0])
		{
			Serial.println("Threshold 2 not passed - not a fall");
			testThreshold[0] = false;
			thresholdBreakCount[0] = 0;
		}
	}

	// Check for 1st threshold - skip checking if 2nd threshold alr passed
	if (am >= thresholds[0] && !testThreshold[1] && !testThreshold[0])
	{
		testThreshold[0] = true;
		Serial.println("Threshold 1 testing begun");
	}
	Serial.println(am);
}

// Counts the time - if they don't respond within one minute that means they need assistance
static void assistanceNeeded()
{
	assistanceTimer++;
	// Check if the button is pressed
	buttonPressed = digitalRead(2);
	// If they don't say that they're okay with the button within one minute
	if (assistanceTimer > 600)
	{
		emergencyMode = true;
	}
}

static void getLocation()
{
	if (ss.available() == 0)
	{
		if (gps.location.isValid())
		{
			if (gps.encode(Serial.read()))
			{
				Serial.print("Latitude: ");
				Serial.println(gps.location.lat(), 6);
				Serial.print("Longitude: ");
				Serial.println(gps.location.lng(), 6);
				Serial.print("Speed in m/s = ");
				Serial.println(gps.speed.mps());

				// Update the location in our variable
				location[0] = gps.location.lat();
				location[1] = gps.location.lng();
			}
			else
			{
				Serial.println("Location: Not Available");
			}
		}
		else{
			Serial.println("Location invalid");
		}
	}
	else{
		Serial.println("SS not available. ");
	}
}

// Setup - runs once when Arduino is booted
void setup()
{
	// Begin serial data transmission
	Serial.begin(9600);
	Serial.println("Serial has begun");
	Wire.begin();

	// Setup GY-521
	Serial.print("Accel is connected: ");
	Serial.println(accel.isConnected());
	if (accel.wakeup())
	{
		Serial.println("Sensor successfully connected");
	}
	else
	{
		while (!accel.wakeup())
		{
			Serial.println("Could not connect to GY521 module - check connection, attempting to connect");
			accel.wakeup();
		}
	}
	accel.setAccelSensitivity(0); //  2g
	accel.setGyroSensitivity(0);  //  250 degrees/s
	accel.setThrottle(false);
	// Set all calibration errors to zero
	// Overwrites the values of axe aye aze gxe gye gze
	Serial.println("Accelerometer will begin calibration - please do not move until instructed that the accelerometer is calibrated. ");
	accel.calibrate(100);
	Serial.println("Accelerometer calibration complete. ");

	// Button setup
	pinMode(2, INPUT);

	// Neo 6M GPS setup
	ss.begin(9600);

	/*
	// Make sure it can get the gps
	while(!gps.location.isValid()) {
		while(ss.available() > 0) {
			if (gps.encode(ss.read())){
				Serial.println("Attempting to setup GPS");
			}
		}
	}
	*/
	Serial.println("Signal successfully collected");
	connectToNetwork();

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
void loop()
{
	// In the event that the user has fallen and is in need of assistance, it should not be wasting any battery power on checking other things
	if (emergencyMode)
	{
		Serial.println("--EMERGENCY--");
		Serial.println("Continually transmitting location information. ");
	}
	// Put any conditions to check when isn't emergency mode in here
	else
	{
		if (fallen && !buttonPressed)
		{
			// Do something about them falling
			Serial.println("USER NEEDS ASSISTANCE");
			assistanceNeeded();
		}
		else
		{
			Serial.println("\n\nNormal\n");
			checkFall();
		}
	}

	// GPS Location stuff
	// Updates the location array -> [latitude, longitutde]
	getLocation();

	// Send the information to the web server
	post_data();
}
