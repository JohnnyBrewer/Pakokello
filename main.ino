/******************************************************************************************************************************************
*                                         3D manufacturing and fabrication                                                                *
*     								Joonas Jyrkkä, Matias Vehkaoja, Jukka Posio 												          *
*       									  Oulun yliopisto 2016      															      *
******************************************************************************************************************************************/

#include "MPU6050.h" 									// Accelerometer library for direct raw data https://github.com/jrowberg/i2cdevlib
#include "Wire.h" 										// Wire communication (I2C / TWI) library
#include "I2Cdev.h" 									// Advanced library for I2C communication  https://github.com/jrowberg/i2cdevlib

#define LED_PIN 13 										// Built in LED on Arduino
#define L_GEAR 6 										// Left gear pin that leads to transistor base (current switch)
#define R_GEAR 5										// Right gear pin that leads to transistor base (current switch)
#define PIEZO 4 										// Piezo pin
#define SLAVE_ADDRESS 0x69 								// MPU6050 I2C slave address

MPU6050 accelgyro(SLAVE_ADDRESS);						// Set accelgyro SLAVE_ADDRESS and initialize the class
int16_t axis_x;											// 16-bit variable to store x-axis data
unsigned long currMillis = 0;							// Stores the current time to compare it with previous change state
unsigned long prevMillis = 0;							// Stores the last time LED_PIN was updated
unsigned long alarmMillis = 5000;						// Delay in milliseconds to start alarming. Due to this is proof of consept, there is no actual alarm clock functionality
boolean triggered = false;								// Boolean value to tell if the alarm has triggered or not
boolean ledState = false;								// LED_PIN current state


void setup() {											// Setup function that is ran only once upon boot
    Wire.begin();										// Begin I2C transmission
    accelgyro.initialize();								// Initialize the accelgyro and start it
    pinMode(LED_PIN, OUTPUT);       					// Setup LED_PIN as OUTPUT                                   
    pinMode(L_GEAR, OUTPUT);							// Setup L_GEAR as OUTPUT
    pinMode(R_GEAR, OUTPUT);							// Setup R_GEAR as OUTPUT
    digitalWrite(L_GEAR, HIGH);
    digitalWrite(R_GEAR, HIGH);
}

void update_status() {									// Update function to change the OUTPUT states of the various components
	prevMillis = currMillis;							// Previous change time is set to current change time
	ledState = !ledState;								// Change the ledState to opposite of the current value
	digitalWrite(LED_PIN, ledState);					// Update the ledState to the digital pin controlling the LED_PIN
	if (ledState == false && triggered == true) {		// If the alarm is triggered and the ledState is FALSE (Turned off)
		tone(PIEZO, 700);								// Run the piezo alarm at 700 Hz (little lower pitch)
	}
	else if (ledState == true && triggered == true) {	// If the ledState is TRUE (Turned on)
		tone(PIEZO, 1000);								// Run the piezo alarm at 1000 Hz (little higher pitch)
	}
}

void trigger_alarm() {									// Trigger function to turn on the alarm
	triggered = true;									// We have triggered the alarm, so set the boolean value to TRUE
	digitalWrite(L_GEAR, HIGH);							// Start moving L_GEAR
	digitalWrite(R_GEAR, HIGH);							// Start moving R_GEAR
}

void loop() {											// Loop function that gets repeated
	currMillis = millis();								// Read current run time
	axis_x = accelgyro.getAccelerationX();				// Read x-axis acceleration value
	
	if (currMillis - prevMillis >= 1000) {				// If substracting previous change time from current run time is equal or higher than 1000 milliseconds (1 second), call the update function
		update_status();								// Function call to change the ledState, update the change to the LED_PIN and alter the tone frequency depending on the previous state
	}
	
	if (currMillis >= alarmMillis) {					// If current run time is higher than alarmMillis (alarming delay), start the gears and piezo tone
		trigger_alarm();								// Function call to set alarm on
	}
	
    if (axis_x > 30000 ) {								// If the x-axis acceleration value is higher than 30'000 our clock robot has encountered a wall and crashed to it
														// Note: Change to axis_x < -30000 for reverse moving direction
		digitalWrite(L_GEAR, LOW);						// We crashed so stop moving L_GEAR
		digitalWrite(R_GEAR, LOW);						// We crashed so..
		triggered == false;								// We crashed so..
		while (accelgyro.getAccelerationY() < 30000) {	// If we do not get Y-axis movement (picking device up from the floor / table), do not quit making horrible tones!
			tone(PIEZO, 400);							// Piezo @ 400 Hz
			delay(250);									// 250ms delay
			tone(PIEZO, 600);							// Piezo @ 600 Hz
			delay(250);									// 250ms delay
			tone(PIEZO, 500);							// Piezo @ 500 Hz
			delay(250);									// 250ms delay
			tone(PIEZO, 700);							// Piezo @ 700 Hz
			delay(250);									// 250ms delay
		}
		noTone(PIEZO);									// So the user must have picked the device up.. Let's quit annoying them.
		while (true) {									// Let's go to infinite loop that we never ever return and start making horrible, horrible things again
			delay(1000);								// Delay loop that lasts exactly one second. Blink the led (for status) even that we have actually stopped the functionality.
			update_status();							// Blink the LED, triggered == FALSE
		}
    }
}
