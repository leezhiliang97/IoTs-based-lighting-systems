/*************************************************************
		 Download latest Blynk library here:
		https://github.com/blynkkk/blynk-library/releases/latest
	
		Blynk is a platform with iOS and Android apps to control
		Arduino, Raspberry Pi and the likes over the Internet.
		You can easily build graphic interfaces for all your
		projects by simply dragging and dropping widgets.
	
		Downloads, docs, tutorials: http://www.blynk.cc
		Sketch generator:           http://examples.blynk.cc
		Blynk community:            http://community.blynk.cc
		Follow us:                  http://www.fb.com/blynkapp
									http://twitter.com/blynk_app
	
		Blynk library is licensed under MIT license
		This example code is in public domain.
	
		*************************************************************
	
		This example shows how to use ESP8266 Shield (with AT commands)
		to connect your project to Blynk.
	
		WARNING!
		It's very tricky to get it working. Please read this article:
		http://help.blynk.cc/hardware-and-libraries/arduino/esp8266-with-at-firmware
	
		Change WiFi ssid, pass, and Blynk auth token to run :)
		Feel free to apply it to any other example. It's simple!
		*************************************************************/
	
	/* Comment this out to disable prints and save space */
	#define BLYNK_PRINT Serial
	
	
	#include <ESP8266_Lib.h>
	#include <BlynkSimpleShieldEsp8266.h>
	// You should get Auth Token in the Blynk App.
	// Go to the Project Settings (nut icon).
	char auth[] = "QY7YSlMt3IVPUtmTV8dnoGIfMq5MTE74";  //own GCP
	
	// Your WiFi credentials.
	// Set password to "" for open networks.
	char ssid[] = "Light&Easy@unifi";
	char pass[] = "simple09";
	
	// Hardware Serial on Mega, Leonardo, Micro...
	//#define EspSerial Serial1
	
	// or Software Serial on Uno, Nano...
	#include <SoftwareSerial.h>
	SoftwareSerial EspSerial(3, 4); // RX, TX
	
	// Your ESP8266 baud rate:
	#define ESP8266_BAUD 9600
	
	ESP8266 wifi(&EspSerial);
	
	//pirsensor declare
	#define pirPin 12
	#define ledPin 6   //socket 
	#define ledPin2 7	//bulb 
	
	int PIRValue = 0;
	//int IDRValue = high;
	
	//ldr declear
	#define ldrPin A0
	
	#include <SPI.h> //serialclock?
	
		#include <TimeLib.h>
		#include <WidgetRTC.h>
	
	//declare library var to use 
	BlynkTimer timer;
	WidgetRTC rtc;
		
		// Digital clock display of the time
		void clockDisplay()
		{
			// You can call hour(), minute(), ... at any time
			// Please see Time library examples for details
		
			String currentTime = String(hour()) + ":" + minute() + ":" + second();
			String currentDate = String(day()) + " " + month() + " " + year();
			//Serial.print("Current time: ");
			//Serial.print(currentTime);
			// Serial.print(" ");
			//Serial.print(currentDate);
			//Serial.println();
	
			//SEND TIME TO THE APP
			Blynk.virtualWrite(V24, currentTime);
			// Send date to the App
			Blynk.virtualWrite(V25, currentDate);
		}
		
	
		
		BLYNK_CONNECTED() {
			// Synchronize time on connection
			rtc.begin();
			Blynk.syncAll();
		}
	
		
	
	int timerPIRSensor;
	int timerAlarmSensor;
	int timerblinkLedWidget;
	
	int selectMode;
	int selectMode2;
	int intruderState; 
	int alarmArm;
	int buttonState;
	
	int pirDelay;
	
	boolean bothFlag;
	boolean socketFlag;
	boolean bulbFlag;
	boolean alarmFlag;
	
	
	void setup()
	{
		// Debug console
		Serial.begin(9600);
			
		delay(10);
	
		//pinmode
		pinMode(pirPin, INPUT);
		digitalWrite(ledPin, HIGH);
		pinMode(ledPin, OUTPUT);
		digitalWrite(ledPin2, HIGH);
		pinMode(ledPin2, OUTPUT);
	
	
		// Set ESP8266 baud rate
		EspSerial.begin(ESP8266_BAUD);
		delay(10);
	
		Blynk.begin(auth, wifi, ssid, pass, "34.72.72.157", 8080); //own GCP
	
	
		//custom timer 
		timerPIRSensor = timer.setInterval(1000L, PIRSensor); 
		timer.disable(timerPIRSensor);  //disable until needed
	
		timerAlarmSensor = timer.setInterval(1000L, AlarmSensor); 
		timer.disable(timerAlarmSensor); 
	
		timerblinkLedWidget = timer.setInterval(50, blinkLedWidget); 
			timerblinkLedWidget = timer.setInterval(50, blinkLedWidget2); 
	
		timer.setInterval(1000L, clockDisplay);
	
		timer.setInterval(60000L, activetoday);  // check every 60s if ON / OFF trigger time has been reached
	
	
	}
	
		
	
	void loop()
	{
		Blynk.run();
		timer.run();  
	}
	
	void activetoday(){         // check if schedule #1 should run today
		if(year() != 1970){
		Blynk.syncVirtual(V4);  // sync scheduler #1  
		}
	}
	

