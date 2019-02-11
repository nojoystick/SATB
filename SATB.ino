/*********
 * 
 * Dallin Williams
 * 1.23.2019
 */

#include <Audio.h>

//#define DEBUG

//inputs
//index 0 - 3 are buttons, 4 is the freq pot, 5 is the volume pot
int in[] = {3, 4, 5, 8, A0, A4};
int swi[] = {0, 1, 2};

//outputs
//index 0 - 3 are LEDs
int out[] = {21, 20, 17, 15};

//button/switch; reading & debounce
int buttonFlag = 1;
int swiFlag = 1;
const int NUM_BUTTONS = 4;
const int NUM_SWITCH = 3;         //num pins for 5 way switch
byte sRead = 0x000;               //curr reading 5 way switch
byte bRead = 0x0000;              // curr reading buttons
int sState;                       // 1 - 5
int bState[NUM_BUTTONS];          // curr output state buttons

unsigned long time = 0;        
unsigned long debounce = 200;   // debounce time for buttons

//audio
AudioSynthWaveform       waveforms[NUM_BUTTONS];
AudioOutputI2S           headphones;         
AudioMixer4              mixer;

AudioConnection          patchCord1(waveforms[0], 0, mixer, 0);
AudioConnection          patchCord2(waveforms[1], 0, mixer, 1);
AudioConnection          patchCord3(waveforms[2], 0, mixer, 2);
AudioConnection          patchCord4(waveforms[3], 0, mixer, 3);
AudioConnection          patchCordL(mixer, 0, headphones, 0);
//For stereo:
//AudioConnection          patchCordR(mixer, 0, headphones, 1);

AudioControlSGTL5000     sgtl5000_1;     

// waveform modulator
int freq_level;                        // for holding pot input
float vol = 0.5;
int freqs[NUM_BUTTONS] = {0, 0, 0, 0}; //frequency matrix; 1 voice per button
int NUM_NOTES = 25;                    //size of penta[]
int SCALE = 1024/(NUM_NOTES-5);        //scale from 1024 pot input into the array size

//pentatonic scale frequencies
int penta[] = {65, 73, 82, 98, 110,
               131, 147, 165, 196, 220,
               262, 294, 330, 392, 440,
               523, 587, 659, 784, 880,
               1047, 1175, 1319, 1568, 1760};
               
void setup()
{
  //initialize button/LED I/O
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    pinMode(in[i], INPUT);
    attachInterrupt(digitalPinToInterrupt(in[i]), updateButtons, RISING); // sttach button ISRs
    pinMode(out[i], OUTPUT);
    bState[i] = HIGH;
  }
  //pots
  pinMode(in[4], INPUT);
  pinMode(in[5], INPUT);
  //5-way switch
  for(int i = 0; i < NUM_SWITCH; i++)
  {
    pinMode(swi[i], INPUT);
    attachInterrupt(digitalPinToInterrupt(swi[i]), updateSwitch, CHANGE);
  }

  //initialize audio
  AudioMemory(20);
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    mixer.gain(i, 0.1);
  }
  
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    waveforms[i].amplitude(0.25);
    waveforms[i].pulseWidth(0.15);
  }

  //flag ISR for initial readings
  updateButtons();
  updateSwitch();
  Serial.begin(9600);
}

void loop()
{
  #ifdef DEBUG
  Serial.begin(9600);
  delay(1);
  Serial.print("BUTTON STATES: ");
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    Serial.print(bState[i]);
    delay(1);
  }
  delay(1);
  Serial.print("     ");
  Serial.print("FREQ VAL: ");
  Serial.print(freq_level);
  Serial.print("     ");
  Serial.print("VOL VAL: ");
  Serial.print(vol);
  Serial.print("     ");
  Serial.print("SWI VAL: ");
  Serial.print(sState);
  Serial.print("\n");
  #endif
  
  synth();
  if(buttonFlag)
    buttonHandler();
    buttonFlag = 0;
  if(swiFlag)
    switchHandler();
    swiFlag = 0;
}

void synth()
{
  //read pot input
  freq_level = analogRead(in[4]);
  vol = analogRead(in[5]);
  sgtl5000_1.volume(vol/1600.0f);  //scale volume from 1024

  //if the voice button is ON, output the voice (offset by 'i' interval for harmony)
  //if the voice button is OFF, output 0
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    if(bState[i] == HIGH)
    {
      freqs[i] = penta[(freq_level/SCALE)+i];
      waveforms[i].frequency(freqs[i]);
    }
    if(bState[i] == LOW)
    {
      muteWaveform(waveforms[i]);
      delay(3);
    }
  }
  //fairly slow sample rate to minimize jitter
  delay(100);
}

void buttonHandler()
{
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    if (bitRead(bRead, i) == HIGH && millis() - time > debounce)
    {
      if (bState[i] == HIGH)
        bState[i] = LOW;
      else
        bState[i] = HIGH;
    time = millis();    
    }
  }
  //write new states to output
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    digitalWrite(out[i], bState[i]);
  }
}

void switchHandler()
{
  // 0x000 corresponds to 0x[(swi[2])(swi[1])(swi[0])]
  parseSwitch();
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
    if(sState == 1)
      waveforms[i].begin(WAVEFORM_SINE);
    if(sState == 2)
      waveforms[i].begin(WAVEFORM_TRIANGLE);
    if(sState == 3)
      waveforms[i].begin(WAVEFORM_TRIANGLE_VARIABLE);
    if(sState == 4)
      waveforms[i].begin(WAVEFORM_SAWTOOTH);
    if(sState == 5)
      waveforms[i].begin(WAVEFORM_SQUARE);
    if(bState[i] == LOW)
      muteWaveform(waveforms[i]);
      delay(3);
  }
}
void parseSwitch()
{
  // 100 = 1; 110 = 2; 010 = 3; 011 = 4; 001 = 5
  //parse switch using switch()
  switch(sRead){
    case 1:
      sState = 1;
    case 3:
      sState = 2;
    case 2:
      sState = 3;
    case 6:
      sState = 4;
    case 4:
      sState = 5;
  }
}

void muteWaveform(AudioSynthWaveform w)
{
  w.frequency(0);
}

//One ISR for all 4 buttons
void updateButtons()
{
  buttonFlag = 1;
  for(int i = 0; i < NUM_BUTTONS; i++)
  {
      bitWrite(bRead,i,digitalRead(in[i]));
  }
}
//ISR for 5 way switch
void updateSwitch()
{
  swiFlag = 1;
  for(int i = 0; i < NUM_SWITCH; i++)
  {
      bitWrite(sRead,i,digitalRead(swi[i]));
  }
}
