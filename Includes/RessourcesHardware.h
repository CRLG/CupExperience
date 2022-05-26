/*! \file RessoucesHardware.h
	\brief Déclare les ressources hardware du MBED utilisées par le reste du logiciel
*/
#ifndef _RESSOURCES_HARDWARE_H_ 
#define _RESSOURCES_HARDWARE_H_

#include "mbed.h"


extern DigitalOut _led1;
extern DigitalOut _led2;
extern DigitalOut _led3;
extern DigitalOut _led4;

extern PwmOut _local_rgb_led_R;
extern PwmOut _local_rgb_led_G;
extern PwmOut _local_rgb_led_B;

extern DigitalIn _Etor1;
extern DigitalIn _Etor2;
extern DigitalIn _Etor3;
extern DigitalIn _Etor4;
extern DigitalIn _Etor5;
extern DigitalIn _Etor6;
extern DigitalIn _Etor_CanRx;
extern DigitalIn _Etor_xbee_status;


extern AnalogIn _Mes_Vbat;
extern AnalogIn _Eana1;
//extern AnalogIn _Eana2;
//extern AnalogIn _Eana3;
//extern AnalogIn _Eana4;
//extern AnalogIn _Eana5;

extern Ticker periodicTick;
extern Timer  _Global_Timer;

extern Serial _rs232_pc_tx;
extern Serial _rs232_pc_rx;
extern Serial _rs232_xbee_network_tx;
extern Serial _rs232_xbee_network_rx;
extern LocalFileSystem local;

extern PwmOut _Mot_PWM;
extern PwmOut _LED_PWM;


extern DigitalOut   _Mot_Sens1;
extern DigitalOut   _Mot_Sens2;

extern DigitalOut   _LED_Sens1;
extern DigitalOut   _LED_Sens2;

extern Serial _ax12_serial;

// __________________________________________________SORTIE LED
#define _Stor_BandeauLED_Haut       (p5)
#define _Stor_BandeauLED_Bas        (p7)
#define _Stor_BandeauLED_Tournant   (p6)

//extern DigitalIn  _Estart;
#endif


