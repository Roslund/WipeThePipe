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

//Delays all in millisecound
#define minNoteTime 0  //minimum time a not will be played and cant be played again

//Config the instrument
#define numberOfPipes 5
#define numberOfPots 1
//#define numberOFTilts 0
#define numberOFAccelorometers 1

//Base notes for the pipes
const int baseNotes[numberOfPipes] = {48, 51, 53, 55, 58}; //Midi values

//Config pipes
const int pipePins[numberOfPipes] = {8, 9, 10, 11, 12}; //digital pin
const int pipeLeds[numberOfPipes] = {13, 13, 13, 13, 13}; //digital pin

//Config Potentiometers
const int potPins[numberOfPots] = {3}; //analog pin

//Config Tiltswitches 
//const int tiltLeftPin[numberOFTilts] = {3,8}; //digital pin
//const int tiltRightPin[numberOFTilts] = {5,9}; //digital pin
//const int tiltButtonPin[numberOFTilts] = {11, 11}; //digital pin: pin 7 = stav, pin 11= handske
//const int tiltDelay[numberOFTilts] = {0,0}; //Time in milisecounds, lower value = faster changes.
/*END Config */


/* Start Structs*/
typedef struct
{
  int pin;
  int ledpin;
  int note;
  bool playing;
  unsigned long pressTime;
}Pipe;

typedef struct
{
  int y;
  int z;
}Accelorometer;

typedef struct
{
  int pin;
  int controllNumber;
  int value;
}Pot;

typedef struct
{
  int leftPin;
  int rightPin;
  int buttonPin;
  int controllNumber;
  int value;
  unsigned long tiltTime;
  int tiltDelay;
}Tilt;

/* END Sturcts*/

Pipe pipes[numberOfPipes];
Pot pots[numberOfPots];
//Tilt tilts[numberOFTilts];
Accelorometer Accelorometers[numberOFAccelorometers];
Adafruit_BNO055 bno = Adafruit_BNO055(55);


// the setup function runs once when you press reset or power the board
void setup()
{
  int controllNumber = 0;

  //  Set MIDI baud rate:
  Serial.begin(115200);

  // Set up the accelorometer
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    while(true)
    {
      midi_note_on(MidiCH, 0, midiVelocity);
      delay(1000);
    }
  }
  delay(1000);
  bno.setExtCrystalUse(true);
  Accelorometers[0].y = 63;
  Accelorometers[0].z = 63;

  //Iniszialice the pipes
  for(int i = 0; i < numberOfPipes; i++)
  {
    pipes[i].pin = pipePins[i];
    pipes[i].ledpin = pipeLeds[i];
    pinMode(pipeLeds[i], OUTPUT);
    pipes[i].note = baseNotes[i];
    pipes[i].playing = false;
    pipes[i].pressTime = millis();
  }

  //Initsialize the pots
  for(int i = 0; i < numberOfPots; i++)
  {
    pots[i].pin = potPins[i];
    pots[i].controllNumber = controllNumber++;
  }

  //Iniszialice the tiltswitches 
  //for(int i = 0; i < numberOFTilts; i++)
  //{
  //  tilts[i].leftPin = tiltLeftPin[i];
  //  tilts[i].rightPin = tiltRightPin[i];
  //  tilts[i].buttonPin = tiltButtonPin[i];
  //  tilts[i].controllNumber = controllNumber++;
  //  tilts[i].value = 0;
  //  tilts[i].tiltTime = millis();
  //  tilts[i].tiltDelay = tiltDelay[i];
  //  
  //}

}

// the loop function runs over and over again until power down or resetvoid
void loop()
{
  //Acelorometer
  sensors_event_t event;
  if(digitalRead(7) == HIGH)
  {
    bno.getEvent(&event);
    int y = event.orientation.y; //pitch
    int z = event.orientation.z; //sidled

    if(y >= -90 && y <= 90)
      Accelorometers[0].y = map(y, -90, 90, 0, 127);
    if(z >= -90 && z <= 90)
      Accelorometers[0].z = map(z, -90, 90, 0, 127);
    midi_controller_change(MidiCH, 13, Accelorometers[0].y);
    midi_controller_change(MidiCH, 14, Accelorometers[0].z);
  }
  //återgå pitchen till 63
  else
  {
    if(Accelorometers[0].y < 63)
    {
      Accelorometers[0].y++;
      midi_controller_change(MidiCH, 13, Accelorometers[0].y);
    } 
    if(Accelorometers[0].y > 63)
    {
      Accelorometers[0].y--;
      midi_controller_change(MidiCH, 13, Accelorometers[0].y);
    }
  }

  //Pipes
  for(int i = 0; i < numberOfPipes; i++)
  {
    if(millis() - pipes[i].pressTime > minNoteTime)
    {
      int status = digitalRead(pipes[i].pin);
      if(status == HIGH && pipes[i].playing == false)
      {
        midi_note_on(MidiCH, pipes[i].note, midiVelocity);
        pipes[i].playing = true;
        digitalWrite(pipes[i].ledpin, HIGH);
        pipes[i].pressTime = millis();
      }
      if(status == LOW && pipes[i].playing == true)
      {
        midi_note_on(MidiCH, pipes[i].note, 0);
        pipes[i].playing = false;
        digitalWrite(pipes[i].ledpin, LOW);
      }
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

  //Tiltswitches
  //for(int i = 0; i < numberOFTilts; i++)
  //{
  //  if(millis() - tilts[i].tiltTime > tilts[i].tiltDelay && digitalRead(tilts[i].buttonPin) == HIGH)
  //  {
  //    tilts[i].tiltTime = millis(); //Reset the delay

  //    if(digitalRead(tilts[i].leftPin) == HIGH && tilts[i].value < 127)
  //    {
  //      tilts[i].value++;
  //      midi_controller_change(MidiCH, tilts[i].controllNumber, tilts[i].value);
  //    }
  //    if(digitalRead(tilts[i].rightPin) == HIGH && tilts[i].value > 0)
  //    {
  //      tilts[i].value--;
  //      midi_controller_change(MidiCH, tilts[i].controllNumber, tilts[i].value);
  //    }
  //  }
  //}
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