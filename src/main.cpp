#include <Arduino.h>
#include <arduino_homekit_server.h>

#include "wifi_info.h"

#define PIN_OPERATOR_CONTROL 14 // D5
#define PIN_MANUAL_CONTROL    5 // D1
#define PIN_RESET_POSITION    4 // D2

#define PIN_OPEN_STATE       12 // D6
#define PIN_CLOSED_STATE     13 // D7
#define PIN_WORKING_STATE_1  15 // D8
#define PIN_WORKING_STATE_2   0 // D3
#define PIN_WORKING_STATE_3   2 // D4

#define DOOR_STATE_OPEN      0
#define DOOR_STATE_CLOSED    1

#define DOOR_OPEN_DURATION   25000
#define DOOR_CLOSE_DURATION  20000
// #define DOOR_OPEN_DURATION   4000
// #define DOOR_CLOSE_DURATION  4000

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t current_door_state;
extern "C" homekit_characteristic_t target_door_state;

inline bool isDoorInSync()
{
	return current_door_state.value.uint8_value == target_door_state.value.uint8_value;
}

// Disable door state lights
inline void preOperation()
{
	digitalWrite(PIN_OPEN_STATE, LOW);
	digitalWrite(PIN_CLOSED_STATE, LOW);
}

inline void operateControl()
{
	digitalWrite(PIN_OPERATOR_CONTROL, HIGH);
	delay(500);
	digitalWrite(PIN_OPERATOR_CONTROL, LOW);
}

// Run door working light sequence
inline void awaitOperation()
{
	const uint32_t startTime = millis();
	const uint32_t duration = target_door_state.value.uint8_value == DOOR_STATE_OPEN ? DOOR_OPEN_DURATION : DOOR_CLOSE_DURATION;

	uint8_t lightIndex = 0;
	while (millis() - startTime < duration) {
		lightIndex = lightIndex % 3;
		uint8_t ix = target_door_state.value.uint8_value == DOOR_STATE_CLOSED ? lightIndex : 2 - lightIndex;

		digitalWrite(PIN_WORKING_STATE_1, ix == 0);
		digitalWrite(PIN_WORKING_STATE_2, ix == 1);
		digitalWrite(PIN_WORKING_STATE_3, ix == 2);

		delay(500);

		++lightIndex;
	}

	digitalWrite(PIN_WORKING_STATE_1, LOW);
	digitalWrite(PIN_WORKING_STATE_2, LOW);
	digitalWrite(PIN_WORKING_STATE_3, LOW);
}

// Notify HomeKit of the new door state
// Set the door state lights
inline void postOperation()
{
	current_door_state.value.uint8_value = target_door_state.value.uint8_value;
	homekit_characteristic_notify(&current_door_state, current_door_state.value);

	digitalWrite(PIN_OPEN_STATE, current_door_state.value.uint8_value == DOOR_STATE_OPEN);
	digitalWrite(PIN_CLOSED_STATE, current_door_state.value.uint8_value == DOOR_STATE_CLOSED);
}

void executeOperation()
{
	preOperation();
	operateControl();
	awaitOperation();
	postOperation();
}

// Called when setting target door state
void setter_TargetDoorState(const homekit_value_t value)
{
	target_door_state.value = value;
	if (!isDoorInSync()) {
		executeOperation();
	}
}

void handleManualControl() {
	static bool manualControlPressed = false;

	if (digitalRead(PIN_MANUAL_CONTROL)) {
		if (!manualControlPressed) {
			manualControlPressed = true;

			target_door_state.value.uint8_value = target_door_state.value.uint8_value == DOOR_STATE_OPEN ? DOOR_STATE_CLOSED : DOOR_STATE_OPEN;
			homekit_characteristic_notify(&target_door_state, target_door_state.value);

			if (!isDoorInSync()) {
				executeOperation();
			}
		}
	}
	else {
		manualControlPressed = false;
	}
}

void handleResetPosition() {
	static bool resetPositionPressed = false;

	if (digitalRead(PIN_RESET_POSITION)) {
		if (!resetPositionPressed) {
			resetPositionPressed = true;

			target_door_state.value.uint8_value = target_door_state.value.uint8_value == DOOR_STATE_OPEN ? DOOR_STATE_CLOSED : DOOR_STATE_OPEN;
			homekit_characteristic_notify(&target_door_state, target_door_state.value);

			postOperation();
		}
	}
	else {
		resetPositionPressed = false;
	}
}

void setup()
{
	pinMode(PIN_MANUAL_CONTROL, INPUT);
	pinMode(PIN_RESET_POSITION, INPUT);

	pinMode(PIN_OPERATOR_CONTROL, OUTPUT);
	pinMode(PIN_OPEN_STATE, OUTPUT);
	pinMode(PIN_CLOSED_STATE, OUTPUT);
	pinMode(PIN_WORKING_STATE_1, OUTPUT);
	pinMode(PIN_WORKING_STATE_2, OUTPUT);
	pinMode(PIN_WORKING_STATE_3, OUTPUT);

	Serial.begin(115200);
	wifi_connect();
	// homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example

	target_door_state.setter = setter_TargetDoorState;

	arduino_homekit_setup(&config);

	// Initialize current and target door state
	current_door_state.value.uint8_value = DOOR_STATE_CLOSED;
	homekit_characteristic_notify(&current_door_state, current_door_state.value);

	target_door_state.value.uint8_value = DOOR_STATE_CLOSED;
	homekit_characteristic_notify(&target_door_state, target_door_state.value);

	// Initialize door state lights
	digitalWrite(PIN_OPEN_STATE, LOW);
	digitalWrite(PIN_CLOSED_STATE, HIGH);
}

void loop()
{
	handleManualControl();
	handleResetPosition();

	arduino_homekit_loop();

	delay(10);
}
