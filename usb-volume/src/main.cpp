#include <Arduino.h>
#include <HID-Project.h>
// #include <Encoder.h>
#include <Rotary.h>
#include <cppQueue.h>

// Encoder encoder( 10, 18, 14 );

Rotary r{};
cppQueue	q(sizeof(RotaryOutput), 16, LIFO);


void setupTimerInterrupt()
{
  TCCR3A = 0; // set entire TCCR3A register to 0
  TCCR3B = 0; // same for TCCR3B
  TCNT3  = 0; // initialize counter value to 0

  // set compare match register (write to the high bit first)
  OCR3AH = 0;

  // set compare match register for particular frequency increments
//  OCR3AL = 133; // = (16000000) / 64 / 2000  -> 133   This is  clock_frequency / prescaler / desired_frequency  ( 2 KHz, 0.5ms)
//  OCR3AL = 50;  // = (16000000) / 64 / 5000  ->  50   This is  clock_frequency / prescaler / desired_frequency  ( 5 KHz, 0.2ms)
//  OCR3AL = 25;  // = (16000000) / 64 / 10000 ->  25   This is  clock_frequency / prescaler / desired_frequency  (10 kHz, 0.1ms)
  OCR3AL = 25;

  // enable timer compare interrupt
  TIMSK3 = (1 << OCIE3A);

  // turn on mode 4 (CTC mode) (up to OCR3A)
  TCCR3B |= (1 << WGM32);

  // Set CS10 and CS12 bits for 64 prescaler
  TCCR3B |= (1 << CS30) | (1 << CS31);

  // More information at
  // http://medesign.seas.upenn.edu/index.php/Guides/MaEvArM-timers
  //
}


void process(){
  auto a = digitalRead(18);
  auto b = digitalRead(10);
  auto btn = !digitalRead(14);

  RotaryOutput s = r.process(a, b, btn);
  if (s != RotaryOutput::NONE)
    q.push(&s);
}


RotaryOutput get_event(){
  RotaryOutput event = RotaryOutput::NONE;
  cli();
  q.pop(&event);
  sei();
  return event;
}

SIGNAL(TIMER3_COMPA_vect)
{
  process();
}


void setup() {
  // Common for a and b
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);

  // set up pins
  pinMode(14, INPUT_PULLUP); // btn
  pinMode(10, INPUT_PULLUP); // a
  pinMode(18, INPUT_PULLUP); // b

  // set up timer to call Rotary::process
  setupTimerInterrupt();

  Keyboard.begin();
}


void loop() {
  auto event = get_event();
  switch(event){
    case RotaryOutput::BTN_SHORT:
      Keyboard.write(KEY_VOLUME_MUTE);
      break;

    case RotaryOutput::BTN_LONG:
      Keyboard.write(MEDIA_PLAY_PAUSE);
      break;

    case RotaryOutput::CCW:
      Keyboard.write(KEY_VOLUME_DOWN);
      break;

    case RotaryOutput::CW:
      Keyboard.write(KEY_VOLUME_UP);
      break;

    default:
      break;
  }
}
