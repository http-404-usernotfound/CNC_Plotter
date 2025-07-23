#include <Servo.h>

#define SIZE 32
#define MICROSTEP 16
//================================================================
// Variables for dataExtractor()

String line, arg1, arg2, linesubx, linesuby;

bool Pause = false, first = true;

long xybuffer[SIZE][2];

byte i, bufferCount = 0, xyPut = 0, xyTake = 0;

float x = 0, y = 0;

char r;

//=====================================================================
//Variables for Stepper and Servo

byte penstate = 45, xst = 2, xdir = 3, yst = 4, ydir = 5, j;

float steps_per_mm = 4.978*MICROSTEP;

short yStep, xStep, stepDelay = 40;

int pendelay;

bool penchange = false, runs = false;

long penpos[2] = {0, 0}, xLimit = (long)steps_per_mm*200, yLimit = (long)steps_per_mm*270,
      x0 = 0, y0 = 0, xn = 0, yn   = 0, dx, dy, err, e2;
unsigned long curr = 0, prev = 0;

Servo penActuator;

//================================================================
void setup() {
  Serial.begin(115200);
  
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  
  line.reserve(35);
  arg1.reserve(4);

  penActuator.attach(10);

  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

//===============================================================
void loop() {
  
  while(first) if(Serial.read() == '?') first = false;

    digitalWrite(7, HIGH);
    digitalWrite(8, HIGH);

//=================== Serial Reader ====================//
  r = Serial.peek();
  if(r == '?' && bufferCount<SIZE){
    Serial.read(); r = '\0' ;Serial.write('R');
  }
  
  if (Serial.available() && Serial.peek() != '?') {
    line = Serial.readStringUntil('\n');
  //============ Data Extractor ===========//

    if(line == "exit"){
      xybuffer[xyPut][0] = 0;
      xybuffer[xyPut][1] = 50002;
      xyPut++; bufferCount++; xyPut %= SIZE;
    }

    arg1 = line.substring(0, 3);
    switch(arg1[0]){
      case 'G':
            switch(arg1[2]){
              case '0':
              case '1':
                    i = 3;
                    if(line[4] == 'X'){
                      i += 3;
                      linesubx = "";
                      while(line[i] != ' '){
                        linesubx += line[i];
                        i++;
                      }
                      x = linesubx.toFloat();
                    }
                    if(line.indexOf('Y') > 0){
                      i += 3;
                      linesuby = "";
                      while(line[i] != ' '){
                        linesuby += line[i];
                        i++;
                      }
                      y = linesuby.toFloat();
                    }
                    xybuffer[xyPut][0] = (long)round(x * steps_per_mm);
                    xybuffer[xyPut][1] = (long)round(y * steps_per_mm);
                    xyPut++; bufferCount++; xyPut %= SIZE;
            }
           break;
       case 'M':
            switch(arg1[2]){
              case '3':
                  xybuffer[xyPut][0] = 0;
                  xybuffer[xyPut][1] = 50000;
                  xyPut++; bufferCount++; xyPut %= SIZE;
                  break;
              case '5':
                  xybuffer[xyPut][0] = 0;
                  xybuffer[xyPut][1] = 50001;
                   xyPut++; bufferCount++; xyPut %= SIZE;                  
            }
    }
  }

   if(!runs && bufferCount && !Pause){
         //==== Linear Interpolation ====//
       penpos[0] = xybuffer[xyTake][0];
       penpos[1] = xybuffer[xyTake][1];
       xyTake++; bufferCount--; xyTake %= SIZE;
       
       if( penpos[0]>xLimit || penpos[1]>yLimit ){
        switch(penpos[1]){
          case 50000:
            penstate = 10;
            penchange = true;
            pendelay = 200;
            prev = millis();
            break;
  
          case 50001:
            penstate = 45;
            penchange = true;
            pendelay = 80;
            prev = millis();
            break;
  
          case 50002:
            digitalWrite(7, LOW);
            digitalWrite(8, LOW);
            first = true;
        }
      }
      
      else {
        xn = penpos[0]; yn = penpos[1];
        dx = abs(xn - x0);
        dy = -abs(yn - y0);
        digitalWrite(xdir, xn>x0?HIGH:LOW); digitalWrite(ydir, yn>y0?HIGH:LOW);
        xStep = xn>x0?1:-1; yStep = yn>y0?1:-1;
        err = dx+dy;
        runs = true;
      }
  }
}

SIGNAL(TIMER0_COMPA_vect){
    curr = millis();
    penActuator.write(penstate);
  if((curr - prev)>=pendelay || !penchange){
    penchange = false;
    if(runs){
        e2 = err<<1;
      if(e2>=dy){
        if(x0 != xn){
          err += dy; x0 += xStep;
          digitalWrite(xst, HIGH);
        }
      }
      if(e2<=dx){
        if(y0 != yn){
          err += dx; y0 += yStep;
          digitalWrite(yst, HIGH);
        }
      }
      digitalWrite(xst, LOW); digitalWrite(yst, LOW);
      if(x0==xn && yn==y0) runs = false;
    }

    if(runs){
        e2 = err<<1;
      if(e2>=dy){
        if(x0 != xn){
          err += dy; x0 += xStep;
          digitalWrite(xst, HIGH);
        }
      }
      if(e2<=dx){
        if(y0 != yn){
          err += dx; y0 += yStep;
          digitalWrite(yst, HIGH);
        }
      }
      digitalWrite(xst, LOW); digitalWrite(yst, LOW);
      if(x0==xn && yn==y0) runs = false;
    }
  }
}
