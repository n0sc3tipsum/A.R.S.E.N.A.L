#ifndef NETCODE_H
#define NETCODE_H

#include <WebServer.h>

// Extern variables to be shared with other files
extern float tilt;
extern float setpoint;
extern float CurrSpeed;
extern float SetSpeed;

// Function declarations
void setupNetwork(const char* ssid, const char* password);
void handleRoot();
void handleSet();

WebServer server(80);

void setupNetwork(const char* ssid, const char* password) {
    // Connecting to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    // Set up server endpoints
    server.on("/", handleRoot);
    server.on("/update_set", handleSet);
    server.begin();
    Serial.println("HTTP server started");
}

void handleRoot() {
    unsigned long timeInSeconds = millis() / 1000; // Get time in seconds since the ESP32 started
    String json = "{";
    json += "\"tiltX\":" + String(tilt, 6) + ","; // Ensure floating-point precision
    json += "\"speed\":" + String(CurrSpeed) + ",";
    json += "\"setpoint\":" + String(setpoint) + ",";
    json += "\"time\":" + String(timeInSeconds);
    json += "}";
    server.send(200, "application/json", json);
}

void handleSet() {
    Serial.println("Received request to update setpoint"); // Debugging statement
    if (server.hasArg("set")) {
        SetSpeed = server.arg("set").toFloat();
        Serial.println("Updated setpoint to: " + String(SetSpeed)); // Debugging statement
        server.send(200, "text/plain", "Setpoint updated");
    } else {
        Serial.println("Invalid setpoint value received"); // Debugging statement
        server.send(400, "text/plain", "Invalid setpoint value");
    }
}

#endif // NETCODE_H