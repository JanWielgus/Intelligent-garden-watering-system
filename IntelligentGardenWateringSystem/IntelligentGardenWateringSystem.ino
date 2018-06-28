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
// Biblioteki z libManager arduino (po wpisaniu 1307rtc)
#include <TimeLib.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
RCSwitch remote = RCSwitch();

// Functions prototypes
void runMenu(); // Funkcja obslugujaca menu glowne
void readDht();
void printText(const char* text);
void receiveData();
uint32_t getPressedBtn(); // Pobranie informacji ktory przycisk byl klikniety, 0 - nie zostal zaden badz juz odczytano
void runTimeModule(); // Uzywane w loop. Pobiera czas z modulu i zapisuje do zmiennych

// Weather variables
float temperature;
float humidity;
uint16_t rainRaw;

// Variables for remote
uint32_t buttonTime = 0;
unsigned long buttonNo; // kod ostatniego odebranego przycisku
bool isNewPressed = false; // true - odebrano przycisniecie przycisku

// Variables for time
uint8_t tSec, tMin, tHour, tDay, tMon;
uint16_t tYear;

// Global variables
int menuPage = 0; // obecna strona w menu. Opisane na dole pliku
bool wereAChangeFlag = true; // Jesli byla zmiana ktora musi zaistniec w menu to true (trzba ponownie zaladowac menu)
uint32_t lastMenuChangeTime = 0; // Czas ostatniej zmiany w menu


void setup()
{
	lcd.init(); // I2C
	dht.begin(); // One Wire
	remote.enableReceive(0); // Reciever on interrupt 0 -> pin #2
	
	setSyncProvider(RTC.get); // Do obslugi zegara
	
	lcd.backlight();
	printText(String("Hello"));
	delay(500);
}

void loop()
{
	runTimeModule(); // Zaktualizuj czas
	receiveData(); // Odbierz dane z pilota
	
	runMenu(); // Wyswietl menu
	
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


void runMenu()
{
	static String dispRow0; // Wiersz 0 do wyswietlenia
	static String dispRow1; // Wiersz 1 do wyswietlenia
	static uint32_t rBtn; // Wcisniety klawisz
	rBtn = getPressedBtn();
	
	// Ladowanie menu
	if (bool(rBtn)) // Jesli kliknieto jakis klawisz
	{
		
		
		
	}
	else if (wereAChangeFlag) // Jesli wystapila inna zmiana
	{
		
		
		wereAChangeFlag = false; // resetuj flage zadania aktualizacji wyswietlacza
	}
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

void runTimeModule()
{
	static uint32_t lastTimeUpdate=0;
	if ((millis()-lastTimeUpdate) > 1000)
	{
		lastTimeUpdate = millis();
		tSec = second();
		tMin = minute();
		tHour = hour();
		tDay = day();
		tMon = month();
		tYear = year();
		if (menuPage = 10) // Jesli to byla strona stan (gdzie trzba odswierzyc date na ekranie) to zadaj zaktualizowania ekranu
			wereAChangeFlag = true;
	}
}


/*
	lcd.setCursor(col, row) 
	
	Strony w menu (bold to wybrana opcja):
	
		0 - stan czuwania: (wlacza sie po jakims czasie bezczynnosci)
			0: "A-AUTO B-podlew" 1: "C-off  D-menu"       ||lub||       0: "(tryb auto / dzien suchy / dzien mokry / wymuszono podlew / wymuszono brak)"  1: "D-menu"
			
		1 - strona glowna: (B - natychmiastowy stan czuwania)
			0: "STAN ustaw tryby"  1: "info"
			
		2 - stan: (B - powrot do menu)
			0: "XX:XX XX.XX.XXXX"  1: "T:XX H:XX R:XX"  -  data i godzina, temp, humid, rain
			
		3 - ustawienia: (B - powrot do menu)
			- 0: "Ust auto podlew"
			  1: obecna opcja
			  1:  (wybrane): "ON | off"
			
			- 0: "Godzina podlew"
			  1: obecna opcja
			  1: (wybrane):  "|23| 24 3 4"
			  
			- 0: "Czas podlewania"
			  1: obecna opcja
			  1: (wybrane): "5 10 |15| 20 25"
			  
		4 - tryby ?
		
		5 - info (informacje o sterownaiu)
			- 0: "A-ENTER B-CANCEL"
			  1: "C-LEWO  D-PRAWO"
*/
