/*
	Date: 09.04.2018
	Author: Jan Wielgus
	Intelligent garden watering system
*/

#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C/LiquidCrystal_I2C.h>
#include <RCSwitch.h>
#include "config.h"

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
RCSwitch remote = RCSwitch();

// Functions prototypes
void readDht();
void printText(const char* text);
void receiveData();
uint32_t getPressedBtn(); // Pobranie informacji ktory przycisk byl klikniety, 0 - nie zostal zaden badz juz odczytano

// weather variables
float temperature;
float humidity;
uint16_t rainRaw;

//variables for remote
uint32_t buttonTime = 0;
unsigned long buttonNo; // kod ostatniego odebranego przycisku
bool isNewPressed = false; // true - odebrano przycisniecie przycisku


void setup()
{
	lcd.init(); // I2C
	dht.begin(); // One Wire
	remote.enableReceive(0); // Reciever on interrupt 0 -> pin #2
	
	
	lcd.backlight();
	printText(String("Hello"));
	delay(500);
}

void loop()
{
	receiveData(); // Odbierz dane z pilota
	// Wykonanie reakcji na odebranie
	switch (uint32_t rBtn = getPressedBtn())
	{
		case 0: // nie ma nowego przycisku
			// cos
			break;
			
		case BTN1: // Przycisk A
			// cos
			break;
			
		default:
			// cos
			break; // do usuniecia, opcjonalne
	}
	
	readDht();
	//lcd.clear();
	lcd.setCursor(0,0);
	printText(tempText);
	printText(String(temperature));
	lcd.setCursor(0,1);
	printText(humidText);
	printText(String(humidity));
	delay(1000);
}


void readDht()
{
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();
	/* Sprawdzanie poprawnoœci: isnan(t) albo isnan(h): true-blad */
}

void printText(String text)
{
	for (int i=0; i < text.length(); i++)
		lcd.print(text[i]);
}

void receiveData()
{
	if (remote.available())
	{
		static unsigned long lastBtn;
		static unsigned long newBtn; // lokalna zmienna do porzechowywania odebranego kodu
		lastBtn = newBtn;
		newBtn = remote.getReceivedValue();
		if ((lastBtn == newBtn && (millis()-buttonTime)>MIN_TIME_BTWN_BTN) ||
			(lastBtn != newBtn))
		{
			buttonTime = millis();
			buttonNo = newBtn;
			isNewPressed = true;
		}
		remote.resetAvailable();
	}
}

uint32_t getPressedBtn()
{
	if (isNewPressed)
	{
		isNewPressed = false;
		return buttonNo;
	}
	return 0;
}


/*
	lcd.setCursor(col, row) 
*/
