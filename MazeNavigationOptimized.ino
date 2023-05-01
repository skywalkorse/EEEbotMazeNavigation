//include all libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <MPU6050_tockn.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <list>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 15  // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 15  // Popular NeoPixel ring size

#define I2C_SLAVE_ADDR 0x04

//Create a objects for components
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

MPU6050 mpu6050(Wire);

//initialize all variables
int distance = 0;
float cmdDistance = 0;
float distanceCm;
int leftSpeed = 0;
int rightSpeed = 0;
int angle = 90;
long receivedValue1;
byte receivedByte1;
byte receivedByte2;

int forwardDistance;
int keyInt;
int listPos = 0;
float deviation = 0;
int distanceDeviation = 0;

bool FORWARD = false;
bool BACKWARD = false;
bool execute = false;
bool TURNLEFT = false;
bool TURNRIGHT = false;
bool menu = true;
bool ninety = false;
bool oneeighty = false;
bool turnInProcess = false;
bool driveInProcess = false;

//initialize the keypad
const byte ROWS = 4;  //four rows
const byte COLS = 3;  //three columns
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

byte rowPins[ROWS] = { 33, 32, 2, 0 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 5, 4, 18 };      //connect to the column pinouts of the keypad


Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//initialize command list
std::list<int> cmdList;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to

const int rs = 13, en = 12, d4 = 14, d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {

  Wire.begin();
  Serial.begin(9600);
  mpu6050.begin();
  lcd.begin(16, 2);
  lcd.print("MPU calibrating");
  mpu6050.calcGyroOffsets(true);
  lcd.clear();


  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
}

