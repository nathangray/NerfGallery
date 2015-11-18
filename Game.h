
#ifndef Servo_h
#include <Servo.h>
#endif

#ifndef Metro_h
#include <Metro.h>
#endif
#ifndef Game_h
#define Game_h

#include "Arduino.h"

// Moving can trigger it, wait before listening
#define MOVE_DELAY 500 // (ms)

// How sensitive.  Analog must be over this
#define HIT_THRESHOLD 80  // 20 works well for a light tap, 100 is good for actual play
#define HIT_SCORE 1

// Target states
#define T_IDLE 1
#define T_SIDE_A 0
#define T_SIDE_B 2
#define T_RANDOM 3
#define T_HIT 4

// Struct for a target
class Target {
  public:
    Target(byte analogPin, byte servoPin, int bScore);
    
    void attach();
    // Checks for a hit & returns the score change
    int hit();
    // Sets the target to a state
    byte setState(byte state);
    byte getState();
    // Allows to check if it's moving
    boolean isMoving();
    
    unsigned int bSideDelay;
    static uint8_t hit_counter;
    
  private:
    // Pins
    byte analogPin;
    byte servoPin;
    byte servoPower;
    
    // Score (+ or -) for the "B" side
    int otherSideScore;
    
    // Supposed current state of this
    byte state;
    
    Servo servo;
    
    // State vars
    boolean moving;
    unsigned long moveTimer;
    unsigned long bSideTimer;
};

extern Target *targets[];
extern int T_COUNT;

class Game {
  public:
    virtual void start(unsigned int time_limit);
    virtual boolean tick();
    virtual void score(int change);
    int getScore();
    unsigned int getTime();
    virtual void stop();
  protected: 
    int _score;
    uint8_t _time_counter;
    unsigned int _time;
};

class Knockdown : public Game {
    virtual void start(unsigned int time_limit);
    boolean tick();
    void score(int change);
    void stop();
};

class Timed : public Game {
  virtual void start(unsigned int time_limit);
    boolean tick();
    void score(int change);
    void stop();
  private:
    unsigned long *_on_targets;
};

#endif
