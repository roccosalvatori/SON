
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

// create audio component objects
AudioPlaySdWav           playWav1;           // SD card wav file player
AudioInputI2S            audioInput;         // audio shield input, mic or line-in selectable
AudioMixer4              mixer1;             // mixers to combine wav file and audio shield inputs
AudioMixer4              mixer2;

// Use one of these 3 output types: Digital I2S, Digital S/PDIF, or Analog DAC
AudioOutputI2S           audioOutput;
//AudioOutputSPDIF       audioOutput;
//AudioOutputAnalog      audioOutput;

// wire up the interfaces between audio components with patch cords
// mixer inputs
AudioConnection          patchCord1(playWav1, 0, mixer1, 0);          // left  channels into mixer 1
AudioConnection          patchCord2(audioInput, 0, mixer1, 1);

AudioConnection          patchCord3(playWav1, 1, mixer2, 0);          // right channels into mixer 2
AudioConnection          patchCord4(audioInput, 1, mixer2, 1);

// mixer outputs
AudioConnection          patchCord5(mixer1, 0, audioOutput, 0);
AudioConnection          patchCord6(mixer2, 0, audioOutput, 1);

// object to allow control of the SGTL5000 audio shield settings
AudioControlSGTL5000     audioShield;

// buttons and potentiometers
#define pot0             A0
#define pot1             A1
#define button0          0
#define button1          1

// attach button debouncers to input buttons
Bounce db_button0 = Bounce(button0, 30);
Bounce db_button1 = Bounce(button1, 30);

// choose mic or line input for audio shield input path
//const int inputChSelect = AUDIO_INPUT_MIC;
const int inputChSelect = AUDIO_INPUT_LINEIN;

// audio shield volume
int masterVolume        = 0;

// uncomment one set of SD card SPI pins to use
// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN     10
#define SDCARD_MOSI_PIN   7
#define SDCARD_SCK_PIN    14

// wav filenames on SD card for playback
char *wavFiles[]       = {"01.WAV", "02.WAV"
                         };
byte wavNum            = 0;      // current wav file index playing from array list
bool wavIsPlaying      = false;  // track if a wav file is currently playing or not

void setup() {
  Serial.begin(9600);
  Serial.println("SD Player Demo\n");

  // buttons are inputs with pullups
  pinMode(button0, INPUT);
  pinMode(button1, INPUT);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // comment these out if not using the audio adaptor board.
  Serial.print("init audio shield...");
  audioShield.enable();
  audioShield.inputSelect(inputChSelect);  // select mic or line-in for audio shield input source
  audioShield.volume(0.5);
  Serial.println("done.");

  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);

  mixer2.gain(0, 0.5);
  mixer2.gain(1, 0.5);
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);

  Serial.print("init SD card...");
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message
    Serial.println("Unable to access the SD card.  Program halted.");
    while (1);
  }
  Serial.println("done.");
  Serial.println("Waiting for control input...");

  // reset audio resource usage stats.
  // useful if tracking max usage in main program
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
}

void playFile(const char *filename)
{
  Serial.print("Start playing file: ");
  Serial.println(filename);

  // start playing the file.
  // sketch continues to run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library to read WAV header info
  delay(5);
}


void loop() {

  // auto select next wav file if current file finishes playing
  // and if playback is enabled
  if ((!(playWav1.isPlaying())) && (wavIsPlaying)) {
    wavNum++;
    if (wavNum > 1) {
      wavNum = 0;
    }
    playFile(wavFiles[wavNum]);
  }

  // read volume control pot and set audio shield volume if required
  int vol = analogRead(pot0);
  if (vol != masterVolume) {
    masterVolume = vol;
    audioShield.volume((float)vol / 1023);  // audio shield headphone out volume (optional)
    mixer1.gain(0, (float)vol / 1023);      // software mixer input channel volume
    mixer1.gain(1, (float)vol / 1023);
    mixer2.gain(0, (float)vol / 1023);
    mixer2.gain(1, (float)vol / 1023);
  }

  // update the button debounce status so falling edges
  // can be detected and processed
  db_button0.update();
  db_button1.update();

  // button 0 pressed - toggle playback start/stop for current wav file
  if (db_button0.fallingEdge()) {
    if (playWav1.isPlaying()) {
      playWav1.stop();
      wavIsPlaying = false;
      Serial.println("Playback stopped\n");
    }
    else {
      playFile(wavFiles[wavNum]);
      wavIsPlaying = true;
      Serial.println("Playback started");
    }
    //Serial.print("Audio memory usage max: ");
    //Serial.println(AudioMemoryUsageMax());
  }

  // button 1 pressed - skip track forward
  if (db_button1.fallingEdge()) {
    Serial.println("Skip track forward");
    if (wavNum == 1)
      wavNum = 0;
    else
      wavNum++;
    playFile(wavFiles[wavNum]);
    wavIsPlaying = true;
  }
}
