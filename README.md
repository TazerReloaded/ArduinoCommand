# ArduinoCommand

This library can be used to control Arduino-projects with text commands, over any connection that extends the "Stream" base class (Serial, Wire, Ethernet, ...). Commands are sent from an external host (like your computer) to the Arduino, where a callback function is executed that returns its results back immediately.

## Advantages:
* self contained header-only library with no dependencies (except Arduino.h)
* super easy to use, just open a terminal and start typing
* user-defined commands with a configurable amount of parameters
* extremely high flexibility because packet format is defined in callback functions and can be different for each command
* support for any data type that has an ASCII representation, like string, int, float, bool
* verification using CRC16-CCITT with lookup table for extra speed
* all buffers have a fixed size and are reused for optimal RAM utilisation

## Disadvantages:
* overall data usage is a lot higher than with a binary protocol, because with the ASCII representation, only about half of the capacity of each byte is used, e.g. 0x7d7b in binary (2 bytes) is converted to 32123 (5 bytes). Some conversions are not that bad, strings remain the same size because they are already ASCII
* processing will take slightly longer, because the strings need to be parsed for evaluation, but with the speed of most chips, that shouldn't be an issue
* some data types may need some tricks for more complex data, because the space symbol (`0x20`) is used as an argument separator, the newline symbol (`0x0A`) as packet boundary and the asterisk (`0x2A`) as CRC marker

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
// include comes after defines because it uses them internally
#include "ArduinoCommand.h"

// this variable is written from a command
uint8_t mode = 0;

// start ArduinoCommand on the default serial port
ArduinoCommand cmd(&Serial);
// define our strings in PROGMEM to save RAM
const char MSG_FORMAT_TIME[]   PROGMEM = "%d %d %d";
const char MSG_FORMAT_INT[]    PROGMEM = "%d";
const char MSG_FORMAT_STRING[] PROGMEM = "%s";
const char MSG_SAY_HELLO[]     PROGMEM = "Hello World!";
const char MSG_ERR_NOARGS[]    PROGMEM = "no_arguments";
// define our commands in PROGMEM to save even more RAM
const ArduinoCommand::ArduinoCommandInfo commands[] PROGMEM = {
    { "get time", [](const uint8_t argc, const char ** argv) {
        // ... get data from somewhere here, maybe an external rtc module ...
        int hour = 15;
        int minute = 30;
        int second = 25;
        // print formatted data
        cmd.respond_P(true, MSG_FORMAT_TIME, hour, minute, second);
    }},
    { "get sensor count", [](const uint8_t argc, const char ** argv) {
        // this variable should come from somewhere else
        // in the real program of course
        int numSensors = 7;
        // all output is formatted using the standard sprintf()-syntax
        cmd.respond_P(true, MSG_FORMAT_INT, numSensors);
    }},
    { "say hello", [](const uint8_t argc, const char ** argv) {
        // you can print strings without variables in them
        cmd.respond_P(true, MSG_SAY_HELLO);
    }},
    { "echo back", [](const uint8_t argc, const char ** argv) {
        // this function just writes a given parameter back
        // you can of course do whatever you want with the parameters,
        // like parsing them to int with atoi() for example
        if (argc > 0) {
            cmd.respond_P(true, MSG_FORMAT_STRING, argv[0]);
        } else {
            // no parameter given, return an error with "false"
            cmd.respond_P(false, MSG_ERR_NOARGS);
        }
    }},
    { "set mode", [](const uint8_t argc, const char ** argv) {
        // you can also set global variables or call class members from here
        mode = atoi(argv[0]);
        // some responses don't need a message
        cmd.respondOK();
    }}
};

void setup() {
    // open serial port
    Serial.begin(115200);

    // assign commands to command processor
    // the compiler calculates the command count for us
    cmd.setCommands(commands, sizeof(commands) / sizeof(*commands))
}

void loop() {
    // read and process data whenever available.
    // this method blocks while your callback executes,
    // so don't use delay() in them
    cmd.read();
}
```

## Q&A
**Q**: Why?  
**A**: Because I wanted a simple, reusable communications library with error checking that works without any additional software directly from the serial monitor.

**Q**: Can I increase the buffer size?  
**A**: Yes, by specifying for example `#define ARDUINO_COMMAND_BUFFER_SIZE 64` before the include (default is 32). Please don't go lower than 32, because internal error messages need some space.

**Q**: Can I use more than 5 arguments?  
**A**: Yes, by increasing the command buffer size, by specifying for example `#define ARDUINO_COMMAND_ARGS_SIZE 10` before the include (default is 5).

**Q**: Can I use fancy commands, like CamelCase or underscore_separated?  
**A**: Yes, commands can be any string, just be sure that they don't overlap, because only the first match is executed. Commands are limited to 15 characters, if you need longer commands, override `#define ARDUINO_COMMAND_NAME_SIZE 16` with your desired value. Notice however that every command will use that space in PROGMEM, regardless of the actual command length.

**Q**: How does the CRC thing work and how do I implement it on the host side?  
**A**: I use the standard CRC16/CCITT-FALSE algorithm, there are libraries and sample implementations for almost any language out there. You can use sites like [https://crccalc.com/] to verify your CRCs. The CRC is calculated over the whole message, excluding the terminating newline, then a space and an asterisk followed by the CRC in hex representation are appended to the message. So if your packet is `get status\n`, it should look like this with CRC: `get status *65a0\n`. The hex representation is case insensitive. CRC is not required for requests, but checked if present. If the request CRC does not match, you'll get a `crc_mismatch` response and the command is ignored.

**Q**: I can't use float values in format strings, `"%f"` will just print `"?"`!  
**A**: I had the same problem, but that's not caused by my library but because the standard Arduino library has floating point formatting stripped to save memory. If your chip can handle the extra ~1.5kB added by full printf support, you can add the compiler flags `-Wl,-u,vfprintf -lprintf_flt -l`. This will tell the linker to include the standard C implementation rather than the one provided by Arduino. You can also use a built-in function like `dtostrf()` and pass the result as a string (`"%s"`). Or you could build your own function and use some math tricks to get the representation you want.
