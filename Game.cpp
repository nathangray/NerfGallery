#include "Arduino.h"
#include "Game.h"


Target::Target(byte analogPin, byte servoPin, int bScore)
{
  this->analogPin = analogPin;
  this->servoPin = servoPin;
  this->otherSideScore = bScore;
}
void Target::attach() {
  this->servo.attach(servoPin);
  pinMode(this->analogPin, INPUT);
  #ifndef T_COUNT
  T_COUNT++;
  #endif
  this->setState(T_IDLE);
}
int Target::hit() {
  int score = 0;
  if(this->moving)
  {
    if(this->moveTimer < millis())
    {
      this->moving = false;
      this->servo.detach();
    }
    else
    {
      return score;
    }
  }
  if(this->state == T_IDLE || hit_counter) return score;
  
  // Check to see if B side has timed out
  if(this->state == T_SIDE_B && this->bSideDelay && this->bSideTimer < millis())
  {
    this->setState(T_RANDOM);
    return 0;
  }
  int analog = analogRead(this->analogPin);
  if(analog > HIT_THRESHOLD)
  {
    score = this->state == T_SIDE_A ? HIT_SCORE : this->otherSideScore;
    Serial.print(this->analogPin); Serial.print(" Hit "); Serial.println(analog);
    hit_counter++;
    this->setState(T_IDLE);
  }
  return score;
}
byte Target::getState()
{
  return this->state;
}
byte Target::setState(byte state)
{
  if(state == this->state) return state;
  this->servo.attach(this->servoPin);
  byte rand;
  switch(state)
  {
    case T_IDLE:
      this->servo.write(90);
      this->moving = true;
      break;
    case T_SIDE_A:
      this->servo.write(5);
      this->moving = true;
      break;
    case T_SIDE_B:
      this->servo.write(170);
      this->moving = true;
      // B side is temporary
      if(this->bSideDelay) this->bSideTimer = millis() + this->bSideDelay;
      break;
    case T_RANDOM:
      rand = random(2);
      return this->setState(rand == 0 ? T_SIDE_B :T_SIDE_A);
    default:
      return this->setState(T_IDLE);
  }
  this->moveTimer = millis() + abs(state - this->state) * MOVE_DELAY;
  this->state = state;
  return this->state;
}
boolean Target::isMoving()
{
  return this->moving;
}
uint8_t Target::hit_counter = 0;


/**
 * Game base class
 * 
 * Handles common game stuff, like timer
 */
void Game::start(unsigned int time_limit)
{
  this->_time = time_limit;
  this->_score = 0;
  Serial1.print("M: "); Serial1.write(time_limit);Serial1.println();
  Serial1.print("S: "); Serial1.println(this->_score);
  Serial1.print("T: "); Serial1.println(this->_time);
}
int Game::getScore()
{
  return this->_score;
}
unsigned int Game::getTime()
{
  return this->_time;
}
boolean Game::tick() {
  this->_time_counter++;
  if(this->_time_counter >= 10)
  {
    this->_time_counter = 0;
    this->_time--;
    Serial1.print("T: "); Serial1.println(this->_time);
  }
  
  return this->_time > 0;
}
void Game::stop() {
  Serial1.print("S: "); Serial1.println(this->getScore());
  for(byte i = 0; i < T_COUNT; i++)
  {
    targets[i]->setState(T_IDLE);
  }
}

/**
 * Knockdown
 *
 * Knockdown starts with all targets, and you knock them down
 */
void Knockdown::start(unsigned int time_limit) {
  Game::start(time_limit);
  Serial.println("Knockdown");
  for(byte i = 0; i < T_COUNT; i++)
  {
    targets[i]->setState(T_RANDOM);
    targets[i]->bSideDelay = random(4000,10000);
  }
}
boolean Knockdown::tick() {
  if(!Game::tick()) return false;
  
  boolean stop = true;
  for(byte i = 0; i < T_COUNT; i++)
  {
    stop = stop && (targets[i]->getState() == T_IDLE);
  }
  return !stop;
}
void Knockdown::score(int change) {
  if(change)
  {
    this->_score += change;
    Serial1.print("S: "); Serial1.println(this->_score);
  }
}
void Knockdown::stop() {
  Serial.println("Knockdown stop");
  Game::stop();
  while(this->_time > 2)
  {
    Knockdown::score(1);
    this->_time-=2;
    Serial1.print("T: "); Serial1.println(this->_time);
    delay(200);
  }
  Serial1.print("T: "); Serial1.println(0);
}

/**
 * Timed
 *
 * Timed gives you a time limit, and pops up targets as you go
 */
void Timed::start(unsigned int time_limit) {
  Serial.println("Timed");
  Game::start(time_limit);
  
  _on_targets = new unsigned long[T_COUNT];
  for(byte i = 0; i < T_COUNT; i++)
  {
    targets[i]->setState(T_IDLE);
    targets[i]->bSideDelay = 2000;
    this->_on_targets[i] = 0;
  }
}
boolean Timed::tick() {
  
  // Pick one
  byte rand = random(20);
  
  // Turn off any needed targets
  for(byte i = 0; i < T_COUNT; i++)
  {
    if(this->_on_targets[i] && this->_on_targets[i] < millis())
    {
      this->_on_targets[i] = 0;   
      targets[i]->setState(T_IDLE);
    }
    else if (!this->_on_targets[i] && i == rand && !targets[rand]->isMoving())
    {
      byte side = random(4);
      targets[rand]->setState(side < T_RANDOM ? side : T_RANDOM);
      this->_on_targets[i] = millis() + 5000;
    }
  }
  return Game::tick();
}

void Timed::score(int change) {
  if(change)
  {
    this->_score += change;
    Serial1.print("S: "); Serial1.println(this->_score);
  }
}

void Timed::stop() {
  Game::stop();
}
