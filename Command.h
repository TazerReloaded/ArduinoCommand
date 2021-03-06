#ifndef ARDUINO_COMMAND_h
#define ARDUINO_COMMAND_h
#ifndef ARDUINO_COMMAND_BUFFER_SIZE
#define ARDUINO_COMMAND_BUFFER_SIZE 32
#endif
#ifndef ARDUINO_COMMAND_ARGS_SIZE
#define ARDUINO_COMMAND_ARGS_SIZE 5
#endif
#ifndef ARDUINO_COMMAND_NAME_SIZE
#define ARDUINO_COMMAND_NAME_SIZE 16
#endif
#ifndef ARDUINO_COMMAND_ECHO
#define ARDUINO_COMMAND_ECHO false
#endif

// lookup table for crc16-ccit, stored in flash memory
const uint16_t crc16_ccitt_table[256] PROGMEM = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

// get the crc16_ccitt of the given buffer from program memory
uint16_t crc16_ccitt(const char *data, const uint16_t length) {
    uint16_t crc = 0xffff;
    for (uint16_t count = 0; count < length; ++count) {
        crc = pgm_read_word(&crc16_ccitt_table[(*data++ ^ (crc >> 8)) & 0xff]) ^ (crc << 8);
    }
    return crc;
}

class ArduinoCommand {
    public:
        typedef struct {
            char Command[ARDUINO_COMMAND_NAME_SIZE];
            void (*Callback)(const uint8_t argc, const char **argv);
        } ArduinoCommandInfo;
        ArduinoCommand(Stream *stream) {
            mStream = stream;
        }
        void setCommands(const ArduinoCommandInfo *commands, uint8_t commandCount) {
            mCommands = commands;
            mCommandCount = commandCount;
        }
        // checks for new data, should be called every loop()
        void read() {
            // check if we have data waiting
            while (mStream->available()) {
                // handle potential overflow
                if (mBufferPos >= ARDUINO_COMMAND_BUFFER_SIZE) {
                    respond_P(false, PSTR("in_buffer_overflow"));
                    mBufferPos = 0;
                }
                
                // store buffer byte into buffer
                mBuffer[mBufferPos] = mStream->read();
                if (ARDUINO_COMMAND_ECHO) {
                    // echo received bytes back to sender
                    mStream->write(mBuffer[mBufferPos]);
                }
                
                if (mBuffer[mBufferPos] == 13 || mBuffer[mBufferPos] == 10) {
                    // delimiter detected (CR or LF), handle command
                    processCommand(mBufferPos + 1);
                    // reset position
                    mBufferPos = 0;
                }
                else {
                    // increment counter
                    mBufferPos++;
                }
            }
        }
        // prints just an ok response without any data, equivalent to respond(true, NULL)
        void respondOK() {
            processResponse(true, true, NULL, NULL);
        }
        // prints a response with format string in RAM
        void respond(const bool success, const char *format, ...) {
            va_list args;
            va_start(args, format);
            processResponse(false, success, format, args);
            va_end(args);
        }
        // prints a response with format string in PROGMEM
        void respond_P(const bool success, const char *format, ...) {
            va_list args;
            va_start(args, format);
            processResponse(true, success, format, args);
            va_end(args);
        }
    private:
        #if ARDUINO_COMMAND_BUFFER_SIZE > 256
            uint16_t mBufferPos = 0;
        #else
            uint8_t mBufferPos = 0;
        #endif
        Stream *mStream;
        const ArduinoCommandInfo *mCommands;
        uint8_t mCommandCount = 0;
        char mBuffer[ARDUINO_COMMAND_BUFFER_SIZE];
        const char *mArgs[ARDUINO_COMMAND_ARGS_SIZE];
        void processResponse(const bool formatInProgmem, const bool success, const char *format, va_list args) {
            bool result;
            // write message content to buffer
            if (format != NULL) {
                // we have a format, write status text
                sprintf_P(mBuffer, success ? PSTR("ok ") : PSTR("err "));
                mBufferPos = ARDUINO_COMMAND_BUFFER_SIZE - strlen(mBuffer);
                // process format string and args
                if (formatInProgmem) {
                    result = checkSnprintf(
                        vsnprintf_P(&mBuffer[strlen(mBuffer)], mBufferPos, format, args),
                        mBufferPos
                    );
                } else {
                    result = checkSnprintf(
                        vsnprintf(&mBuffer[strlen(mBuffer)], mBufferPos, format, args),
                        mBufferPos
                    );
                }
                if (!result) {
                    // buffer write failed, quit here
                    return;
                }
            } else {
                // no format specified, just print status text
                sprintf_P(mBuffer, success ? PSTR("ok") : PSTR("err"));
            }
            // calculate crc over buffer content and append
            mBufferPos = ARDUINO_COMMAND_BUFFER_SIZE - strlen(mBuffer);
            result = checkSnprintf(
                snprintf_P(
                    &mBuffer[strlen(mBuffer)],
                    mBufferPos,
                    PSTR(" *%x\n"),
                    crc16_ccitt(mBuffer, strlen(mBuffer))
                ),
                mBufferPos
            );
            if (!result) {
                // buffer write failed, quit here
                return;
            }
            // write full buffer (preamble, message, crc) to stream
            mStream->write(mBuffer, strlen(mBuffer));
            // reset position and free args
            mBufferPos = 0;
        }
        bool checkSnprintf(int written, int n) {
            if (written < 0) {
                // cannot parse format string
                respond_P(false, PSTR("format_error"));
                return false;
            } else if (written > n) {
                // string does not fit in buffer
                respond_P(false, PSTR("out_buffer_overflow"));
                return false;
            }
            return true;
        }
        void processCommand(int len) {
            // insert null byte instead of delimiter at the end
            mBuffer[len - 1] = 0x00;

            // check CRC if present
            char *crc = strchr(mBuffer, '*');
            if (crc != NULL) {
                crc++;
                if (strtoul(crc, NULL, 16) != crc16_ccitt(mBuffer, len - strlen(crc) - 3)) {
                    respond_P(false, PSTR("crc_mismatch"));
                    return;
                }
            }

            // evaluate command
            for (uint8_t i = 0; i < mCommandCount; i++) {
                if (strstr_P(mBuffer, mCommands[i].Command) == mBuffer) {
                    // clear the args and loop over every arg separated by a space
                    // by using a string tokenizer which replaces spaces by null bytes
                    for (int j = 0; j < ARDUINO_COMMAND_ARGS_SIZE; j++) {
                        mArgs[j] = NULL;
                    }
                    uint8_t pos = 0;
                    char *ptr = strtok_P(&mBuffer[strlen_P(mCommands[i].Command)], PSTR(" "));
                    while (ptr != NULL) {
                        // check for overflow and skip CRC
                        if (pos >= ARDUINO_COMMAND_ARGS_SIZE) {
                            respond_P(false, PSTR("args_overflow"));
                            return;
                        } else if (crc != NULL && ptr == crc - 1) {
                            break;
                        }
                        // find next token until end
                        mArgs[pos++] = ptr;
                        ptr = strtok_P(NULL, PSTR(" "));                        
                    }
                    mCommands[i].Callback(pos, mArgs);
                    return;
                }
            }
            respond_P(false, PSTR("unknown_command"));
        }
};

#endif
