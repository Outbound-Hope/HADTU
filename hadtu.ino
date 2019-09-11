#include <Adafruit_NeoPixel.h>

// Code by Cam Wian
// Edits and direction by Jason Griffey


//    PINS

const int neoPin = 2;

const int keySwitchPin = 3;
const int keyAPin = 4;
const int keyBPin = 5;
const int keyCPin = 6;

const int useButtonPin = 7;

const int resetButtonPin = 8;

const int buzzerPin = 9;


////////////////////////


const int ledCount = 8;
Adafruit_NeoPixel strip(ledCount,neoPin);



//    BUTTON STATES

int keySwitchState;
int keyAState;
int keyBState;
int keyCState;

int useButtonState;
int prevUseState;

int resetButtonState;  
int prevResetState;

//    TIMING

int rechargeTime = 3;        // Time, in seconds, to recharge after using one "heal".
int rechargeCounter = rechargeTime;

int resetTime = 10;           // Time, in seconds, to reset on dock. Countdown is 
int resetCounter = resetTime; // paused if device leaves dock, and resumes upon returning.
                              // Set resetTime to zero for instant reset. 
                              
int partialResetTime = 2;     // Time, in seconds, to increase each "heal" counter by one.
                              // Only used if partialReset is true. The device will finish
                              // resetting if all the healing types are at full capacity from 
                              // partial reset OR if the resetTime time limit is reached.
int partialResetCounter = partialResetTime;


//    COLORS

uint32_t red = strip.Color(255,0,0);  // Physical Damage color
uint32_t green = strip.Color(0,255,0);  // Radiation Damage color
uint32_t blue = strip.Color(0,0,255);  // Poison or Chemical Damage color

uint32_t orange = strip.Color(255,132,0);
uint32_t aqua = strip.Color(0,247,255);
uint32_t purple = strip.Color(100,0,255);
uint32_t pink = strip.Color(255,0,234);
uint32_t lime = strip.Color(85,255,0);
uint32_t yellow = strip.Color(255,255,0);

uint32_t white = strip.Color(255,255,255);

uint32_t typeAColor = red;
uint32_t typeBColor = blue;
uint32_t typeCColor = green;



//    OTHER VARIABLES

bool active = false;

bool shouldRecharge = true;

bool silent = false;        // Silences all sounds

bool partialReset = true;   // Allows or disables partial recharging on the dock. For
                            // example, if partialReset is set to true, a device left 
                            // the dock for half of the reset time will regain half
                            // of the total uses.
char currentType = 'a';

const int brightness = 10; // Brightness of strip throughout use, 0 being off and 255 full.

int amountA = 5;          // Number of uses for each type of healing. If this is over the number 
int amountB = 5;          // of LEDs, the strip will show the corresponding percentage left mapped 
int amountC = 5;          // to the amount of LEDs. Otherwise, each LED is one use.

int counterA = amountA;
int counterB = amountB;
int counterC = amountC;

int chaseChanges = 8;
int chaseCounter = chaseChanges;
int chaseDelay = 1000 / chaseChanges;
int chaseLedLit;
int chaseChangeAmount = 1;

bool flashOn = false;




void setup() {
  strip.begin();
  strip.show();
  strip.setBrightness(brightness);

  playAnimation("startup");
  
  pinMode(keySwitchPin,INPUT_PULLUP);
  pinMode(keyAPin,INPUT_PULLUP);
  pinMode(keyBPin,INPUT_PULLUP);
  pinMode(keyCPin,INPUT_PULLUP);
  
  Serial.begin(9600);
}

