#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
}

// format: uint8; HAP section 9.30; 0 = opened, 1 = closed
homekit_characteristic_t current_door_state = HOMEKIT_CHARACTERISTIC_(CURRENT_DOOR_STATE, 1);

// format: uint8; HAP section 9.118; 0 = opened, 1 = closed
homekit_characteristic_t target_door_state = HOMEKIT_CHARACTERISTIC_(TARGET_DOOR_STATE, NULL);

// format: string; HAP section 9.62; max length 64
homekit_characteristic_t cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "GarageDoorOpener");

homekit_accessory_t *accessories[] = {
	HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_garage, .services=(homekit_service_t*[]) {
		HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
			HOMEKIT_CHARACTERISTIC(NAME, "GarageDoorOpener"),
			HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Arduino HomeKit"),
			HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
			HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266/ESP32"),
			HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
			HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
			NULL
		}),
		HOMEKIT_SERVICE(GARAGE_DOOR_OPENER, .primary=true, .characteristics=(homekit_characteristic_t*[]){
			&current_door_state,
			&target_door_state,
			&cha_name,
			NULL
		}),
		NULL
	}),
	NULL
};

homekit_server_config_t config = {
	.accessories = accessories,
	.password = "111-11-111"
};
