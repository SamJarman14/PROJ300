#include "mbed.h"

DigitalOut Trigger(D7); 
DigitalIn Echo(D2);
Timer pulse;

int getdistance()
{ //Function Name to be called
int EchoPulseWidth=0,EchoStart=0,EchoEnd=0; //Assign and set to zero the local
//variables for this function
Trigger = 1; //Signal goes High i.e. 3V3
wait_us(100); //Wait 100us to give a pulse width
//Triggering the Ultrasonic Module
Trigger = 0; //Signal goes Low i.e. 0V
pulse.start(); //Start the instance of the class timer
pulse.reset(); //Reset the instance of the Timer
//Class
while(Echo == 0 && EchoStart < 25000){ //wait for Echo to go high
EchoStart=pulse.elapsed_time().count(); //Conditional 'AND' with timeout
//to prevent blocking
}
while(Echo == 1 && ((EchoEnd - EchoStart) < 25000)){//wait for echo to return low
EchoEnd=pulse.elapsed_time().count(); //Conditional 'OR' with timeout to
// prevent blocking!
}
EchoPulseWidth = EchoEnd - EchoStart; //time period in us
pulse.stop(); //Stop the instance of the class
//timer
return (int)EchoPulseWidth/5.8f; //calculate distance in mm and
//return the value as a float
}