#include <LiquidCrystal.h>

#define LED_PIN 13
#define READ_PIN 2
#define PWM_PIN 3

#define POT_PIN 0

volatile long timeSinceLast = 0;
volatile long period = 0;
volatile long pilseLength = 0;

ISR(INT0_vect) {
  int currentTime = TCNT1;
  if (digitalRead(READ_PIN) == HIGH) {
    timeSinceLast = currentTime;
    period = pilseLength + currentTime;
  } else {
    TCNT1 = 0;
    pilseLength = currentTime - timeSinceLast;
  }
}

int currState = LOW;
LiquidCrystal lcd(5, 6, 8, 9, 10, 11);

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(READ_PIN, INPUT);
  pinMode(PWM_PIN, OUTPUT);
  
  
  lcd.begin(16, 2);
  lcd.print("hello, world!");
  Serial.begin(9600);
  TCCR1A = 0;    // set timer1 operation mode to normal
  //TIFR1 |= _BV(OCF1A);     // clear any pending interrupts; 
  //TIMSK1 |=  _BV(OCIE1A) ; // enable the output compare interrupt 
  
  EICRA |= (1 << ISC00); // ext.int. any logic change
  EIMSK |= (1 << INT0);  // enable INT0
  TCCR1B = 2; // set prescaler 8
  // change timer2 prescaler. Timer2 controls PWM no pin 3
  TCCR2B &= 0xF8;  // clear prescaler bits
  TCCR2B |= 5;     // set 1024 prescaler
  
  Serial.println(TCCR0B);
  Serial.println(TCCR1B);
  Serial.println(TCCR2B);
}

void loop()
{
  blink();
      
  byte potValue = analogRead(POT_PIN)/4;
  analogWrite(PWM_PIN, potValue);
  
  lcd.setCursor(0, 0);
  lcd.print("Pulse(ms): ");
  lcd.print(1000.0 / (double)(F_CPU / (pilseLength * 8)));
  lcd.setCursor(0, 1);
  lcd.print("Period(ms): ");
  lcd.print(1000.0 / (double)(F_CPU / (period * 8)));
  
  Serial.print("Pulse(ms): ");
  Serial.print(1000.0 / (double)(F_CPU / (pilseLength * 8)));
  Serial.print(" Period(ms): ");
  Serial.print(1000.0 / (double)(F_CPU / (period * 8)));
  Serial.print(" PotValue: ");
  Serial.println(potValue);
 //Serial.println(TCNT1);
 
 //char str[255];
 //sprintf(str, "qwe %i", 555);
 //Serial.println(str);

 delay(10);
}

void blink() {
   digitalWrite(LED_PIN, currState);
     if (currState == LOW)
      currState = HIGH;
    else
      currState = LOW;
}


