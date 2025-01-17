#include "arduinoFFT.h"
#include <FastLED.h>

#define LED_PIN  6
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define BRIGHTNESS 44

#define SAMPLES 32             //Must be a power of 2
#define SAMPLING_FREQUENCY 10000  // Must be less then the limit for prescaler level below

#define inputCLK 3
#define inputDT 4
#define inputSW 2
#define MAX_BRIGHT 100
#define MIN_BRIGHT 10
#define TEXT_SIZE 34
#define SPRITE_SIZE 32
#define MODE_MAX 4 // three modes (0=spectrum analyize, 1=all white, 2=scroll text)

// knob globals
int bright = BRIGHTNESS;
int currentStateCLK;
int currentMode;
int lastMode = 0;
unsigned long lastMillis;

// LED globals
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 9;
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds[NUM_LEDS];
// Peak levels
char peak_levels [kMatrixWidth];
boolean peakHold = 1;
boolean resetLevels = 0;

// Text to display (ELYSE)
boolean letterE[TEXT_SIZE][kMatrixHeight] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0},
  {1, 1, 1, 1, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 1, 0, 0},
  {0, 1, 0, 0, 0, 1, 0, 1, 0},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {0, 1, 0, 0, 1, 0, 0, 1, 0},
  {0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1}
};
long int textPosition;
int textSize = TEXT_SIZE;

char sprites[SPRITE_SIZE][kMatrixHeight] = {
  {'B', 'W', 'B', 'B', 'B', 'B', 'B', 'W', 'B'},
  {'W', 'W', 'W', 'B', 'B', 'B', 'W', 'W', 'W'},
  {'B', 'W', 'B', 'B', 'W', 'B', 'B', 'W', 'B'},
  {'B', 'B', 'B', 'W', 'W', 'W', 'B', 'B', 'B'},
  {'B', 'W', 'B', 'B', 'W', 'B', 'B', 'B', 'B'},
  {'W', 'W', 'W', 'B', 'B', 'B', 'W', 'B', 'B'},
  {'B', 'W', 'B', 'B', 'B', 'W', 'W', 'W', 'B'},
  {'B', 'B', 'B', 'B', 'B', 'B', 'W', 'B', 'B'},
  {'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B'},
  {'B', 'B', 'R', 'R', 'B', 'B', 'B', 'B', 'B'},
  {'B', 'R', 'R', 'R', 'R', 'B', 'B', 'B', 'B'},
  {'R', 'R', 'R', 'R', 'R', 'R', 'B', 'Y', 'B'},
  {'R', 'R', 'R', 'R', 'R', 'R', 'Y', 'B', 'Y'},
  {'R', 'R', 'R', 'R', 'R', 'R', 'B', 'Y', 'B'},
  {'B', 'R', 'R', 'R', 'R', 'B', 'B', 'B', 'B'},
  {'B', 'B', 'R', 'R', 'B', 'B', 'B', 'B', 'B'},
  {'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B'},
  {'W', 'W', 'W', 'B', 'B', 'W', 'W', 'B', 'B'},
  {'W', 'W', 'W', 'W', 'W', 'W', 'C', 'T', 'B'},
  {'W', 'W', 'W', 'W', 'W', 'W', 'W', 'T', 'T'},
  {'W', 'W', 'B', 'W', 'W', 'O', 'C', 'T', 'B'},
  {'B', 'B', 'B', 'B', 'W', 'O', 'W', 'B', 'B'},
  {'B', 'B', 'B', 'B', 'B', 'O', 'B', 'B', 'B'},
  {'B', 'G', 'B', 'B', 'B', 'B', 'B', 'B', 'B'},
  {'B', 'G', 'G', 'B', 'G', 'B', 'B', 'B', 'B'},
  {'B', 'G', 'G', 'B', 'G', 'G', 'B', 'B', 'B'},
  {'B', 'G', 'G', 'G', 'G', 'G', 'B', 'G', 'B'},
  {'T', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
  {'B', 'G', 'G', 'G', 'G', 'G', 'B', 'G', 'B'},
  {'B', 'G', 'G', 'B', 'G', 'G', 'B', 'B', 'B'},
  {'B', 'G', 'G', 'B', 'G', 'B', 'B', 'B', 'B'},
  {'B', 'G', 'B', 'B', 'B', 'B', 'B', 'B', 'B'}
};
long int spritePosition;

char cCane[kMatrixWidth][kMatrixHeight] = {
  {'R', 'R', 'W', 'W', 'W', 'R', 'R', 'R', 'W'},
  {'R', 'W', 'W', 'W', 'R', 'R', 'R', 'W', 'W'},
  {'W', 'W', 'W', 'R', 'R', 'R', 'W', 'W', 'W'},
  {'W', 'W', 'R', 'R', 'R', 'W', 'W', 'W', 'R'},
  {'W', 'R', 'R', 'R', 'W', 'W', 'W', 'R', 'R'},
  {'R', 'R', 'R', 'W', 'W', 'W', 'R', 'R', 'R'},
  {'R', 'R', 'W', 'W', 'W', 'R', 'R', 'R', 'W'},
  {'R', 'W', 'W', 'W', 'R', 'R', 'R', 'W', 'W'},
  {'W', 'W', 'W', 'R', 'R', 'R', 'W', 'W', 'W'},
  {'W', 'W', 'R', 'R', 'R', 'W', 'W', 'W', 'R'},
  {'W', 'R', 'R', 'R', 'W', 'W', 'W', 'R', 'R'},
  {'R', 'R', 'R', 'W', 'W', 'W', 'R', 'R', 'R'},
  {'R', 'R', 'W', 'W', 'W', 'R', 'R', 'R', 'W'},
  {'R', 'W', 'W', 'W', 'R', 'R', 'R', 'W', 'W'},
  {'W', 'W', 'W', 'R', 'R', 'R', 'W', 'W', 'W'},
  {'W', 'W', 'R', 'R', 'R', 'W', 'W', 'W', 'R'},
};
unsigned int cCaneStart;

// FFT globals
arduinoFFT FFT = arduinoFFT();
unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];
double total = 0.0;

