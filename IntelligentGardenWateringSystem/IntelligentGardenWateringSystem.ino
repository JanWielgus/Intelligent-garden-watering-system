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
uint8_t workingMode = 0; // 0 - auto, 1 - zawsze podlewa o godzinie, 2 - nigdy nie podlewa


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
	static int8_t chOnPage1 = 1; // Wybrana opcja na stronie 1
	static const uint8_t AmOnPage1 = 4; // Ilosc na stronie 1
	static int8_t chOnPage3 = 1; // Wybrana opcja w ustawieniach
	static const uint8_t AmOnPage3 = 3; // Ilosc opcji w ustawieniach
	
	rBtn = getPressedBtn(); // Pobranie przycisku
	
	// Reakcja na przycisk
	if (bool(rBtn)) // Jesli kliknieto jakis klawisz
	{
		wereAChangeFlag = true;
		switch (menuPage)
		{
			// =======  STAN CZUWANIA  =======
			case 0:
				if (rBtn==BTN1) // auto
					workingMode = modeAuto;
				else if (rBtn==BTN2) // podlew (zawsze o godzinie)
					workingMode = modeAlwOn;
				else if (rBtn==BTN3) // off (nigdy)
					workingMode = modeAlwOff;
				else if (rBtn==BTN4) // menu
					menuPage = 1;
				break;
			
			// =======  STRONA GLOWNA  =======
			case 1:
				if (rBtn==BTN1) // ENTER
					menuPage = chOnPage1+1; // akurat tak jest
				else if (rBtn==BTN2) // CANCEL
					menuPage = 0; // powrot do stand by
				else if (rBtn==BTN3) // LEFT
				{
					chOnPage1--;
					if (chOnPage1<1)
						chOnPage1+=AmOnPage1;
				}
				else if (rBtn==BTN4) // RIGHT
				{
					chOnPage1++;
					if (chOnPage1>AmOnPage1)
						chOnPage1-=AmOnPage1;
				}
				break;
				
			// =======  STAN  =======
			case 2:
				if (rBtn==BTN2) // CANCEL
					menuPage = 1;
				break;
					
			// =======  USTAWIENIA  =======
			case 3:
				if (rBtn==BTN2) // CANCEL
					menuPage = 1;
				else if (rBtn==BTN1) // CANCEL
				{
					// STH CLEVER
					//
					// zmiana ustawien
					//
					//
					// END OF IT
				}
				else if (rBtn==BTN3) // UP
				{
					chOnPage3--;
					if (chOnPage3<1)
						chOnPage3+=AmOnPage3;
				}
				else if (rBtn==BTN4) // DOWN
				{
					chOnPage3++;
					if (chOnPage3>AmOnPage3)
						chOnPage3-=AmOnPage3;
				}
				break;
			
			// =======  TRYBY  =======
			case 4:
				// to samo co dalej
				
			// =======  INFO  =======
			case 5:
				menuPage = 1; // nic tu narazie nie ma wiec powrot do menu
				break;
		}
	}
	
	
	// Rysowanie menu
	if (wereAChangeFlag) // Jesli wystapila zmiana
	{
		dispRow0 = "";
		dispRow1 = "";
		
		switch (menuPage)
		{
			case 0:
				chOnPage1 = 1; // Reset pozycji w menu
				dispRow0 += "A-";
				//workingMode==modeAuto?dispRow0+=sbAuto.toUpperCase():dispRow0+=sbAuto;
				break;
				
			case 1:
				chOnPage3 = 1; // Reset pozycji w ustawieniach
				// dalej...
				break;
		}
		
		
		wereAChangeFlag = false; // resetuj flage zadania aktualizacji wyswietlacza
	}
}

void readDht()
{
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();
	/* Sprawdzanie poprawności: isnan(t) albo isnan(h): true-error */
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
		if (menuPage = 2) // Jesli to byla strona stan (gdzie trzba odswierzyc date na ekranie) to zadaj zaktualizowania ekranu
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
