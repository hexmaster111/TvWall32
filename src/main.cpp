#include <Arduino.h>
#include <TMC2209.h>

// This example will not work on Arduino boards without HardwareSerial ports,
// such as the Uno, Nano, and Mini.
//
// See this reference for more details:
// https://www.arduino.cc/reference/en/language/functions/communication/serial/
//
// To make this library work with those boards, refer to this library example:
// examples/UnidirectionalCommunication/SoftwareSerial

HardwareSerial &serial_stream = Serial2;

const uint8_t Y_STEP_PIN = 0;
const uint8_t Y_DIRECTION_PIN = 4;
const uint8_t X_STEP_PIN = 14;
const uint8_t X_DIRECTION_PIN = 27;

const uint32_t STEP_COUNT = 8000;
const uint16_t STOP_DURATION = 1;
const uint16_t STEP_TIME = 100;
const uint8_t POWER_BYTE = 100; // 0-255

enum edirection
{
    POSITIVE = 1,
    NEGITIVE = -1
};

// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage

// Instantiate TMC2209 -- this one handles talking to both of the drivers
TMC2209 stepper_driver;

// Stepper positions
int32_t g_x, g_y;
int g_x_dir, g_y_dir; // -1 neg, +1 pos

#define STEP_X()                                            \
    do                                                      \
    {                                                       \
        digitalWrite(X_STEP_PIN, !digitalRead(X_STEP_PIN)); \
        g_x += g_x_dir;                                     \
        delayMicroseconds(STEP_TIME);                       \
    } while (0)

#define SET_X_DIR(POL)                                                   \
    do                                                                   \
    {                                                                    \
        if (POL != g_x_dir)                                              \
        {                                                                \
            digitalWrite(X_DIRECTION_PIN, POL == POSITIVE ? HIGH : LOW); \
            g_x_dir = POL;                                               \
        }                                                                \
    } while (0)

#define STEP_Y()                                            \
    do                                                      \
    {                                                       \
        digitalWrite(Y_STEP_PIN, !digitalRead(Y_STEP_PIN)); \
        g_y += g_y_dir;                                     \
        delayMicroseconds(STEP_TIME);                       \
    } while (0)

#define SET_Y_DIR(POL)                                                   \
    do                                                                   \
    {                                                                    \
        if (POL != g_y_dir)                                              \
        {                                                                \
            digitalWrite(Y_DIRECTION_PIN, POL == POSITIVE ? HIGH : LOW); \
            g_y_dir = POL;                                               \
        }                                                                \
    } while (0)

void setup_and_enable_stepper_at_low_power()
{
    stepper_driver.setup(serial_stream);
    stepper_driver.setAllCurrentValues(20, 10, 10);
    stepper_driver.enableCoolStep();
    stepper_driver.setPwmOffset(120);
    stepper_driver.enable();
}

#define SLOW_STEP_DELAY (50)

void enable_and_do_homing()
{
    setup_and_enable_stepper_at_low_power();

    SET_X_DIR(NEGITIVE);
    SET_Y_DIR(NEGITIVE);

    // run x y into the zero stop
    for (int i = 0; i < 100000; i++)
    {
        STEP_X();
        STEP_Y();
        delayMicroseconds(SLOW_STEP_DELAY);
    }

    SET_X_DIR(POSITIVE);
    SET_Y_DIR(POSITIVE);

    // move Y forward onto our screens zero position
    for (int i = 0; i < 10000 /*steps to get to zero*/; i++)
    {
        STEP_Y();
        delayMicroseconds(SLOW_STEP_DELAY);
    }

    // move X forward onto our screens zero position
    for (int i = 0; i < 40000 /*steps to get to zero*/; i++)
    {
        STEP_X();
        delayMicroseconds(SLOW_STEP_DELAY);
    }

    g_x = 0;
    g_y = 0;
}

#define SIDELEN (500)

void draw_rectangle_outline_forever()
{
    while (true)
    {

        SET_X_DIR(POSITIVE);
        SET_Y_DIR(POSITIVE);

        for (int i = 0; i < SIDELEN; i++)
            STEP_X();
        for (int i = 0; i < SIDELEN; i++)
            STEP_Y();

        SET_X_DIR(NEGITIVE);
        SET_Y_DIR(NEGITIVE);

        for (int i = 0; i < SIDELEN; i++)
            STEP_X();
        for (int i = 0; i < SIDELEN; i++)
            STEP_Y();
    }
}

void SetCursorPosition(int x, int y)
{
    Serial.printf("SetCursorPosition(x:%d,y: %d, g_x=%d)\n", x, y, g_x);
    SET_X_DIR(g_x > x ? NEGITIVE : POSITIVE);
    SET_Y_DIR(g_y > y ? NEGITIVE : POSITIVE);

    Serial.printf("x: %s\n", g_x > x ? "NEGITIVE" : "POSITIVE");

    while (g_x != x || g_y != y)
    {
        if (g_x != x)
        {
            STEP_X();
        }

         if (g_y != y)
        {
            STEP_Y();

        }
    }
}

void WriteChar(char c)
{
}

void setup()
{
    Serial.begin(9600);
    pinMode(X_STEP_PIN, OUTPUT);
    pinMode(X_DIRECTION_PIN, OUTPUT);

    pinMode(Y_STEP_PIN, OUTPUT);
    pinMode(Y_DIRECTION_PIN, OUTPUT);

    g_x_dir = POSITIVE;
    g_y_dir = POSITIVE;
    digitalWrite(X_DIRECTION_PIN, HIGH);
    digitalWrite(Y_DIRECTION_PIN, HIGH);

    enable_and_do_homing();

    SET_X_DIR(POSITIVE);
    SET_Y_DIR(POSITIVE);

    // stepper_driver.setAllCurrentValues(100, 10, 10);
    // stepper_driver.setPwmOffset(POWER_BYTE);
    while (true)
    {

        SetCursorPosition(10000, 0);
        delay(1000);

        SetCursorPosition(0, 0);
        delay(1000);
        // SetCursorPosition(0, 0);
        // WriteChar('H');
        // WriteChar('E');
        // WriteChar('L');
        // WriteChar('L');
        // WriteChar('O');
        // WriteChar(' ');
        // WriteChar('W');
        // WriteChar('O');
        // WriteChar('R');
        // WriteChar('L');
        // WriteChar('D');
    }
}

void loop()
{

    // One step takes two iterations through the for loop
    for (uint32_t i = 0; i < STEP_COUNT * 2; ++i)
    {
        digitalWrite(X_STEP_PIN, !digitalRead(X_STEP_PIN));
        digitalWrite(Y_STEP_PIN, !digitalRead(Y_STEP_PIN));
        delayMicroseconds(1);
    }
    digitalWrite(X_DIRECTION_PIN, !digitalRead(X_DIRECTION_PIN));
    digitalWrite(Y_DIRECTION_PIN, !digitalRead(Y_DIRECTION_PIN));
    delay(STOP_DURATION);
}