/*
Name:    MidiTest.ino
Created: 4/29/2016 1:17:11 PM
Author:  Whipe The Pipe!
*/


/* Start Config*/
#define PipeCH 144
#define midiVelocity 69
#define numberOfPipes 5
#define numberOfPots 1

const int pipePins[numberOfPipes] = {2, 4, 6, 10, 12}; //digital pin
const int pipeLeds[numberOfPipes] = {13, 13, 13, 13, 13};
const int potPins[numberOfPots] = {5}; //analog pin
const int baseNotes[numberOfPipes] = {51, 54, 56, 58, 61};
/*END Config */

typedef struct
{
  int pin;
  int ledpin;
  int note;
  bool playing;
}pipe;

typedef struct
{
  int pin;
  int value;
}pot;

pipe pipes[numberOfPipes];
pot pots[numberOfPots];


// the setup function runs once when you press reset or power the board
void setup()
{
  //  Set MIDI baud rate:
  Serial.begin(115200);

  //Initsialize the pots
  for(int i = 0; i < numberOfPots; i++)
  {
    pots[i].pin = potPins[i];
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

}

// the loop function runs over and over again until power down or resetvoid loop() {
void loop()
{

  for(int i = 0; i < numberOfPots; i++)
  {
    int potVal = map(analogRead(pots[i].pin), 0, 1023, 0, 127);

    // kollar om potens värde har förändrats mer än +/-1. Pots'en är oexakta och hamnar mitt imellan 2 lägen...
    if(pots[i].value != potVal && pots[i].value != potVal-1 && pots[i].value != potVal+1)
    {
      pots[i].value = potVal;
      sendMidi(176, i, pots[i].value);
    }
  }

  for(int i = 0; i < numberOfPipes; i++)
  {
    if(digitalRead(pipes[i].pin) == HIGH && pipes[i].playing == false)
    {
      sendMidi(PipeCH, pipes[i].note, midiVelocity);
      pipes[i].playing = true;
      digitalWrite(pipes[i].ledpin, HIGH);
    }
    if(digitalRead(pipes[i].pin) == LOW && pipes[i].playing == true)
    {
      sendMidi(PipeCH, pipes[i].note, 0);
      pipes[i].playing = false;
      digitalWrite(pipes[i].ledpin, LOW);
    }
  }
}

// plays a MIDI note.  Doesn't check to see that cmd is greater than 127, or that data values are less than 127:
// noteOn is a midi message, it's also used to end a note.
void sendMidi(int cmd, int data1, int data2)
{
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

// Value is +/- 8192
void PitchWheelChange(int value)
{
  unsigned int change = 8192 + value;  // 
  unsigned char low = change & 127;  // Low 7 bits
  unsigned char high = (change >> 7) & 0x7F;  // High 7 bits
  sendMidi(224, low, high);
}