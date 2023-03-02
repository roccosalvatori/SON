/* THIS PROGRAM CONTAINS THE EFFECTS THAT WILL BE APPLIED TO A CHOSEN AUDIO FILE AND PROCESSED BY THE ARDUINO TEENSY 4.0.
   3 EFFECTS HAVE BEEN IMPLEMENTED : REVERB, GRANULAR AND BITCRUSHER.
   TWO BUTTONS AND TWO POTENTIOMETERS HAVE BEEN USED AS HARDWARE. */


#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Bounce.h>

//Audio file that has been converted into an array from a WAV file
#include "AudioSample.h"  

// The granular effect requires memory to operate
#define GRANULAR_MEMORY_SIZE 12800  // Enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

// Create the Audio components
//File read
AudioPlayMemory    sound0;   

// Comes out from the headphones jack
AudioOutputI2S      headphones;

// Defining buttons
Bounce button0 = Bounce(0, 5); 
Bounce button1 = Bounce(1, 15);

//Audio Effects definitions
AudioEffectBitcrusher   left_BitCrusher;
AudioEffectBitcrusher   right_BitCrusher;
AudioEffectReverb reverb;
AudioEffectGranular granular;

//BitCrusher variables definitions
int current_CrushBits = 16; //this defaults to passthrough.
int current_SampleRate = 44100; // this defaults to passthrough.

// Create Audio mixers in order to apply different effects simultaneously
AudioMixer4              mixer1;         
AudioMixer4              mixer2;   
AudioMixer4              mixer3;   
AudioMixer4              mixer4;   
AudioMixer4              mixer5;

// Create the audio connections between the different entities

AudioConnection          patchCord1(sound0, 0, left_BitCrusher, 0);
AudioConnection          patchCord2(sound0, 1, right_BitCrusher, 0);
AudioConnection          patchCord3(sound0, 0, l_myEffect, 0);
AudioConnection          patchCord4(sound0, 1, r_myEffect, 0);
AudioConnection          patchCord5(left_BitCrusher, 0, mixer1, 0);
AudioConnection          patchCord6(right_BitCrusher, 0, mixer2, 0);
AudioConnection          patchCord9(mixer1, 0, mixer5, 0);
AudioConnection          patchCord10(mixer2, 0, mixer5, 1);
AudioConnection          patchCord11(sound0, 0, mixer3, 0);
AudioConnection          patchCord12(sound0, 1, mixer3, 1);
AudioConnection          patchCord13(mixer3, reverb);
AudioConnection          patchCord14(mixer3, 0, mixer4, 1);
AudioConnection          patchCord15(reverb, 0, mixer4, 0);
AudioConnection          patchCord16(mixer4, 0, mixer2, 2);
AudioConnection          patchCord17(mixer4, 0, mixer1, 2);
AudioConnection          patchCord18(sound0, 0, mixer5, 0);
AudioConnection          patchCord19(sound0, 1, mixer5, 1);
AudioConnection          patchCord20(mixer5, granular);
AudioConnection          patchCord21(granular, 0, headphones, 0);
AudioConnection          patchCord22(granular, 0, headphones, 1);

// For AudioShield use
AudioControlSGTL5000 audioShield;

// Setup code part, to run once
void setup() {

  // Activate the two buttons
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  
  Serial.begin(38400);
  while (!Serial) ;
  delay(3000);
  
   // Bitcrusher effect definition
  left_BitCrusher.bits(current_CrushBits); //set the crusher to defaults. This will passthrough clean at 16,44100
  left_BitCrusher.sampleRate(current_SampleRate); //set the crusher to defaults. This will passthrough clean at 16,44100
  right_BitCrusher.bits(current_CrushBits); //set the crusher to defaults. This will passthrough clean at 16,44100
  right_BitCrusher.sampleRate(current_SampleRate); //set the crusher to defaults. This will passthrough clean at 16,44100
  //

  mixer5.gain(0, 0.5);
  mixer5.gain(1, 0.5);

  // The granular effect requires memory to operate
  granular.begin(granularMemory, GRANULAR_MEMORY_SIZE);

  // Does not work with any less
  AudioMemory(40);

  audioShield.enable();
  audioShield.volume(0.7);
}

// Loop code part, to run repeatedly
void loop()
{
  elapsedMillis msec;
  if (sound0.isPlaying() == false) {
    sound0.play(AudioSample);
  }
   while (sound0.isPlaying()) {

    // While the music plays, adjust reverb parameters and print info
    if (msec > 250) {
      msec = 0;
      float knob_A1 = 0.9;
        
      // Adjust parameter with analog input
      knob_A1 = (float)analogRead(A1) / 1023;
  
      mixer4.gain(0, knob_A1);
      mixer4.gain(1, 1.0 - knob_A1);

      Serial.print("Reverb: mix=");
      Serial.print(knob_A1 * 100.0);

      Serial.print("%, CPU Usage=");
      Serial.print(reverb.processorUsage());
      Serial.println("%");
    }  
        
    // Update all the button objects
    button0.update();
    button1.update();
  
    if (button1.fallingEdge()) {
      //Bitcrusher BitDepth
      if (current_CrushBits >= 2) { // Each time the button is pressed, it deduces 1 bit from the settings
          current_CrushBits--;
      } else {
        current_CrushBits = 16; // If it gets down to 1, goes back to the top
      }
  
      left_BitCrusher.bits(current_CrushBits);
      left_BitCrusher.sampleRate(current_SampleRate);

      right_BitCrusher.bits(current_CrushBits);
      right_BitCrusher.sampleRate(current_SampleRate);

      Serial.print("Bitcrusher set to ");
      Serial.print(current_CrushBits);

      Serial.print(" Bit, Samplerate at ");
      Serial.print(current_SampleRate);
      Serial.println("Hz");
    }
      // Read knobs, scale to 0-1.0 numbers
      float knobA2 = (float)analogRead(A2) / 1023.0;
    
      // Button 0 starts the Freeze effect
      if (button0.fallingEdge()) {
        float msec = 100.0 + (500/1023 * 190.0);
        granular.beginFreeze(msec);
        Serial.print("Begin granular freeze using ");
        Serial.print(msec);
        Serial.println(" grains");
      }
      if (button0.risingEdge()) {
        granular.stop();
      }
    
      // Continuously adjust the speed of the granular effect
      float ratio;
      ratio = powf(2.0, knobA2 * 2.0 - 1.0); 
      granular.setSpeed(ratio);
   }
}
