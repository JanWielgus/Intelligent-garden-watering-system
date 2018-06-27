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


const String tempText = "Temp: ";
const String humidText = "Humid: ";
const String Space = " ";



#endif /* CONFIG_H_ */

