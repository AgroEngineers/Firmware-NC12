#include <Arduino.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define BAUD_RATE 115200
#define MAX_BIND_SERVOS 8
#define MAX_BIND_MOTORS 10

#define ARDUINOJSON_DEPRECATED_5

Servo servos[MAX_BIND_SERVOS];
uint8_t servoCount = 0;

int motors[MAX_BIND_MOTORS][3];
int motorCount = 0;
int motorSpeed = 128;

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

void registerMotor(const uint8_t forward, const uint8_t backward, const uint8_t speed) {
    pinMode(forward, OUTPUT);
    pinMode(backward, OUTPUT);
    pinMode(speed, OUTPUT);

    motors[motorCount][0] = forward;
    motors[motorCount][1] = backward;
    motors[motorCount][2] = speed;
    motorCount++;
}

void deregisterServos() {
    for (auto &&servo : servos) {
        servo.detach();
    }
    servoCount = 0;
}

void deregisterMotors() {
    for (auto &&motor : motors) {
        motor[0] = 0;
        motor[1] = 0;
    }
    motorCount = 0;
}


void setGate(const uint8_t gate, const uint8_t angle) {
    servos[gate].write(angle);
}

void setDirection(const uint8_t direction){
    for (auto motor : motors){
        digitalWrite(motor[0], LOW);
        digitalWrite(motor[1], LOW);
        analogWrite(motor[2], 0);
    }
    switch (direction) {
        case 1:
            for (auto motor : motors){
                digitalWrite(motor[0], HIGH);
                analogWrite(motor[2], motorSpeed);
            }
            break;
        case 2:
            for (auto motor : motors){
                digitalWrite(motor[1], HIGH);
                analogWrite(motor[2], motorSpeed);
            }
            break;
        default:
            break;
    }
}

void parseJson(const char *json) {
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(json);

    if (!root.success()) {
        Serial.println("Parse failed");
        return;
    }

    const char* task = root["task"];

    if (!task) {
        Serial.println("No task");
        return;
    }

    // ---------- SERVOS ----------
    if (strcmp(task, "servos") == 0) {
        JsonArray &json_servos = root["servos"];

        Serial.println("Servos:");
        deregisterServos();

        for (auto &&servo : json_servos) {
            uint8_t s = servo;
            registerServo(s);

            Serial.print(s);
            Serial.print("; ");
        }
        Serial.println();
    }

        // ---------- MOTORS ----------
    else if (strcmp(task, "motors") == 0) {
        JsonArray &json_motors = root["motors"];

        Serial.println("Motors:");
        deregisterMotors();

        for (auto &&motor : json_motors) {
            JsonArray &pair = motor;

            uint8_t forward = pair[0];
            uint8_t backward = pair[1];
            uint8_t speed = pair[2];

            registerMotor(forward, backward, speed);

            Serial.print(forward);
            Serial.print(", ");
            Serial.print(backward);
            Serial.print(", ");
            Serial.print(speed);
            Serial.print("; ");
        }
        Serial.println();
    }

    // ---------- SPEED ----------
    else if (strcmp(task, "speed") == 0) {
        int speed = root["speed"];
        motorSpeed = speed;
        Serial.print("Speed: ");
        Serial.println(speed);
    }

        // ---------- GATE ----------
    else if (strcmp(task, "gate") == 0) {
        uint8_t gate = root["gate"];
        uint8_t angle = root["angle"];

        Serial.print("Gate ");
        Serial.print(gate);
        Serial.print(": ");
        Serial.println(angle);

        setGate(gate, angle);
    }

        // ---------- DIRECTION ----------
    else if (strcmp(task, "direction") == 0) {
        uint8_t direction = root["direction"];

        Serial.print("Direction: ");
        Serial.println(direction == 1 ? "Forward" : (direction == 2 ? "Backward" : "None"));
        setDirection(direction);
    }

    else {
        Serial.println("Unknown task");
    }
}
void setup() {
    servoCount = 0;
    motorCount = 0;

    Serial.begin(BAUD_RATE);
    Serial.println("Machine ready!");
}

void loop() {
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line.length() > 0) {
            parseJson(line.c_str());
        }
    }
}
