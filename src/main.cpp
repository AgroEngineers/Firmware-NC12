#include <Arduino.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define BAUD_RATE 115200
#define MAX_BIND_SERVOS 8
#define MAX_BIND_MOTORS 10

#define ARDUINOJSON_DEPRECATED_5

Servo servos[MAX_BIND_SERVOS];
uint8_t servoCount = 0;

int motors[MAX_BIND_MOTORS][2];
int motorCount = 0;

auto input = "";

void registerServo(const int pin) {
    uint8_t outPin;

    switch (pin) {
        case 1: outPin = OUT1;
            break;
        case 2: outPin = OUT2;
            break;
        case 3: outPin = OUT3;
            break;
        case 4: outPin = OUT4;
            break;
        case 5: outPin = OUT5;
            break;
        case 6: outPin = OUT6;
            break;
        case 7: outPin = OUT7;
            break;
        case 8: outPin = OUT8;
            break;
        default: outPin = pin;
    }

    servos[servoCount].attach(outPin);
    servoCount++;
}

void registerMotor(const uint8_t forward, const uint8_t backward) {
    pinMode(forward, OUTPUT);
    pinMode(backward, OUTPUT);

    motors[motorCount][0] = forward;
    motors[motorCount][1] = backward;
    motorCount++;
}

void deregisterServos() {
    for (auto &&servo : servos) {
        servo.detach();
    }
}

void deregisterMotors() {
    for (auto &&motor : motors) {
        motor[0] = 0;
        motor[1] = 0;
    }
}

void parseJson(const char *json) {
    StaticJsonBuffer<256> jsonBuffer;

    JsonObject &root = jsonBuffer.parseObject(json);

    if (!root.success()) {
        Serial.println("Parse failed");
        return;
    }

    // ---------- SERVOS ----------
    JsonArray &json_servos = root["servos"];

    if (json_servos.success()) {
        Serial.println("Servos: ");
        deregisterServos();
        for (auto &&servo: json_servos) {
            const uint8_t s = servo;

            registerServo(s);

            Serial.print(s);
            Serial.print("; ");
        }
    }

    // ---------- MOTORS ----------
    JsonArray &json_motors = root["motors"];

    if (json_motors.success()) {
        Serial.println("Motors:");
        deregisterMotors();
        for (auto &&motor: json_motors) {
            JsonArray &pair = motor;

            const uint8_t forward = pair[0];
            const uint8_t backward = pair[1];

            registerMotor(forward, backward);

            Serial.print(forward);
            Serial.print(", ");
            Serial.print(backward);
            Serial.print("; ");
        }
    }
}

void setGate(const uint8_t gate, const uint8_t angle, const bool inverted) {
    servos[gate].write(inverted ? 180 - angle : angle);
}

void setup() {
    servoCount = 0;
    motorCount = 0;

    Serial.begin(BAUD_RATE);
    Serial.println("Machine ready!");
}

void loop() {
    while (Serial.available()) {
        const char c = static_cast<char>(Serial.read());

        if (c == '\n') {
            parseJson(input);
            input = "";
        } else {
            input += c;
        }
    }
}
