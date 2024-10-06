#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Ps3Controller.h>

#define PIN_WS2812B 5
#define NUM_PIXELS 8

uint16_t pixelNumber = NUM_PIXELS;
unsigned long pixelPrevious = 0;
int pixelInterval = 50;
int pixelQueue = 0;
int pixelCycle = 0;
unsigned long patternPrevious = 0;
bool patternComplete = false;
int patternInterval = 2000;
int patternCurrent = 0;

Adafruit_NeoPixel strip(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

void colorWipe(uint32_t color, int wait)
{
  static uint16_t current_pixel = 0;
  pixelInterval = wait;
  strip.setPixelColor(current_pixel++, color);
  strip.show();
  if (current_pixel >= pixelNumber)
  {
    current_pixel = 0;
    patternComplete = true;
  }
}

void theaterChase(uint32_t color, int wait)
{
  static uint32_t loop_count = 0;
  static uint16_t current_pixel = 0;

  pixelInterval = wait;

  strip.clear();

  for (int c = current_pixel; c < pixelNumber; c += 3)
  {
    strip.setPixelColor(c, color);
  }
  strip.show();

  current_pixel++;
  if (current_pixel >= 3)
  {
    current_pixel = 0;
    loop_count++;
  }

  if (loop_count >= 10)
  {
    current_pixel = 0;
    loop_count = 0;
    patternComplete = true;
  }
}

uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void theaterChaseRainbow(uint8_t wait)
{
  if (pixelInterval != wait)
    pixelInterval = wait;
  for (int i = 0; i < pixelNumber; i += 3)
  {
    strip.setPixelColor(i + pixelQueue, Wheel((i + pixelCycle) % 255));
  }
  strip.show();
  for (int i = 0; i < pixelNumber; i += 3)
  {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0));
  }
  pixelQueue++;
  pixelCycle++;
  if (pixelQueue >= 3)
    pixelQueue = 0;
  if (pixelCycle >= 256)
    pixelCycle = 0;
}

void rainbow(uint8_t wait)
{
  if (pixelInterval != wait)
    pixelInterval = wait;
  for (uint16_t i = 0; i < pixelNumber; i++)
  {
    strip.setPixelColor(i, Wheel((i + pixelCycle) & 255));
  }
  strip.show();
  pixelCycle++;
  if (pixelCycle >= 256)
    pixelCycle = 0;
}

void display()
{
  Serial.printf("patternCurrent=%d\n", patternCurrent);

  unsigned long currentMillis = millis();
  if (patternComplete || (currentMillis - patternPrevious) >= patternInterval)
  {
    patternComplete = false;
    patternPrevious = currentMillis;
  }

  if (currentMillis - pixelPrevious >= pixelInterval)
  {
    pixelPrevious = currentMillis;

    switch (patternCurrent)
    {
    case 8:
      theaterChaseRainbow(50);
      break;
    case 7:
      rainbow(10);
      break;
    case 6:
      theaterChase(strip.Color(0, 0, 127), 50); // Blue
      break;
    case 5:
      theaterChase(strip.Color(127, 0, 0), 50); // Red
      break;
    case 4:
      theaterChase(strip.Color(127, 127, 127), 50); // White
      break;
    case 3:
      colorWipe(strip.Color(0, 0, 255), 50); // Blue
      break;
    case 2:
      colorWipe(strip.Color(0, 255, 0), 50); // Green
      break;
    case 1:
      colorWipe(strip.Color(255, 0, 0), 50); // Red
      break;
    case 0:
      strip.clear();
      strip.show();
      break;
    }
  }
}

void setupDisplay()
{
  strip.begin();
  strip.clear();
  strip.show();
  strip.setBrightness(20);
}

// Motor
int motor_left = MOTORDEFAULT;
int motor_right = MOTORDEFAULT;
boolean direction_left = DIRECTIONDEFAULT;
boolean direction_right = DIRECTIONDEFAULT;

void motorSpeed()
{
  digitalWrite(MOTOR_R_2, direction_right);
  ledcWrite(PWMCHANNELMOTORRIGHT, motor_right);

  digitalWrite(MOTOR_L_2, direction_left);
  ledcWrite(PWMCHANNELMOTORLEFT, motor_left);

  // Serial.printf("motor_right=%d, motor_left=%d\n", motor_right, motor_left);
}

void motorStop()
{
  motor_left = MOTORDEFAULT;
  motor_right = MOTORDEFAULT;
  direction_left = DIRECTIONDEFAULT;
  direction_right = DIRECTIONDEFAULT;

  motorSpeed();
}

// PS3
void notify()
{
  int yAxisValue = (Ps3.data.analog.stick.ly);
  int xAxisValue = (Ps3.data.analog.stick.ry);

  direction_left = yAxisValue >= 0;
  direction_right = xAxisValue >= 0;

  motor_left = map(yAxisValue, POSITIVELIMIT_X_Y, NEGATIVELIMIT_X_Y, POSITIVELIMIT_R_L, NEGATIVELIMIT_R_L);
  motor_right = map(xAxisValue, POSITIVELIMIT_X_Y, NEGATIVELIMIT_X_Y, POSITIVELIMIT_R_L, NEGATIVELIMIT_R_L);

  motorSpeed();

  // if (Ps3.event.button_down.cross)
  // {
  // }

  if (Ps3.event.button_up.up)
  {
    if ((patternCurrent + 1) <= 8)
    {
      patternCurrent++;
    }
  }

  if (Ps3.event.button_up.down)
  {
    if ((patternCurrent - 1) >= 0)
    {
      patternCurrent--;
    }
  }
}

void onConnect()
{
  motorStop();

  Serial.println("Connected!.");
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(1000);
  Serial.println();
  Serial.println();

  pinMode(MOTOR_L_2, OUTPUT);
  ledcAttachPin(MOTOR_L_1, PWMCHANNELMOTORLEFT);
  ledcSetup(PWMCHANNELMOTORLEFT, FREQ, RESOLUTION);

  pinMode(MOTOR_R_2, OUTPUT);
  ledcAttachPin(MOTOR_R_1, PWMCHANNELMOTORRIGHT);
  ledcSetup(PWMCHANNELMOTORRIGHT, FREQ, RESOLUTION);

  motorStop();

  setupDisplay();

  Ps3.attachOnConnect(onConnect);
  Ps3.begin(MAC_PS3);
}

void loop()
{
  if (!Ps3.isConnected())
  {
    motorStop();

    Serial.println("Waiting for connection to control");
    delay(300);

    return;
  }

  notify();

  display();
}
