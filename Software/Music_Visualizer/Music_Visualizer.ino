#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    120
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define updateLEDS 1
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];

//Auido input
int strobe = 4;
int reset = 5;
int audio1 = A0;
int audio2 = A1;
int left[7];
int right[7];
int band;
int audio_input = 0;
int freq = 0;

//Music Visualizer
int midway = NUM_LEDS / 2; 
int loop_max = 0;
int k = 255; 
int decay = 0; 
int decay_check = 0;
long pre_react = 0; 
long react = 0; 
long post_react = 0; 

int wheel_speed = 2;// color wheel settings

void setup(){
  // SPECTRUM SETUP
  pinMode(audio1, INPUT);
  pinMode(audio2, INPUT);
  pinMode(strobe, OUTPUT);
  pinMode(reset, OUTPUT);
  digitalWrite(reset, LOW);
  digitalWrite(strobe, HIGH);
  
  // LED LIGHTING SETUP
  delay( 3000 ); //power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  //Clear LEDs
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB(0, 0, 0);
  FastLED.show();

  Serial.begin(115200);
  Serial.println("\nListening...");
}

void loop(){  
  //singleLevel();//Uncomment individually to display animation
  doubleLevel();
  //Snake();
  //delayMicroseconds(20);
}

// Function to generate raonbow color wheel
CRGB Scroll(int pos){
  pos = abs(pos);
  CRGB color (0,0,0);
  if(pos < 85){
    color.g = 0;
    color.r = ((float)pos / 85.0f) * 255.0f;
    color.b = 255 - color.r;
  }
  else if(pos < 170) {
    color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
    color.r = 255 - color.g;
    color.b = 0;
  } 
  else if(pos < 256) {
    color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
    color.g = 255 - color.b;
    color.r = 1;
  }
  return color;
}

void SinglePulse(){
  for(int i = NUM_LEDS - 1; i >= 0; i--) {
    if (i < react){
      leds[i] = Scroll((i * 256 / 50 + k) % 256);//Displays Rainbow color wheel
    }else{
      leds[i] = CRGB(0, 0, 0);      
    }
  }
  FastLED.show(); 
}

void DoublePulse(){
  for(int i = NUM_LEDS - 1; i >= midway; i--) {
    if (i < react + midway){
      leds[i] = Scroll((i * 256 / 50 + k) % 256);
      leds[(midway - i) + midway] = Scroll((i * 256 / 50 + k) % 256);
    }else{
      leds[i] = CRGB(0, 0, 0);//pulse left side 
      leds[midway - react] = CRGB(0, 0, 0);
      leds[i] = CRGB(0, 0, 0);//pulse right side 
      leds[midway - react] = CRGB(0, 0, 0);
    }
  }
  FastLED.show();
}

// Function to read 7 band equalizers
void readMSGEQ7(){
  digitalWrite(reset, HIGH);
  digitalWrite(reset, LOW);
  for(band=0; band <7; band++){
    digitalWrite(strobe, LOW);
    delayMicroseconds(30); 
    left[band] = analogRead(audio1);
    right[band] = analogRead(audio2);
    digitalWrite(strobe, HIGH); 
  }
}

//Function to trasnlate audio to LED with Single animation
void convertSingle(){
  if(left[freq] > right[freq]){
    audio_input = left[freq];
  }else{
    audio_input = right[freq];
  }
  if(audio_input > 80){
    pre_react = ((long)NUM_LEDS * (long)audio_input) / 1023L;//Translate audio to number of LED's
    if(pre_react > react){
      react = pre_react;
    }
    Serial.print(audio_input);
    Serial.print(" -> ");
    Serial.println(pre_react);
  }
}

//Function to trasnlate audio to LED with Double animation
void convertDouble(){
  if(left[freq] > right[freq]){
    audio_input = left[freq];
  }else{
    audio_input = right[freq];
  }
  if(audio_input > 80){
    pre_react = ((long)midway * (long)audio_input) / 1023L;
    if(pre_react > react){
      react = pre_react;
    }
    Serial.print(audio_input);
    Serial.print(" -> ");
    Serial.println(pre_react);
  }
}

void singleLevel(){
  readMSGEQ7();
  convertSingle();
  SinglePulse(); //Display Single Pulse animation

  k = k - wheel_speed;//speed of color wheel
  if (k < 0) //Resets color wheel
    k = 255;

  decay_check++;
  if(decay_check > decay)
  {
    decay_check = 0;
    if(react > 0)
      react--;
  }
}

void doubleLevel(){
  readMSGEQ7();
  convertDouble();
  DoublePulse();

  k = k - wheel_speed;
  if (k < 0){ 
    k = 255;
  }

  decay_check++;
  if (decay_check > decay){
    decay_check = 0;
    if (react > 0){
      react--;
    }
  }
}

void Snake(){
  readMSGEQ7();
  convertSingle(); 
  
  for(int i = NUM_LEDS - 1; i >= updateLEDS; i--){
    leds[i] = leds[i - updateLEDS];
  }
  for(int i = 0; i < updateLEDS; i++){
    if(i < react - 20){  
      leds[i] = Scroll((i * 256 / 50 + k) % 256);//Change to a different color setting
    }else{
        leds[i] = CRGB(0,0,0);//Set the LED color to None 
    }
  }
  FastLED.show();

  k = k - wheel_speed;
  if (k < 0)
  k = 255;
    
  decay_check++;
  if (decay_check > decay){
    decay_check = 0;
    if (react > 0){
      react--;
    }
  }
}