void loop() {
  pixels.clear();  // Set all pixel colors to 'off'
  mpu6050.update();
  float currentAngle = mpu6050.getAngleZ() + deviation;

  int cmd0 = 10;
  int cmd1 = 20;

  encoderRead();
  
  distanceCm = distance * 3.8;

  char key = keypad.getKey();


  if (menu) {
    
    lcd.setCursor(0, 0);
    lcd.print("MAZE NAV");
    lcd.setCursor(0, 1);
    for (int i : cmdList) {
      lcd.print(i);
    }


    for (int i = 0; i < NUMPIXELS; i++) {  // For each pixel...

      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    }
    pixels.show();  // Send the updated pixel colors to the hardware.
  }


  if (key) {
    if (key <= '9' && key >= '0') {
      keyInt = atoi(&key);
    }

    if (key == '2' && !(BACKWARD) && !(TURNRIGHT) && !(TURNLEFT)) {
      FORWARD = true;
      menu = false;
    }

    if (key == '8' && !(FORWARD) && !(TURNRIGHT) && !(TURNLEFT)) {
      BACKWARD = true;
      menu = false;
    }

    if (key == '*') {

      execute = true;
      menu = false;
      cmdList.push_back(99);
    }
    if (key == '4' && !(BACKWARD) && !(FORWARD) && !(TURNRIGHT)) {
      TURNLEFT = true;
      menu = false;
    }

    if (key == '6' && !(BACKWARD) && !(FORWARD) && !(TURNLEFT)) {
      TURNRIGHT = true;
      menu = false;
    }

    if (key == '0' && menu) {
      cmdList.clear();
      lcd.setCursor(0, 0);
      lcd.print("Clear commands");
      delay(1000);
      lcd.clear();
    }
  }

  if (FORWARD) {
    for (int i = 0; i < NUMPIXELS; i++) {  // For each pixel...

      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(200, 0, 0));
    }

    lcd.setCursor(0, 0);
    lcd.print("Forwards setting");
    lcd.setCursor(0, 1);
    lcd.print("1-9 means *10cm");


    if (key == '0' || key == '*') {
      lcd.clear();
      lcd.print("Invalid");
      delay(500);
      lcd.clear();
    }

    if (key == '#') {

      lcd.clear();
      delay(100);
      cmdList.push_back(cmd0 + keyInt);
      Serial.println("");
      FORWARD = false;
      menu = true;
    }
    pixels.show();  // Send the updated pixel colors to the hardware.
  }

  if (BACKWARD) {


    for (int i = 0; i < NUMPIXELS; i++) {  // For each pixel...

      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(0, 0, 200));
    }


    lcd.setCursor(0, 0);
    lcd.print("Backwards settin");
    lcd.setCursor(0, 1);
    lcd.print("g");
    lcd.print("1-9 means *10cm");




    if (key == '0' || key == '*') {
      lcd.clear();
      lcd.print("Invalid");
      delay(500);
      lcd.clear();
    }

    if (key == '#') {

      lcd.clear();
      delay(100);
      cmdList.push_back(cmd1 + keyInt);

      BACKWARD = false;
      menu = true;
    }
    pixels.show();  // Send the updated pixel colors to the hardware.
  }

  if (TURNLEFT) {
    lcd.setCursor(0, 0);
    lcd.print("Turn left");
    lcd.setCursor(0, 1);
    lcd.print("use 8 or 9");


    if (key == '9') {
      lcd.setCursor(0, 0);
      lcd.print("left 90°");
      ninety = true;
      lcd.clear();
    }
    if (key == '8') {
      lcd.setCursor(0, 0);
      lcd.print("left 180°");
      oneeighty = true;
      lcd.clear();
    }

    if (key == '#' && ninety) {
      lcd.clear();
      delay(100);
      cmdList.push_back(90);
      TURNLEFT = false;
      ninety = false;
      menu = true;
    }
  }

  if (TURNRIGHT) {
    lcd.setCursor(0, 0);
    lcd.print("Turn right");
    lcd.setCursor(0, 1);
    lcd.print("use 8 or 9");


    if (key == '9') {
      lcd.setCursor(0, 0);
      lcd.print("right 90°");
      ninety = true;
      lcd.clear();
    }
    if (key == '8') {
      lcd.setCursor(0, 0);
      lcd.print("right 180°");
      oneeighty = true;
      lcd.clear();
    }

    if (key == '#' && ninety) {
      lcd.clear();
      delay(100);
      cmdList.push_back(91);
      TURNRIGHT = false;
      ninety = false;
      menu = true;
    }
  }


  if (execute) {

    std::list<int>::iterator it = cmdList.begin();
    std::advance(it, listPos);
    int currentCmd = *it;
    int action = currentCmd / 10;
    cmdDistance = currentCmd % 10;

    if (currentCmd == 99) {

      execute = false;
      listPos = 0;
      lcd.clear();
      lcd.print("End Process");
      lcd.blink();
      delay(1000);
      lcd.noBlink();
      lcd.clear();
      menu = true;
    }

    if (action == 1) {
      if (cmdDistance * 10 > distanceCm) {
        driveInProcess = true;

        leftSpeed = 120;
        rightSpeed = 120;
        lcd.setCursor(0, 0);
        lcd.print(distanceCm);

      } else {
        driveInProcess = false;
        distance = 0;
        leftSpeed = 0;
        rightSpeed = 0;
        listPos++;
        delay(500);
      }
    }
    if (action == 2) {
      if (cmdDistance * -10 < distanceCm) {
        driveInProcess = true;
        leftSpeed = -120;
        rightSpeed = -120;
        lcd.setCursor(0, 0);
        lcd.print(distanceCm);

      } else {
        driveInProcess = false;
        distance = 0;
        leftSpeed = 0;
        rightSpeed = 0;
        listPos++;
        delay(500);
      }
    }
    if (currentCmd == 90) {
      for (int i = 13; i < NUMPIXELS; i++) {  // For each pixel...


        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        pixels.show();
      }

      for (int i = 13; i < NUMPIXELS; i++) {  // For each pixel...


        pixels.setPixelColor(i, pixels.Color(0, 0, 255));
        pixels.show();
      }
      turnInProcess = true;
      float angleTurn = -88;
      if (angleTurn < currentAngle) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(currentAngle);
        leftSpeed = 90;
        rightSpeed = 120;
        angle = 50;
      } else {
        turnInProcess = false;
        deviation += 90;
        angle = 90;

        leftSpeed = 0;
        rightSpeed = 0;
        listPos++;
      }
    }
    if (currentCmd == 91) {
      for (int i = 0; i < 3; i++) {  // For each pixel...


        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        pixels.show();
      }
      for (int i = 13; i < NUMPIXELS; i++) {  // For each pixel...


        pixels.setPixelColor(i, pixels.Color(0, 0, 255));
        pixels.show();
      }
      float angleTurn = 88;
      turnInProcess = true;
      if (angleTurn > currentAngle) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(currentAngle);
        leftSpeed = 120;
        rightSpeed = 90;
        angle = 130;
      } else {
        turnInProcess = false;
        deviation -= 90;
        angle = 90;
        leftSpeed = 0;
        rightSpeed = 0;
        listPos++;
      }
    }
  }
  drive(leftSpeed, rightSpeed, angle);
}


//drive function, able to move the EEEbot
void drive(int leftMotor, int rightMotor, int angle) {

  Wire.beginTransmission(I2C_SLAVE_ADDR);

  Wire.write((byte)((leftMotor & 0x0000FF00) >> 8));
  Wire.write((byte)(leftMotor & 0x000000FF));

  Wire.write((byte)((rightMotor & 0x0000FF00) >> 8));
  Wire.write((byte)(rightMotor & 0x000000FF));


  Wire.write((byte)((angle & 0x0000FF00) >> 8));
  Wire.write((byte)(angle & 0x000000FF));

  Wire.endTransmission();
  delay(100);
}

void encoderRead(){
  Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(byte) * 2);
  if (Wire.available() >= sizeof(byte) * 2) {

    // reading the data
    byte receivedByte1 = Wire.read();
    byte receivedByte2 = Wire.read();
    if (receivedValue1 < receivedByte1 && !turnInProcess && driveInProcess) {
      distance++;
    }
    if (receivedValue1 > receivedByte1 && !turnInProcess && driveInProcess) {
      distance--;
    }
    receivedValue1 = receivedByte1;
  }

}