uint16_t XY( uint8_t x, uint8_t y) {
  uint16_t i;

  if ( x & 0x01) {
    // Odd columns run forwards
    i = (x * kMatrixHeight) + y;
  } else {
    // Even rows run backwards
    uint8_t reverseY = (kMatrixHeight - 1) - y;
    i = (x * kMatrixHeight) + reverseY;
  }
  return i;
}

void setup() {
  // Set encoder pins as inputs
  pinMode (inputCLK, INPUT);
  pinMode (inputDT, INPUT);
  pinMode (inputSW, INPUT);
  currentMode = 0;
  textPosition = kMatrixWidth;
  spritePosition = kMatrixWidth;

  // init peak levels
  for(int i=0; i<kMatrixWidth; i++) {
    peak_levels[i] = 0;
  }
  lastMode = 1; // This is safe and will make the other modes reset
  // LED setup
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(UncorrectedColor);
  FastLED.setBrightness( BRIGHTNESS );

  // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
  // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  ADCSRA |= bit (ADPS0) | bit (ADPS2);                 //  32 for 38Khz
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop() {
  if (currentMode == 0) { // spectrum analyze
    if (lastMode != 0) {
      // reset peak levels if new to mode
      for(int i=0; i<kMatrixWidth; i++) {
        peak_levels[i] = 0;
      }
      lastMode = 0;
      delay(70); // so as not to just skip past this mode when the knob is clicked
      lastMillis = millis();
    } else {
      if(millis() - lastMillis > 3000) { // reset every 3 seconds
        for(int i=0; i<kMatrixWidth; i++) {
          peak_levels[i] = 0;
        }
        lastMillis = millis();
      }
    }
    
    /*SAMPLING*/
    for (int i = 0; i < SAMPLES; i++) {
      microseconds = micros();
      vReal[i] = analogRead(0) - 256;
      vImag[i] = 0;
      while (micros() < (microseconds + sampling_period_us)) {
      }
    }

    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    // set the spectrum
    for (int x = 0; x < kMatrixWidth; x++) {
      for (int y = 0; y < kMatrixHeight; y++) {
        if (y < (int)kMatrixHeight * (vReal[x] / 2000)) {
          if (y >= peak_levels[x] ) {
            leds[XY(x, y)] = CRGB::Red;
            peak_levels[x] = y;
          } else {
            leds[ XY(x, y) ] = CRGB::Green;
          }
        } else {
          if (y != peak_levels[x]) {
            leds[ XY(x, y) ] = CRGB::Black;
          }
        }
      }
    }
  }

  if (currentMode == 1) {
    lastMode = 1;
    // Turn all LEDs on, then pause
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::White;
    }
    delay(200);
  }

  if (currentMode == 2) {
    if (lastMode != 2) {
      for(int i=0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
      lastMode = 2;
    }
    for (int x = 0; x < kMatrixWidth; x++) {
      for (int y = 0; y < kMatrixHeight; y++) {
        if (x >= textPosition && x < textPosition + textSize) {
          if (letterE[x - textPosition][y]) {
            leds[ XY(x, y) ] = CRGB::Purple;
          } else {
            leds[ XY(x, y) ] = CRGB::Black;
          }
        } else {
          leds[ XY(x, y) ] = CRGB::Black;
        }
      }
    }
    if (textPosition == -(textSize - 1)) {
      textPosition = kMatrixWidth;
    } else {
      textPosition--;
    }
    delay(100);
  }

  if (currentMode == 3) {
    if (lastMode != 3) {
      for(int i=0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
      lastMode = 3;
    }
    for (int x = 0; x < kMatrixWidth; x++) {
      for (int y = 0; y < kMatrixHeight; y++) {
        if (x >= spritePosition && x < spritePosition + SPRITE_SIZE) {
          switch (sprites[x - spritePosition][y]) {
            case 'R': // Red
              leds[ XY(x, y) ] = CRGB::Red;
              break;
            case 'Y': // Yellow
              leds[ XY(x, y) ] = CRGB::Yellow;
              break;
            case 'W': // White
              leds[ XY(x, y) ] = CRGB::White;
              break;
            case 'T': // Brown
              leds[ XY(x, y) ] = CRGB::Brown;
              break;
            case 'C': // Blue
              leds[ XY(x, y) ] = CRGB::Blue;
              break;
            case 'O': // Orange
              leds[ XY(x, y) ] = CRGB::Orange;
              break;
            case 'G': // Green
              leds[ XY(x, y) ] = CRGB::Green;
              break;
            default: // Black
              leds[ XY(x, y) ] = CRGB::Black;
          }
        } else {
          leds[ XY(x, y) ] = CRGB::Black;
        }
      }
    }
    if (spritePosition == -(SPRITE_SIZE - 1)) {
      spritePosition = kMatrixWidth;
    } else {
      spritePosition--;
    }
    delay(200);
  }

  if (currentMode == 4) {
    if (lastMode != 4) {
      lastMode = 4;
      cCaneStart = 0;
    }
    for (int x=0; x < kMatrixWidth; x++) {
      for (int y=0; y < kMatrixHeight; y++) {
        if (cCane[(cCaneStart+x)%kMatrixWidth][y] == 'R') {
          leds[ XY(x, y) ] = CRGB::Red;
        } else {
          leds[ XY(x, y) ] = CRGB::White;
        }
      }
    }
    cCaneStart++;
    delay(150);
  }
  
  // Was the knob rotated? Read the current state of inputCLK
  currentStateCLK = digitalRead(inputCLK);
  // If the previous and the current state of the inputCLK are different then a pulse has occured
  if (!currentStateCLK) {
    if (digitalRead(inputDT)) { // clockwise
      if (bright < MAX_BRIGHT) {
        bright += 10;
        FastLED.setBrightness(bright);
      }
    } else { // counter clockwise
      if (bright > MIN_BRIGHT) {
        bright -= 10;
        FastLED.setBrightness(bright);
      }
    }
  }

  // Was the knob depressed to switch the mode
  if (!digitalRead(inputSW)) {
    if (currentMode == MODE_MAX) {
      currentMode = 0;
    } else {
      currentMode++;
    }
  }

  FastLED.show();
}
