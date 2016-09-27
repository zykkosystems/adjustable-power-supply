/* Adjustable Power Supply
 * Luke Ashford 2016
 */

#include <SPI.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// Chip Select Pins
const int CSADC = 9;
const int CSDAC = 10;
const int ENABLE2675 = 7;

// I2C LCD pins
#define I2C_ADDR    0x27
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// LEDs
const int SHUTDOWN_LED = 3;
const int CURLIM_LED = 4;

void setPinModes()
{
    pinMode(CSADC, OUTPUT);
    pinMode(CSDAC, OUTPUT);
    pinMode(ENABLE2675, OUTPUT);
    digitalWrite(CSADC, HIGH);
    digitalWrite(CSDAC, HIGH);
    digitalWrite(ENABLE2675, LOW);

    pinMode(SHUTDOWN_LED, OUTPUT);
    pinMode(CURLIM_LED, OUTPUT);
    digitalWrite(SHUTDOWN_LED, HIGH);
    digitalWrite(CURLIM_LED, LOW);
}

void initLCD()
{
    lcd.begin(16,2);
    lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
    lcd.setBacklight(HIGH);
    lcd.home();
}

void initSPI()
{
    SPI.setClockDivider(SPI_CLOCK_DIV4); // slow the SPI bus down
    SPI.setBitOrder(MSBFIRST);  // Most significant bit arrives first
    SPI.setDataMode(SPI_MODE0);    // SPI 0,0 as per MCP4821/2 data sheet
    SPI.begin();
}

/*
 * channel - 0, 1, 2 or 3
 * returns: int value 0 up to & including 4095
 */
int readAdc(int channel)
{
    byte b0 = 0;
    b0 |= 1 << 2; // Start bit
    b0 |= 1 << 1; // Single-ended

    byte b1 = 0;
    b1 |= (channel >= 2) << 7;
    b1 |= (channel % 2) << 6;

    int output = 0;

    digitalWrite(CSADC, LOW);
    SPI.transfer(b0);
    output = SPI.transfer(b1) << 8;
    output |= SPI.transfer(0);
    digitalWrite(CSADC, HIGH);

    return output & 4095;
}

/*
 * channel - 0 or 1; gainEnable - 1 to enable x2 gain; value - 0 up to & including 4095
 */
void writeDac(int channel, boolean gainEnable, int value)
{
    byte b = 0;
    b |= ((boolean)(channel)) << 7;
    b |= !gainEnable << 5;
    b |= 1 << 4;
    b |= ((value & 4095) >> 8);

    digitalWrite(CSDAC, LOW);
    SPI.transfer(b);
    SPI.transfer((byte)(value & 255));
    digitalWrite(CSDAC, HIGH);
}

/*
 * Generates the required input value for the 2675 reg in order to produce the desired target output
 * targetmV - 1250 up to & including 10 000
 * returns: int value 0 up to & including 4095
 */
int generateChannel0Output(int targetmV)
{
    return 5421 - (348/797 * (targetmV + 2040));
}

/*
 * Generates the required input value for the 317 reg in order to produce the desired target output
 * targetmV - 1250 up to & including 10 000
 * returns: int value 0 up to & including 4095
 */
int generateChannel1Output(int targetmV)
{
    return 10 * ((targetmV - 1249) / 22);
}

void setup() 
{
    setPinModes();
    initSPI();
    initLCD();

    lcd.clear();
    lcd.home();
    lcd.print("Hello world");

    
    digitalWrite(ENABLE2675, HIGH);

    // 6V
    writeDac(0, 1, 1910);
    writeDac(1, 1, 2159);
}

void loop() 
{
    int ch0 = readAdc(0);
    int ch1 = readAdc(1);

    lcd.clear();
    lcd.home();
    lcd.print(ch0);
    lcd.setCursor(0, 1);
    lcd.print(ch1);
    delay(1000);
}

