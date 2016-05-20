/*
Name:    MidiTest.ino
Created: 4/29/2016 1:17:11 PM
Author:  Whipe The Pipe!
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* Start Config*/
#define MidiCH 0
#define midiVelocity 69

//Config the instrument
#define numberOfPipes 5
#define numberOfPots 1

//Base notes for the pipes
const int baseNotes[numberOfPipes] = {48, 51, 53, 55, 58}; //Midi values

//Config pipes
const int pipePins[numberOfPipes] = {8, 9, 10, 11, 12}; //digital pin
const int pipeLeds[numberOfPipes] = {13, 13, 13, 13, 13}; //digital pin

//Config Potentiometers
const int potPins[numberOfPots] = {3}; //analog pin

/*END Config */


/* Start Structs*/
typedef struct
{
  int pin;
  int ledpin;
  int note;
  bool playing;
}Pipe;

typedef struct
{
  int buttonPin;
  int y1;
  int y2;
  int z;
  int controllNumberY1;
  int controllNumberY2;
  int controllNumberZ;
  Adafruit_BNO055 bno;
}Accelorometer;

typedef struct
{
  int pin;
  int controllNumber;
  int value;
}Pot;

/* END Sturcts*/

Pipe pipes[numberOfPipes];
Pot pots[numberOfPots];
Accelorometer accelorometer;


// the setup function runs once when you press reset or power the board
void setup()
{
  int controllNumber = 0;

  //  Set MIDI baud rate:
  Serial.begin(115200);

  // Set up the accelorometer
  accelorometer.bno = Adafruit_BNO055(55);
  accelorometer.buttonPin = 2;
  accelorometer.y1 = 0;
  accelorometer.y2 = 0;
  accelorometer.z = 64;
  accelorometer.controllNumberY1 = controllNumber++;
  accelorometer.controllNumberY2 = controllNumber++;
  accelorometer.controllNumberZ = controllNumber++;
  if(!accelorometer.bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    while(true)
    {
      midi_note_on(MidiCH, 0, midiVelocity);
      delay(1000);
    }
  }
  delay(1000); //Not sure if this is nessasary
  accelorometer.bno.setExtCrystalUse(true);


  //Iniszialice the pipes
  for(int i = 0; i < numberOfPipes; i++)
  {
    pipes[i].pin = pipePins[i];
    pipes[i].ledpin = pipeLeds[i];
    pinMode(pipeLeds[i], OUTPUT);
    pipes[i].note = baseNotes[i];
    pipes[i].playing = false;
  }

  //Initsialize the pots
  for(int i = 0; i < numberOfPots; i++)
  {
    pots[i].pin = potPins[i];
    pots[i].controllNumber = controllNumber++;
  }
}

// the loop function runs over and over again until power down or resetvoid
void loop()
{
  checkAccelorometer();

  //Pipes
  for(int i = 0; i < numberOfPipes; i++)
  {
      int status = digitalRead(pipes[i].pin);
      if(status == HIGH && pipes[i].playing == false)
      {
        midi_note_on(MidiCH, pipes[i].note, midiVelocity);
        pipes[i].playing = true;
        digitalWrite(pipes[i].ledpin, HIGH);
      }
      if(status == LOW && pipes[i].playing == true)
      {
        midi_note_on(MidiCH, pipes[i].note, 0);
        pipes[i].playing = false;
        digitalWrite(pipes[i].ledpin, LOW);
      }
  }
  //Potentiometers
  for(int i = 0; i < numberOfPots; i++)
  {
    int potVal = map(analogRead(pots[i].pin), 0, 1023, 0, 127);

    // kollar om potens värde har förändrats mer än +/-1. Pots'en är oexakta och hamnar mitt imellan 2 lägen...
    if(pots[i].value != potVal && pots[i].value != potVal - 1 && pots[i].value != potVal + 1)
    {
      pots[i].value = potVal;
      midi_controller_change(MidiCH, pots[i].controllNumber, pots[i].value);
    }
  }
}


void checkAccelorometer()
{
  sensors_event_t event;
  if(digitalRead(accelorometer.buttonPin) == HIGH)
  {
    accelorometer.bno.getEvent(&event);
    int y = event.orientation.y; //sidled
    int z = event.orientation.z; //pitch
    if(y <= 0)
    {
      if(y < -45)
        y = -45;
      accelorometer.y1 = map(y, -45, 0, 127, 0);
      midi_controller_change(MidiCH, accelorometer.controllNumberY1, accelorometer.y1);
    }
    else if(y >= 0)
    {
      if(y > 80)
        y = 80;
      accelorometer.y2 = map(y, 0, 80, 0, 127);
      midi_controller_change(MidiCH, accelorometer.controllNumberY2, accelorometer.y2);
    }

    //Z-Led (Pitchbend)
    if(z >= -90 && z <= 90)
    {
      accelorometer.z = map(z, -90, 90, 0, 127);
      midi_controller_change(MidiCH, accelorometer.controllNumberZ, accelorometer.z);
    }
  }
  //nolla allt
  else if(digitalRead(accelorometer.buttonPin) == LOW)
  {

    accelorometer.z = 64;
    accelorometer.y1 = 0;
    accelorometer.y2 = 0;
    midi_controller_change(MidiCH, accelorometer.controllNumberZ, accelorometer.z);
    midi_controller_change(MidiCH, accelorometer.controllNumberY1, accelorometer.y1);
    midi_controller_change(MidiCH, accelorometer.controllNumberY2, accelorometer.y2);
  }
}
//Midifunctions 
void midi_note_on(int channel, int key, int velocity)
{
  midi_command(144 + channel, key, velocity);
}

void midi_controller_change(int channel, int control, int value)
{
  midi_command(176+channel, control, value);
}

void midi_pitch_bend(int channel, int value)
{
  midi_command(224+channel, value & 127, value >> 7);
}

void midi_command(int cmd, int data1, int data2)
{
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}