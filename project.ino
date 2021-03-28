#include <Servo.h> 
#include "avr/interrupt.h"
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "pitches.h"


//linie melodica de petrecere
int melody[] = {NOTE_E4,NOTE_E4,NOTE_E4,NOTE_E4,NOTE_E4,NOTE_E4,NOTE_E4, NOTE_G4,NOTE_C4,NOTE_D4,NOTE_E4,0, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4,
  NOTE_E4,NOTE_E4,NOTE_E4,NOTE_E4, NOTE_D4, NOTE_D4,NOTE_E4, NOTE_D4,NOTE_G4};
 
int noteDurations[] = {1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2 };

//pini folositi pentru buzzer
const int buzzPin=8;
const int partyPin=9;

//pini senzor ultrasonic
const int echoPin=A10;
const int trigPin=A11;

//pini punte H
const int ENA=51;
const int IN1=52;
const int IN2=53;

//pin fotorezistor
const int ledSenzorPin = A1;

byte clock1[8] = {
B11111,
B11111,
B01110,
B00100,
B00100,
B01010,
B10001,
B11111
};

byte clock2[8] = {
B11111,
B10111,
B01110,
B00100,
B00100,
B01110,
B10001,
B11111
};
byte clock3[8] = {
B11111,
B10011,
B01110,
B00100,
B00100,
B01010,
B11101,
B11111
};
byte clock4[8] = {
B11111,
B10001,
B01110,
B00100,
B00100,
B01110,
B11101,
B11111
};
byte clock5[8] = {
B11111,
B10001,
B01110,
B00100,
B00100,
B01010,
B11111,
B11111
};
byte clock6[8] = {
B11111,
B10001,
B01010,
B00100,
B00100,
B01110,
B11111,
B11111
};

byte umbrelaD[8] = {
B01110,
B11111,
B11111,
B11111,
B00100,
B00100,
B00101,
B00110
};

byte umbrelaI[8] = {
B00100,
B01110,
B01110,
B11111,
B00100,
B00100,
B00101,
B00110
};

Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
LiquidCrystal_I2C lcd(0x27,16,2);

OneWire oneWire(A5);//senzor temperatura
DallasTemperature sensors(&oneWire);
 // select the input pin for LDR

//A vedea de cat timp ne aflam in zona de relaxare
volatile int ora,minut,secunda;

// variabile pentru pozitia motorului servo
int pos = 0;    
int ledSensorValue=0;

//senzor ultrasonic
int distance=30;;
long duration;
int zonaRosie=0;

//variabila care ne spune daca butonul de party e apasat sau nu 
int party=1;

//variabila ce contine temperatura
float temperatura=0.0f;

//variabile pentru intreruperile folosite pentru ridicare si coborare
volatile int sus=0;
volatile int jos=0;

void setup() 
{ 
  Serial.begin(9600);
  minut=0;
  secunda=0;
  ora=0;

  //BUZZER
  pinMode(buzzPin,OUTPUT);
  pinMode(partyPin,INPUT_PULLUP);

  //TIMER INTERN
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(showClock);

  //LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0,clock1);
  lcd.createChar(1,clock2);
  lcd.createChar(2,clock3);
  lcd.createChar(3,clock4);
  lcd.createChar(4,clock5);
  lcd.createChar(5,clock6);
  lcd.createChar(6,umbrelaD);
  lcd.createChar(7,umbrelaI);

  //senzor ultrasonic
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);

  //motor DC
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);
  pinMode (ENA, OUTPUT);

  //TEMPERATURA
  sensors.begin();//senzor temperatura

  //MOTOR SERVO
  myservo.attach(7);  // attaches the servo on pin 9 to the servo object 
  myservo.write(pos);


  //INTRERUPERI EXTERNE
  pinMode(2,INPUT_PULLUP);//intrerupere externa pune acoperis(caz ploaie)
  pinMode(3,INPUT_PULLUP);//scoate acoperis, vreau soare

  attachInterrupt(digitalPinToInterrupt(2), susUmbrela,RISING);
  attachInterrupt(digitalPinToInterrupt(3), josUmbrela,RISING);

  
} 
 
