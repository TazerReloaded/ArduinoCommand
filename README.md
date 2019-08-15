# ArduinoCommand

This library can be used to control Arduino-projects with text commands, over any connection that extends the "Stream" base class (Serial, Wire, Ethernet, ...). Commands are sent from an external host (like your computer) to the Arduino, where a callback function is executed that returns its results back immediately.

## Advantages:
* self contained header-only library with no dependencies (except Arduino.h)
* super easy to use, just open a terminal and start typing
* user-defined commands with a configurable amount of parameters
* verification using CRC16-CCITT with lookup table for extra speed
* all buffers have a fixed size and are reused for optimal RAM utilisation

## Example communication:
```
Request:  get status
Response: ok idle *c379

Request:  get sensor 1
Response: ok 25.7 *d0ba

Request:  set time 15 30 27 *ecb8
Response: ok *dbd6

Request:  get banana
Response: err unknown_command *12e
```

The request CRC is optional and only checked if present. Successful commands return `ok <result> *<CRC>`, failed commands return `err <message> *<CRC>`.

## Working example:
```c
// echo all typed bytes back, disabled by default
#define ARDUINO_COMMAND_ECHO true
// number of defined commands, required for fixed buffer sizes,
// update this number whenever you add or delete commands
#define ARDUINO_COMMAND_SIZE 4
// include comes after defines because it uses them internally
#include "ArduinoCommand.h"

// start ArduinoCommand on the default serial port
ArduinoCommand cmd(&Serial);

// define callbacks, don't forget the PSTR() around format strings!
// you get two parameters, the number of arguments and the arguments as strings
const void getTime(const uint8_t argc, const char **argv) {
    // ... get data from somewhere here, maybe an external rtc module ...
    cmd.printResponse(true, PSTR("%u %u %u"), t.hour, t.minute, t.second);
}
const void getSensorCount(const uint8_t argc, const char **argv) {
    // all output is formatted using the standard sprintf()-syntax
    cmd.printResponse(true, PSTR("%d"), NUM_SENSORS);
}
const void sayHello(const uint8_t argc, const char **argv) {
    // you can print strings without variables in them
    cmd.printResponse(true, PSTR("Hello World!"));
}
const void echoBack(const uint8_t argc, const char **argv) {
    // this function just writes a given parameter back
    // you can of course do whatever you want with the parameters
    if (argc > 0) {
        cmd.printResponse(true, PSTR("%s"), argv[0]);
    } else {
        // no parameter given, return an error with "false"
        cmd.printResponse(false, PSTR("no_arguments"));
    }
}

void setup() {
    // open serial port
    Serial.begin(115200);

    // define commands, with PSTR() again to store strings in flash
    cmd.addCommand(PSTR("get time"),         getTime);
    cmd.addCommand(PSTR("get sensor count"), getSensorCount);
    cmd.addCommand(PSTR("say hello"),        sayHello);
    cmd.addCommand(PSTR("echo back"),        echoBack);
}

void loop() {
    // read and process data whenever available.
    // this method blocks while your callback executes,
    // so don't use delay() there
    cmd.read();
}
```