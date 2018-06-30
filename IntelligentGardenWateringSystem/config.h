/*
 * config.h
 *
 * Created: 09/04/2018 16:59:35
 *  Author: Jan Wielgus
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#define DHTPIN 4
#define DHTTYPE DHT11
#define LCD_ADDRESS 0x3F

// Buttons received codes
#define BTN1 10420129 // A
#define BTN2 10420130 // B
#define BTN3 10420132 // C
#define BTN4 10420136 // D
#define NO_BUTTON 0 // Wartosc jesli nie kliknieto zadnego przycisku

#define MIN_TIME_BTWN_BTN 350 // Minimalny czas pomiedzy kliknieciami (zeby uniknac powturzonego odebrania)

// Tryby pracy
#define modeAuto 0   // Auto 
#define modeAlwOn 1  // Zawsze podlewa o godzinie
#define modeAlwOff 2 // Nigdy nie podlewa


const String tempText = "Temp: ";
const String humidText = "Humid: ";
const String Space = " ";
// standby labels (sb - stand by)
String sbAuto = "auto";
String sbOn = "on";
String sbOff = "off";
const String sbMenu = "menu";
// labels in menu
String menStan = "stan";
String menUstaw = "ustaw";
String menTryby = "tryby";
String menInfo = "info";
// labels on each page
#define NO_SETTINGS_PAGES 3 // Ilosc stron z ustawieniami
const String pagUstLab[NO_SETTINGS_PAGES] = {"Ust auto podlew", "Godzina podlew", "Czas podlewania"};




#endif /* CONFIG_H_ */