boolean timerOn;

	BLYNK_WRITE(V4) {   // Scheduler #1 Time Input widget  
		TimeInputParam t(param);
		unsigned int nowseconds = ((hour() * 3600) + (minute() * 60) + second());
		unsigned int startseconds = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);  
		unsigned int stopseconds = (t.getStopHour() * 3600) + (t.getStopMinute() * 60);
		int dayadjustment = -1;  
			if(weekday() == 1){
			dayadjustment = 6; // needed for Sunday Time library is day 1 and Blynk is day 7
			}
			if(t.isWeekdaySelected((weekday() + dayadjustment))){ //Time library starts week on Sunday, Blynk on Monday  
			//Schedule is ACTIVE today 
			if(nowseconds >= startseconds - 31 && nowseconds <= startseconds + 31 ){    // 62s on 60s timer ensures 1 trigger command is sent
				//Blynk.virtualWrite(V0, 255);  // turn on virtual LED
				if (bothFlag){
					digitalWrite(ledPin, !HIGH);  
					digitalWrite(ledPin2, !HIGH);  
				}
				
				else if (socketFlag){
					digitalWrite(ledPin, !HIGH);  
				}
	
				else if (bulbFlag){
					digitalWrite(ledPin2, !HIGH);  
				}

				Serial.println("Schedule 1 started");
			}                  
			if(nowseconds >= stopseconds - 31 && nowseconds <= stopseconds + 31 ){   // 62s on 60s timer ensures 1 trigger command is sent
				//Blynk.virtualWrite(V0, 0);   // turn OFF virtual LED
					if (bothFlag){
					digitalWrite(ledPin, !LOW);  
					digitalWrite(ledPin2, !LOW);  
				}
				
				else if (socketFlag){
					digitalWrite(ledPin, !LOW);  
				}
	
				else if (bulbFlag){
					digitalWrite(ledPin2, !LOW);  
				} 
				Serial.println("Schedule 1 finished");
			}               
		}
	}
	
	
	
	
	WidgetLED led1(V1); //virtual pin 1
	void blinkLedWidget() {
		int state = digitalRead(ledPin);
		if (state == !HIGH) {
			led1.on();
			//Blynk.virtualWrite(V0,1);  
		} else {
			led1.off();
			//Blynk.virtualWrite(V0,0);  
		}
	}
	
	
	WidgetLED led2(V5); //virtual pin 1
	void blinkLedWidget2() {
		int state2 = digitalRead(ledPin2);
		if (state2 == !HIGH) {
			led2.on();
			//Blynk.virtualWrite(V0,1);  
		} else {
			led2.off();
			//Blynk.virtualWrite(V0,0);  
		}
	}
	
	int ldrValue = 0;
	
	//this function is runing with setInterval(1sec) when called
	void PIRSensor() {  
		if(digitalRead(pirPin) == HIGH) {   
			
			Serial.println("Motion deteced.");
			PIRValue = 1;
	
			ldrValue = analogRead(ldrPin);
			Serial.print("ldrSensor:" ); //prints the values coming from the sensor on the screen
			Serial.println(ldrValue);
	
			
	
			if (ldrValue <= 200 && selectMode == 3){          //ON LED only when the mode  is automatic    
			digitalWrite(ledPin2, !HIGH);         
			}                                 
		}
	
		
		if(digitalRead(pirPin) == LOW) {
	
			Serial.println("Motion ended.");          
			PIRValue = 0;
	
			if (PIRValue == 0 && selectMode == 3){  //OFF LED only when the mode is uaotmatic 
				//digitalWrite(ledPin, !LOW);      
				
				timer.setTimeout(pirDelay*1000, ACTIONOFF);  
				Serial.println(pirDelay*1000);
				
			}
	
		}    
		
	}
	
	//this function seems not working properly when using with setInterval
	void ACTIONOFF(){
	
		if (PIRValue == 0 && selectMode == 3){
	
			digitalWrite(ledPin2, !LOW);
		led1.off();
		Serial.println("led off"); 
		
	
		}
	
	
	}
	
	
	//slider for fan control
	BLYNK_WRITE(V3)  // Runs when slider widget (on V0) is moved.  Set limits 0-1023 for ESP
	{
		//int brightness = param.asInt();  // Get slider value.
		//analogWrite(9, brightness);  // Send slider value to real LED on any PWM capable pin on your board
		pirDelay = param.asInt();  // This gets the 'value' of the Widget as an integer 
	}
	
	
	
	BLYNK_WRITE(V0) {  //control LED 
		buttonState = param.asInt();     
	
		if (buttonState == 1 && selectMode == 2){         
				digitalWrite(ledPin, !HIGH);
				}   
	
	
			else if (buttonState == 0 && selectMode == 2){  //on LED only when user choose manual mode
					digitalWrite(ledPin, !LOW);
				}
	} 
	
	BLYNK_WRITE(V7) {  //control LED2 
		int buttonState2 = param.asInt();     
	
		if (buttonState2 == 1 && selectMode == 2){         
				digitalWrite(ledPin2, !HIGH);
				}   
	
	
			else if (buttonState2 == 0 && selectMode == 2){  //on LED only when user choose manual mode
					digitalWrite(ledPin2, !LOW);
				}
	} 
	
	
		
	
	BLYNK_WRITE(V2) {  //control MODE
		switch (selectMode = param.asInt()) {  // Read the button state    
			case 1 : { //alarmMode                   
				timer.disable(timerPIRSensor);
				timer.disable(timerAlarmSensor);
	
				timer.enable(timerPIRSensor);
				timer.enable(timerAlarmSensor);        
				break;
			}
	
			case 2 : {  //manual
				timer.disable(timerPIRSensor);
				timer.disable(timerAlarmSensor);               
				break;
			}
	
			case 3 : { //automatic   
				timer.disable(timerPIRSensor);
				timer.disable(timerAlarmSensor); 
	
				timer.enable(timerPIRSensor);
		
		
				break;
			}                                   
	
			
		} 
	}
	
	
	BLYNK_WRITE(V6) {  //control MODE for timer
		switch (selectMode2 = param.asInt()) {  // Read the button state    
			case 1 : { //bulb       
				
				bothFlag = 1; 

				socketFlag = 0;
				bulbFlag = 0;
				break;
			}
	
			case 2 : {  //socket
				socketFlag = 1;    

				bothFlag = 0;
				bulbFlag = 0;     
				break;
			}
	
			case 3 : {  //both
				bulbFlag = 1;             

				socketFlag = 0;
				bothFlag = 0;
				break;

			}			                         
			
		} 
	}
	
	
	
	
	
	
	void AlarmSensor () {    
		if (selectMode == 1 && PIRValue == 1){
			//intruderState = 1;
			Blynk.notify("intruder detected");
			tone(8, 494, 700);              //beep 
		}
		else if (selectMode == 1 && PIRValue == 0){
			//intruderState = 0;
		}
	}



//always manual control
BLYNK_WRITE(V15) // change color Day
{   
	analogWrite(9, param[0].asInt()); // getting first value
	analogWrite(10, param[1].asInt()); // getting second value
	analogWrite(11, param[2].asInt()); // getting N value  
}
