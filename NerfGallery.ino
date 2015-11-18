
/**
 * NERF gallery
 *
 * A shooting gallery for nerf type guns using piezo elements on targets.
 * 
 * Servos hold the targets, which have a "good" side and either a "bad" side
 * or a bonus side.  One side is turned to the player, and when hit their 
 * score is adjusted accordingly.  Nice sounds & reactions happen too.
 *
 * Copyright Nathan Gray, 2015
 */

#include <TimerOne.h>
#include <Bounce2.h>
#include <Servo.h>
#include <Metro.h>

#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int T_COUNT = 9;
const int LIGHTS = 11;

#include "Game.h"

// Game States
#define GAME_IDLE 0
#define GAME_PLAY 1
#define GAME_OVER 10

Target *targets[] = {
  //    A                      B                      C
  new Target(0,0,-1),   new Target(1,1,-1),  new Target(2,2,-1),  // 1
  new Target(3,3,-1),   new Target(4,4, 5),  new Target(5,5,-1),  // 2
  new Target(6,6, 5),   new Target(7,9,-1),  new Target(8,10,5),  // 3
  
};
//int T_COUNT = sizeof(targets) / sizeof(targets[0]);


// Display
#define NUM_LEDS 28
#define LED_PIN 24
const byte digits[10][7] = {
  {0,1,1,1,1,1,1}, // 0
  {0,0,0,0,0,1,1}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,0,0,1,1,1}, // 3
  {1,0,1,0,0,1,1}, // 4
  {1,1,1,0,1,1,0}, // 5
  {1,1,1,1,1,1,0}, // 6
  {0,1,0,0,0,1,1}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,0,1,1,1}  // 9
};

// Different game types
Knockdown knockdown;
Timed timed;
Game  *games[] = {
  &knockdown,
  &timed
};
Game *game;
Metro gameTick = Metro(100);

// State variables
byte currentState = GAME_IDLE;
int score = 0;
void setup() {
  #if FASTADC
   // set prescale to 16
   sbi(ADCSRA,ADPS2) ;
   cbi(ADCSRA,ADPS1) ;
   cbi(ADCSRA,ADPS0) ;
  #endif
  Serial1.begin(9600);
  Serial1.setTimeout(1000);
  pinMode(LIGHTS,OUTPUT);
  digitalWrite(LIGHTS,LOW);
  
  // Servo check
  for(byte i = 0; i < T_COUNT; i++)
  {
    delay(100);
    targets[i]->setState(T_SIDE_B);
  }
  for(byte i = 0; i < T_COUNT; i++)
  {
    delay(100);
    targets[i]->setState(T_SIDE_A);
  }
  for(byte i = 0; i < T_COUNT; i++)
  {
    delay(100);
    targets[i]->setState(T_IDLE);
  }
  
  for(byte i = 0; i < T_COUNT; i++)
  {
    int score = targets[i]->hit();
  }
  delay(5000);
  
  
  // Pick game type
  Serial1.println("Choose");
  for(byte i = 0; i < 2; i++)
  {
    targets[i]->bSideDelay = 0;
    targets[i]->setState(T_SIDE_B);
  }
  int type = 10;
  while(type == 10)
  {
    for(byte i = 0; i < 2; i++)
    {
      if(targets[i]->hit())
      {
        type = i;
      }
    }
  }
  game = games[type];
  game->start(60);
  digitalWrite(LIGHTS,HIGH);
}

void loop() {
  if(gameTick.check() && !game->tick())
  {
    game->stop();
    digitalWrite(LIGHTS,LOW);
    
    delay(1000);
    
    // Call this to make sure servos turn off
    checkHits();
    
    boolean start= false;
    while(!start)
    {
      start = Serial1.find("Start");
      Serial.print(".");
    }
    game->start(60);
    digitalWrite(LIGHTS,HIGH);
    gameTick.reset();
  }
  
  checkHits();
}

void checkHits() {
  for(byte i = 0; i < T_COUNT; i++)
  {
    game->score(targets[i]->hit());
  }
  // Reset hit counter
  Target::hit_counter = 0;
}

