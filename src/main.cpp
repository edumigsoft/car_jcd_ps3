#include <Arduino.h>
#include <Ps3Controller.h>

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

  motor_left = map(yAxisValue, 127, -128, 0, -254);
  motor_right = map(xAxisValue, 127, -128, 0, -254);

  motorSpeed();

  if (Ps3.event.button_down.cross)
  {
    // startBuzzer = !startBuzzer;
  }
}

void onConnect()
{
  motorStop();

  Serial.println("Connected!.");
}

void onDisConnect()
{
  motorStop();

  Serial.println("DisConnected!.");
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

  // motorStop();

  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.attachOnDisconnect(onDisConnect);
  Ps3.begin(MAC_PS3);

  while (!Ps3.isConnected())
  {
    Serial.println("Waiting for connection to control");
    delay(300);
  }
}

void loop()
{
}
