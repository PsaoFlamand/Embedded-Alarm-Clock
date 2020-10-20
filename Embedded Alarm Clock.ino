
#include <Energia.h>
#include <SPI.h>
#include <LCD_screen.h>
#include <LCD_screen_font.h>
#include <LCD_utilities.h>
#include <Screen_HX8353E.h>
#include <Terminal12e.h>
#include <Terminal6e.h>
#include <Terminal8e.h>
#include <Wire.h>
#include <OPT3001.h>
#include <Adafruit_TMP006.h>

Adafruit_TMP006 tempSensor;
//Define instances
Screen_HX8353E myScreen;
opt3001 lightSensor;

//Global Variable Definitions
const int Pin_RedLED = 30;     
const int Pin_Backlight = 39;  
const int Pin_Button1 = 33;    
const int Pin_JoystickX = 2;   
const int Pin_Button2 = 32;    
const int Pin_GreenLED = 38;  
const int Pin_Buzzer = 40;    
const int Pin_UltrasoundTrig = 28; 
const int Pin_UltrasoundEcho = 29; 
const int Pin_AccelX=23;
const int Pin_AccelY=24;
unsigned long nextMillis;     
unsigned long currentMillis;  
int hour = 1;         
int minute = 0;           
int second = 0;            
String timeString;        
String tempString;
int state = 0;           
unsigned long debounceTime;    
char bluetoothIn;              
int timeSet;              
unsigned long debounceTime2;  
int alarmEnabled = 0;     
int alarmHour = 1;       
int alarmMinute = 0;         
String alarmString;          

void setup() {                      //Initial setup
    Serial.begin(9600);             
    Serial1.begin(9600);          
    myScreen.begin();               
    lightSensor.begin();            
    pinMode(Pin_RedLED,OUTPUT);     
    pinMode(Pin_Backlight,OUTPUT);  
    pinMode(Pin_Button1,INPUT);     
    pinMode(Pin_JoystickX,INPUT);  
    pinMode(Pin_Button2,INPUT);     
    pinMode(Pin_GreenLED,OUTPUT);  
    pinMode(Pin_Buzzer,OUTPUT);   
    pinMode(Pin_UltrasoundTrig,OUTPUT); 
    pinMode(Pin_UltrasoundEcho,INPUT); 
    nextMillis=millis()+1000;    //1 second
    attachInterrupt(Pin_Button1,functionbutton1,CHANGE);   
    attachInterrupt(Pin_Button2,functionbutton2,CHANGE);  
    tempSensor.begin(TMP006_CFG_8SAMPLE);
}

