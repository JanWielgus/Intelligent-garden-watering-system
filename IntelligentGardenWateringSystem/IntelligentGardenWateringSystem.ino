/*
	Date: 09.04.2018
	Author: Jan Wielgus
	Intelligent garden watering system
*/

#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C/LiquidCrystal_I2C.h>
#include "config.h"

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

void readDht();
void printText(const char* text);

float temperature;
float humidity;


void setup()
{
	lcd.init();
	dht.begin();
	
	
	lcd.backlight();
	printText(String("Hello"));
	delay(500);
}

void loop()
{
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


/*
	lcd.setCursor(col, row) 
*/
