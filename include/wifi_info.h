#pragma once

#include <ESP8266WiFi.h>

const char *ssid = "your-ssid";
const char *password = "your-password";

void wifi_connect() {
	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.begin(ssid, password);
	Serial.println("WiFi connecting...");
	while (!WiFi.isConnected()) {
		delay(100);
		Serial.print(".");
	}
	Serial.print("\n");
	Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
}
