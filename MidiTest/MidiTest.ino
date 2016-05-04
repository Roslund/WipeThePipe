/*
Name:    MidiTest.ino
Created: 4/29/2016 1:17:11 PM
Author:  Whipe The Pipe!
*/

/* Start Config*/
#define MidiCH 0
#define midiVelocity 69
#define numberOfPipes 5
#define numberOfPots 1
#define numberOFTilts 1

const int pipePins[numberOfPipes] = {2, 4, 6, 10, 12}; //digital pin
const int pipeLeds[numberOfPipes] = {13, 13, 13, 13, 13};
const int potPins[numberOfPots] = {5}; //analog pin
const int baseNotes[numberOfPipes] = {51, 54, 56, 58, 61};
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
}Tilt;
/* END Sturcts*/


Pipe pipes[numberOfPipes];
Pot pots[numberOfPots];
Tilt tilts[numberOFTilts];

static unsigned long DELTATIME = millis();


// the setup function runs once when you press reset or power the board
void setup()
{
  int controllNumber = 0;
  //  Set MIDI baud rate:
  Serial.begin(115200);

  //Initsialize the pots
  for(int i = 0; i < numberOfPots; i++)
  {
    pots[i].pin = potPins[i];
    pots[i].controllNumber = controllNumber++;
  }

  //Iniszialice the pipes
  for(int i = 0; i < numberOfPipes; i++)
  {
    pipes[i].pin = pipePins[i];
    pipes[i].ledpin = pipeLeds[i];
    pinMode(pipeLeds[i], OUTPUT);
    pipes[i].note = baseNotes[i];
    pipes[i].playing = false;
  }

  //set up the tiltswitches 
  tilts[1].leftPin = 0;
  tilts[1].rightPin = 0;
  tilts[1].buttonPin = 0;
  tilts[1].controllNumber = controllNumber++;
  tilts[1].value = 0;

}

// the loop function runs over and over again until power down or resetvoid
void loop()
{
  //Potentiometers
  if(millis() - DELTATIME > 20)
  {
    DELTATIME = millis();
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

  //Tiltswitches
  if(millis() - DELTATIME > 20)
  {
    DELTATIME = millis();

    for(int i = 0; i < numberOFTilts; i++)
    {

      if(digitalRead(tilts[i].leftPin) == HIGH && tilts[i].value < 127)
      {
        tilts[i].value++;
        midi_controller_change(MidiCH, tilts[i].controllNumber, tilts[i].value);
      }
      if(digitalRead(tilts[i].rightPin) == HIGH && tilts[i].value > 0)
      {
        tilts[i].value--;
        midi_controller_change(MidiCH, tilts[i].controllNumber, tilts[i].value);
      }
    }
  }

}

void midi_controller_change(int channel, int control, int value)
{
  midi_command(176+channel, control, value);
}

void midi_note_on(int channel, int key, int velocity)
{
  midi_command(144+channel, key, velocity);
}

void midi_command(int cmd, int data1, int data2)
{
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}