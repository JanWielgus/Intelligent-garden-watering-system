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
uint8_t workingMode = modeAlwOff; // 0 - auto, 1 - zawsze podlewa o godzinie, 2 - nigdy nie podlewa
bool isTimeColon = true; // Czy na stronie stan ma byc wyswietlany dwukropek pomiedzy godz i min


void setup()
{
	lcd.init(); // I2C
	dht.begin(); // One Wire
	remote.enableReceive(0); // Reciever on interrupt 0 -> pin #2
	
	setSyncProvider(RTC.get); // Do obslugi zegara
	
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print("IGWS is starting");
	lcd.setCursor(0,1);
	lcd.print("Please wait");
	delay(700);
	lcd.print(".");
	delay(700);
	lcd.print(".");
	delay(700);
	lcd.print(".");
	delay(600);
	lcd.home();
}

void loop()
{
	runTimeModule(); // Zaktualizuj czas
	receiveData(); // Odbierz dane z pilota
	
	runMenu(); // Wyswietl menu
	
	readDht();
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
			case 0: // satn czuwania
				chOnPage1 = 1; // Reset pozycji w menu
				workingMode==modeAuto?sbAuto.toUpperCase():sbAuto.toLowerCase();
				workingMode==modeAlwOn?sbOn.toUpperCase():sbOn.toLowerCase();
				workingMode==modeAlwOff?sbOff.toUpperCase():sbOff.toLowerCase();
				// Tworzenie linii do wyswietlenia
				dispRow0 += "A-"+sbAuto+"  B-"+sbOn;
				dispRow1 += "C-"+sbOff+"   D-"+sbMenu;
				break;
				
			case 1:
				chOnPage3 = 1; // Reset pozycji w ustawieniach
				chOnPage1==1?menStan.toUpperCase():menStan.toLowerCase();
				chOnPage1==2?menUstaw.toUpperCase():menUstaw.toLowerCase();
				chOnPage1==3?menTryby.toUpperCase():menTryby.toLowerCase();
				chOnPage1==4?menInfo.toUpperCase():menInfo.toLowerCase();
				// Tworzenie linii do wyswietlenia
				dispRow0 += menStan+Space+menUstaw+Space+menTryby;
				dispRow1 += menInfo;
				break;
				
			case 2:
				if (tHour<10) dispRow0+="0";
				dispRow0+=tHour;
				isTimeColon==true?dispRow0+=":":dispRow0+=Space;
				if (tMin<10) dispRow0+="0";
				dispRow0+=tMin;
				dispRow0+=Space;
				dispRow0+=tDay;
				dispRow0+=".";
				dispRow0+=tMon;
				dispRow0+=".";
				dispRow0+=tYear;
				//endl
				dispRow1 += "T:"+String(uint8_t(temperature))+" H:"+String(uint8_t(humidity))+" R:"+String(rainRaw);
				break;
				
			case 3:
				//
				//
				//
				// DO PRZEMYSLEWNIA !!!!!
				//
				//
				break;
				
			case 4:
				dispRow0 += "NIC TU NIE MA";
				dispRow1 += "klik aby wyjsc";
				break;
			
			case 5:
				dispRow0 += "A-ENTER B-CANCEL";
				dispRow1 += "C-LEWO  D-PRAWO";
				break;
		}
		
		// Zeby zawsze bylo 16 znakow (nie pozostalo nic starego)
		while (dispRow0.length() < 16)
			dispRow0 += " ";
		while (dispRow1.length() < 16)
			dispRow1 += " ";
		
		lcd.setCursor(0,0);
		lcd.print(dispRow0);
		lcd.setCursor(0,1);
		lcd.print(dispRow1);
		wereAChangeFlag = false; // resetuj flage zadania aktualizacji wyswietlacza
	}
}

void readDht()
{
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();
	/* Sprawdzanie poprawnoœci: isnan(t) albo isnan(h): true-error */
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
	if ((millis()-lastTimeUpdate) >= 1000)
	{
		lastTimeUpdate = millis();
		tSec = second();
		tMin = minute();
		tHour = hour();
		tDay = day();
		tMon = month();
		tYear = year();
		if (menuPage == 2) // Jesli to byla strona stan (gdzie trzba odswierzyc date na ekranie) to zadaj zaktualizowania ekranu
		{
			wereAChangeFlag = true;
			isTimeColon = !isTimeColon;
		}
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