void loop() {                           //Main Loop
    currentMillis = millis();          
    if (currentMillis >= nextMillis) {  //Check if it's time for the clock to tick
        nextMillis += 1000;             
        blink(Pin_RedLED);              
        second++;                      
        if (second==60) {              
            second=0;                   
            minute++;                   
            if (minute==60) {          
                minute=0;              
                hour++;              
                if (hour==13) {         
                    hour=1;             //Reset the hour counter
                }
            }
        }
       
        unsigned long readings = lightSensor.readResult();  //Read from the light sensor and save the result as readings
     
        int backlight = map(readings, 0, 500, 50, 255);    
        backlight = constrain(backlight, 50, 255);          //Constrain the output in case it goes out of range
        analogWrite(Pin_Backlight, backlight);              //Write the output to the backlight pin
        while (Serial1.available() > 0){                            //Check Bluetooth availability
            bluetoothIn = Serial1.read();                          
            if (bluetoothIn == 'H' || bluetoothIn == 'h'){          //If the character is an H
                state = 1;                                          //Go to hour-setting state
                timeSet=0;                                          //Clear timeSet variable
            }
            else if (bluetoothIn == 'M' || bluetoothIn == 'm'){     //M for minute-setting state
                state = 2;
                timeSet=0;
            }
            else if (bluetoothIn == 'S' || bluetoothIn == 's'){     //S for second-setting state
                state = 3;
                timeSet=0;
            }
            else if (bluetoothIn == 'C' || bluetoothIn == 'c'){     //C for clock state
                state = 0;
                timeSet=0;
            }
            else if (bluetoothIn >= '0' && bluetoothIn <= '9'){     
                timeSet *= 10;                                      
                timeSet += atoi(&bluetoothIn);                     
            }
            else if (timeSet>0) {                                
                if (state == 1){                                    
                    hour = constrain(timeSet,1,12);                 
                }
                else if (state == 2){                             
                    minute = constrain(timeSet,0,59);
                }
                else if (state == 3){                              
                    second = constrain(timeSet,0,59);
                }
                timeSet=0;                                          //Reset timeSet
            }
            
        }
        if (alarmEnabled == 1 && alarmHour == hour && alarmMinute == minute) { //code to set off the alarm
            state = 6; 
        }
    }
    timeString=i32toa(hour,1,0,2)+":"+i32toa(minute,1,0,2)+":"+i32toa(second,1,0,2);
                                
    timeString.replace(": ",":0");  

   
                                //Write string to LCD screen
                                
    alarmString=i32toa(alarmHour,1,0,2)+":"+i32toa(alarmMinute,1,0,2)+" A"; 
    alarmString.replace(": ",":0");
    myScreen.gText(0,30,alarmString,whiteColour,blackColour,2,2);
    myScreen.dRectangle(0,20,myScreen.screenSizeX(),2,blackColour);
                                //Draw a black rectangle to erase the underlines
                               
    myScreen.dRectangle(0,50,myScreen.screenSizeX(),2,blackColour); 
    float Temp = tempSensor.readDieTempC();
    tempString=i32toa((int)Temp);
    myScreen.gText(0,60,tempString,whiteColour,blackColour,2,2);
    
    int Accel_XValue = analogRead(Pin_AccelX);
    int Accel_YValue = analogRead(Pin_AccelY);
    Serial.println("X = " + i32toa(Accel_XValue));
    timeString=i32toa(Accel_XValue);
    myScreen.gText(0,0,timeString,whiteColour,blackColour,2,2);
    if (abs(Accel_XValue)>=300 || abs(Accel_YValue)>=300){
        if (abs(Accel_XValue)>=abs(Accel_YValue)){
            myScreen.setOrientation(1); 
            myScreen.clear(blackColour);
        }
        else{
            myScreen.setOrientation(0); 
            myScreen.clear(blackColour);

        }
    }
    if (state==1) {                                 //Hour set code
        myScreen.dRectangle(0,20,25,2,whiteColour); //Underline the hour number
        int changeBy = analogRead(Pin_JoystickX);   //Read from the joystick to an integer called changeBy
        changeBy = map(changeBy, 0, 4096, -1, 2);   
        hour += changeBy;                           
        if (hour > 12) {                            
            hour = 1;
        }
        else if (hour < 1) {
            hour = 12;
        }
    }
    else if (state==2) {                            //Minute set code
        myScreen.dRectangle(35,20,25,2,whiteColour); 
        int changeBy = analogRead(Pin_JoystickX);   
        changeBy = map(changeBy, 0, 4096, -1, 2);
        minute += changeBy;
        if (minute > 59) {
            minute = 0;
        }
        else if (minute < 0) {
            minute = 59;
        }
    }
    else if (state==3) {                            //Second set code
        myScreen.dRectangle(70,20,25,2,whiteColour); 
        int changeBy = analogRead(Pin_JoystickX);   
        changeBy = map(changeBy, 0, 4096, -1, 2);
        second += changeBy;
        if (second > 59) {
            second = 0;
        }
        else if (second < 0) {
            second = 59;
        }
    }
    else if (state==4) {                            //Alarm hour set code
        myScreen.dRectangle(0,50,25,2,whiteColour);
        int changeBy = analogRead(Pin_JoystickX);
        changeBy = map(changeBy, 0, 4096, -1, 2);
        alarmHour += changeBy;
        if (alarmHour > 12) {
            alarmHour = 1;
        }
        else if (alarmHour < 1) {
            alarmHour = 12;
        }
    }
    else if (state==5) {                            //Alarm minute set code
        myScreen.dRectangle(35,50,25,2,whiteColour);
        int changeBy = analogRead(Pin_JoystickX);
        changeBy = map(changeBy, 0, 4096, -1, 2);
        alarmMinute += changeBy;
        if (alarmMinute > 59) {
            alarmMinute = 0;
        }
        else if (alarmMinute < 0) {
            alarmMinute = 59;
        }
    }
    else if (state==6) {                //Active alarm code
        blink(Pin_Buzzer);              //Switch the buzzer pin to make a noise
        digitalWrite(Pin_UltrasoundTrig,0);     //Write a 20 microsecond high pulse to the ultrasound for distance check
        delayMicroseconds(20);                 
        digitalWrite(Pin_UltrasoundTrig,1);
        delayMicroseconds(20);
        digitalWrite(Pin_UltrasoundTrig,0);
        long duration = pulseIn(Pin_UltrasoundEcho,HIGH)*0.017;       //Convert from microseconds to cm
        if (duration < 20 && duration != 0) {   //Distance is less than 20 cm and not a time-out
            state = 0;                          
            digitalWrite(Pin_Buzzer,0);      
            alarmMinute = minute + 1;           
            alarmHour = hour;                 
            if (alarmMinute > 59) {            
                alarmMinute = 0;
                alarmHour++;
                if (alarmHour > 12) {
                    alarmHour = 1;
                }
            }
        }
    }
}

void blink(int pin) {        
    int ledState = digitalRead(pin);   
    digitalWrite(pin,!ledState);      
}

void functionbutton1() {                                           
    if (millis()-debounceTime>200 && digitalRead(Pin_Button1)==0){ 
        if (state==5) {                                            
            state=0;                                             
        }
        else if (state==6){                                       
            alarmOff();                                             //Shut off the alarm
        }
        else {
            state+=1;                                             
        }
    }
    debounceTime=millis();                                          //Update the last time the button changed
}

void functionbutton2() {                                           
    if (millis()-debounceTime2>200 && digitalRead(Pin_Button2)==0) { //If the button is pressed and it's been a long time since the last change
        if (state==6) {                     
            alarmOff();                     //Turn off the alarm
        }
        else if (alarmEnabled==1){          //If the alarm is enabled
            alarmEnabled = 0;               //Disable it
            digitalWrite(Pin_GreenLED,0);   //Shut off the LED
        }
        else {
            alarmEnabled = 1;               //If the alarm is disabled, enable it
            digitalWrite(Pin_GreenLED,1);   //Turn on the LED
        }
    }
    debounceTime2=millis();                 //Update the last time the button changed
}

void alarmOff() {                   //Code to turn off the alarm
    state = 0;                    
    alarmEnabled = 0;               //Disable the alarm
    digitalWrite(Pin_Buzzer, 0);    
    digitalWrite(Pin_GreenLED,0);   
}