void loop() {
  keySwitchState = digitalRead(keySwitchPin);
  if(keySwitchState == LOW){
    active = !active;
    Serial.print("active: ");
    Serial.println(active);
    if(active == true){
      playAnimation("turnOn");
     }else{
      playAnimation("turnOff");
    }
    while(keySwitchState == LOW){
      keySwitchState = digitalRead(keySwitchPin);
    }
    delay(100);
  }
  if(active){
    keyAState = digitalRead(keyAPin);
    keyBState = digitalRead(keyBPin);
    keyCState = digitalRead(keyCPin);
    useButtonState = digitalRead(useButtonPin);
    resetButtonState = digitalRead(resetButtonPin);
    if(keyAState == LOW){
      currentType = 'a';
      playSound("select");
      Serial.println("a");
      while(keyAState == LOW){
        keyAState = digitalRead(keyAPin);
      }
      delay(100);
    }
    if(keyBState == LOW){
      currentType = 'b';
      playSound("select");
      Serial.println("b");
      while(keyBState == LOW){
        keyBState = digitalRead(keyBPin);
      }
      delay(100);
    }
    if(keyCState == LOW){
      currentType = 'c';
      playSound("select");
      Serial.println("c");
      while(keyCState == LOW){
        keyCState = digitalRead(keyCPin);
      }
      delay(100);
    }
    if(useButtonState == HIGH){
      Serial.print("use ");
      Serial.println(currentType);
      switch(currentType){
        case 'a':
          if(counterA > 0){
            playSound("success");
            shouldRecharge = true;
          }else{
            playSound("fail");
            shouldRecharge = false;
          }
          if(counterA > 1){
            counterA--;
            strip.clear();
            strip.fill(typeAColor,0,counterA);
            strip.fill(white,counterA);
            strip.show();
          }else{
            counterA = 0;
            strip.clear();
            strip.fill(white);
            strip.show();
          }
        break;
        case 'b':
          if(counterB > 0){
            playSound("success");
            shouldRecharge = true;
          }else{
            playSound("fail");
            shouldRecharge = false;
          }
          if(counterB > 1){
            counterB--;
            strip.clear();
            strip.fill(typeBColor,0,counterB);
            strip.fill(white,counterB);
            strip.show();
          }else{
            counterB = 0;
            strip.clear();
            strip.fill(white);
            strip.show();
          }
        break;
        case 'c':
          if(counterC > 0){
            playSound("success");
            shouldRecharge = true;
          }else{
            playSound("fail");
            shouldRecharge = false;
          }
          if(counterC > 1){
            counterC--;
            strip.clear();
            strip.fill(typeCColor,0,counterC);
            strip.fill(white,counterC);
            strip.show();
          }else{
            counterC = 0;
            strip.clear();
            strip.fill(white);
            strip.show();
          }
        break;
      }
      if(shouldRecharge){
      rechargeCounter = rechargeTime;
      while(rechargeCounter > 0 && resetButtonState == LOW){
        resetButtonState = digitalRead(resetButtonPin);
        rechargeCounter--;
        if(flashOn){
          strip.clear();
          flashOn = false;
        }else{
          strip.fill(white);
          flashOn = true;
        }
        strip.show();
        Serial.println("recharging");
        delay(1000);
      }
      Serial.println("recharging done");
      }
    }
    if(resetButtonState == HIGH){
      if(counterA == amountA && counterB == amountB && counterC == amountC){
        goto breakout;
      }else{
      if(partialReset){
        resetCounter = resetTime;
      }
      while(resetButtonState == HIGH && resetCounter > 0){
        Serial.println(counterA);
        for(chaseCounter = chaseChanges; chaseCounter > 0; chaseCounter--){
          chaseLedLit = chaseLedLit + chaseChangeAmount;
          if(chaseLedLit <= 0 || chaseLedLit >= 7){
            chaseChangeAmount = chaseChangeAmount * -1;
          }
          strip.clear();
          strip.setPixelColor(chaseLedLit,white);
          strip.show();
          delay(chaseDelay);
        }
        if(partialReset){
            partialResetCounter--;
          if(partialResetCounter == 0){
            if(counterA < amountA){
              counterA++;
            }
            if(counterB < amountB){
              counterB++;
            }
            if(counterC < amountC){
              counterC++;
            }
            if(counterA == amountA && counterB == amountB && counterC == amountC){
              resetCounter = 1;
            }
            partialResetCounter = partialResetTime;
          }
        }
        resetCounter--;
        Serial.print("reset counter: ");
        Serial.println(resetCounter);
        resetButtonState = digitalRead(resetButtonPin);
      }
      if(resetCounter == 0){
          counterA = amountA;
          counterB = amountB;
          counterC = amountC;
        }
    }
    }
    breakout:
    switch(currentType){
        case 'a':
            if(counterA > 0){
              strip.clear();
              strip.fill(typeAColor,0,counterA);
              strip.fill(white,counterA);
            }else{
              strip.clear();
              strip.fill(white);
            }
        break;
        case 'b':
            if(counterB > 0){
              strip.clear();
              strip.fill(typeBColor,0,counterB);
              strip.fill(white,counterB);
            }else{
              strip.clear();
              strip.fill(white);
            }
        break;
        case 'c':
            if(counterC > 0){
              strip.clear();
              strip.fill(typeCColor,0,counterC);
              strip.fill(white,counterC);
            }else{
              strip.clear();
              strip.fill(white);
            }
        break;
      }
    strip.show();
  }else{
  strip.clear();
  strip.show();
  }
}

void playAnimation(String animationName){
  if(animationName == "startup"){
    strip.fill(red,3,2);
    strip.show();
    delay(200);
    strip.clear();
    strip.setPixelColor(2,orange);
    strip.setPixelColor(5,orange);
    strip.show();
    delay(200);
    strip.clear();
    strip.setPixelColor(1,yellow);
    strip.setPixelColor(6,yellow);
    strip.show();
    delay(200);
    strip.clear();
    strip.setPixelColor(0,lime);
    strip.setPixelColor(7,lime);
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
  }else if(animationName == "turnOn"){
      strip.fill(white,3,2);
      strip.show();
      tone(9,300);
      delay(150);
      noTone(9);
      strip.fill(white,2,4);
      strip.show();
      tone(9,400);
      delay(150);
      noTone(9);
      strip.fill(white);
      strip.show();
      tone(9,500);
      delay(150);
      noTone(9);
      strip.clear();
      strip.show();
  }else if(animationName == "turnOff"){
      strip.fill(white);
      strip.show();
      tone(9,500);
      delay(150);
      noTone(9);
      strip.clear();
      strip.fill(white,2,4);
      strip.show();
      tone(9,400);
      delay(150);
      noTone(9);
      strip.clear();
      strip.fill(white,3,2);
      strip.show();
      tone(9,300);
      delay(150);
      noTone(9);
      strip.clear();
      strip.show();
  }
}


void playSound(String soundName){
  if(!silent){
    if(soundName == "success"){
      // success sound
      tone(buzzerPin,500);
      delay(150);
      tone(buzzerPin,600);
      delay(200);
      noTone(buzzerPin);
    }else if(soundName == "fail"){
      // fail sound
      tone(buzzerPin,200);
      delay(150);
      noTone(buzzerPin);
      delay(100);
      tone(buzzerPin,200);
      delay(150);
      noTone(buzzerPin);
    }else if(soundName == "select"){
      // select sound
      tone(buzzerPin,550);
      delay(75);
      noTone(buzzerPin);
    }
  }
}