void loop() 
{ 
  //CITIM DE LA BUTONUL DE PETRECERE
  party=digitalRead(partyPin);

  if(party==0)
  {
    muzica();//BUNCTIE PENTRU BUZZER
  }
  //CITIRE VALOARE FOTOREZISTOR
  ledSensorValue = analogRead(ledSenzorPin); // read the value from the sensor
  Serial.println(ledSensorValue);

  //se ia temperatura de la senzor
  sensors.requestTemperatures();
  temperatura=sensors.getTempCByIndex(0);

  //afisarea pe lcd
  afisareLcd();

  //DACA VALOARE FOTOREZISTORULUI E PESTE 500 SAU DACA ARE LOC INTRERUPEREA CE PUNE VARIABILA SUS PE 1
  if(ledSensorValue>500 || sus==1)
  {
    //DACA POZITIE MOTORULUI SERVO E 0
    if(pos<=0)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ridicare ");
      lcd.setCursor(0,1);
      lcd.print("Umbrela ");
      lcd.write(6);

      //SE RIDICA ACOPERISUL
      for(pos = 0; pos <= 90; pos += 1) // goes from 0 degrees to 180 degrees 
      {                                  // in steps of 1 degree 
        myservo.write(pos);              // tell servo to go to position in variable 'pos' 
        delay(10);                       // waits 15ms for the servo to reach the position 
      }
      lcd.clear();
    }
    
  }
  else if(ledSensorValue<=500 || jos==1)
  {
    if(pos>=90)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Coborare ");
      lcd.setCursor(0,1);
      lcd.print("Umbrela ");
      lcd.write(7);
      //SE COBOARA ACOPERISUL
      for(pos = 90; pos>=0; pos-=1)     // goes from 180 degrees to 0 degrees 
      {                                
        myservo.write(pos);              // tell servo to go to position in variable 'pos' 
        delay(10);                       // waits 15ms for the servo to reach the position 
      } 
      lcd.clear();
    }
    
  }

  //senzor ultrasonic
  intrus();

 
  if(temperatura>=25)
  {
    //PORNIRE MOTOR DC
    ventilatorOn();
  }
  else
  {
    //OPRIRE MOTOR DC
    ventilatorOff();
  }
 
} 

//FUNCTIE PENTRU BUZZER
void muzica()
{
  lcd.clear();
  lcd.print("PARTY TIME");
   for(int thisNote=0;thisNote<26;thisNote++)
  {
    int noteDuration =noteDurations[thisNote]*200;

    if(melody[thisNote]==0)
    {
      delay(noteDuration);
    }
    else
    {
      tone(buzzPin,melody[thisNote],noteDuration);
    }
    delay(noteDuration*2);
    noTone(buzzPin);
  }
  lcd.clear();
}

void ventilatorOn()
{
  analogWrite(ENA,255);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void ventilatorOff()
{
  analogWrite(ENA,0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void susUmbrela()
{
  sus=1;
  jos=0;
}

void josUmbrela()
{
  jos=1;
  sus=0;
}

//INTRERUPERE INTERNA
void showClock(void)
{
  secunda++;
  if(secunda>=60)
  {
    secunda=0;
    minut++;
    if(minut>=60)
    {
      minut=0;
      ora++;
    }
  }
}

//FUNCTIE CE DETECTEAZA UN INTRUS
void intrus()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // CALCULUL DISTANTEI 

  if(distance<10)
  {
    if(zonaRosie==0)
    {
      lcd.clear();
      zonaRosie=1;
    }
    lcd.setCursor(0,1);
    lcd.print("INTRUS la ");
    lcd.print(distance);
    delay(50);
    distance=30;
    
  }
  else
  {
    if(zonaRosie==1)
    {
      lcd.clear();
      zonaRosie=0;
    }

    distance=30;
    
  }
}

//AFISAREA PENTRU TEMPERATURA SI PENTRU CRONOMETRU 
void afisareLcd()
{
  
  lcd.setCursor(0,0);
  lcd.print("Timp ");
  if(ora<10)
  {
    lcd.print(0);
  }
  lcd.print(ora);
  lcd.print(":");
  if(minut<10)
  {
    lcd.print(0);
  }
  lcd.print(minut);
  lcd.print(":");
  if(secunda<10)
  {
    lcd.print(0);
  }
  lcd.print(secunda);
  lcd.print(" ");
  lcd.write(byte(secunda%6));

  
  lcd.setCursor(0,1);
  lcd.print("Temp:");
  lcd.setCursor(6,1);
  lcd.print(temperatura);
}
//a doua procedură ISR
