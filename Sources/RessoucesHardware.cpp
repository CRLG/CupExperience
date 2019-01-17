/*! \file RessoucesHardware.cpp
	\brief Déclare les ressources hardware du MBED utilisées par le reste du logiciel
*/
#include "RessourcesHardware.h"

// __________________________________________________LEDs DU MBED
// Les LED du MBED
DigitalOut _led1(LED1);
DigitalOut _led2(LED2);
DigitalOut _led3(LED3);
DigitalOut _led4(LED4);

// __________________________________________________ENTREES TOUT OU RIEN
DigitalIn  	_Etor1(p5);
DigitalIn  	_Etor2(p6);
DigitalIn  	_Etor3(p7);
DigitalIn  	_Etor4(p8);
DigitalIn  	_Etor5(p11);
DigitalIn  	_Etor6(p12);
DigitalIn  	_Etor_CanRx(p30);	// pins du CAN utilisée en entrée TOR lorsque
DigitalIn  	_Etor_CanTx(p29);	//  l'application n'a pas besoin du bus CAN


// __________________________________________________ENTREES ANALOGIQUES
AnalogIn   _Mes_Vbat(p15);
AnalogIn   _Eana1(p16);
AnalogIn   _Eana2(p17);
AnalogIn   _Eana3(p18);
AnalogIn   _Eana4(p19);
AnalogIn   _Eana5(p20);

// __________________________________________________SORTIES TOUT OU RIEN
//DigitalOut 	_Mot1_Sens1(p22);


// __________________________________________________SORTIES PWM
//PwmOut _Mot1_PWM (p21);
//PwmOut _Mot2_PWM (p24);


// __________________________________________________ TIMERS
Ticker periodicTick;
Timer  _Global_Timer;  // pour connaitre l'heure depuis le démarrage avec résolution usec (attention : débordement au bout de 30minutes)


// __________________________________________________ RS232
Serial _rs232_pc_tx(USBTX, USBRX);
Serial _rs232_pc_rx(USBTX, USBRX);

Serial _rs232_xbee_network_tx(p9, p10);
Serial _rs232_xbee_network_rx(p9, p10);

// __________________________________________________ FICHIERS SUR MBED
LocalFileSystem local("local");





