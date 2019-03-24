/*! \file CGlobale.h
	\brief Classe qui contient toute l'application
*/
#ifndef _GLOBALE_H_
#define _GLOBALE_H_

#include "CLeds.h"
#include "CEEPROM.h"
#include "MessengerXbeeNetwork2019.h"
#include "xbeedriver.h"

typedef enum {
	MODE_AUTONOME = 0,
	MODE_PILOTE_LABOTBOX,
	MODE_PILOTE_TERMINAL
}tModeFonctionnement;

// Pour le séquenceur de tâche
#define PERIODE_TICK	(10)
#define TEMPO_10msec	(10/PERIODE_TICK) 
#define TEMPO_20msec	(20/PERIODE_TICK) 
#define TEMPO_50msec	(50/PERIODE_TICK) 
#define TEMPO_100msec	(100/PERIODE_TICK) 
#define TEMPO_200msec	(200/PERIODE_TICK) 
#define TEMPO_500msec	(500/PERIODE_TICK) 
#define TEMPO_1sec		(1000/PERIODE_TICK) 
#define TEMPO_2sec		(2000/PERIODE_TICK) 
#define TEMPO_5sec		(5000/PERIODE_TICK) 
#define TEMPO_10sec		(10000/PERIODE_TICK) 
#define TEMPO_15sec		(15000/PERIODE_TICK) 



// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CGlobale {
public :
    //! Le mode de fonctionnement (autonome ou piloté via Anaconbot)
    unsigned int ModeFonctionnement;

    //! La gestion d'Anaconbot
    //CLaBotBox m_LaBotBox;
    //! Le gestionnaire d'EEPROM
    CEEPROM m_eeprom;
    //! La gestion des Led
    CLeds m_leds;
    //! Network
    MessengerXbeeNetwork m_messenger_xbee_ntw;

    CGlobale();
    ~CGlobale();

    //! Point d'entree pour lancer l'application
    void Run(void);

    //! Lecture des paramètres EEPROM
    void readEEPROM();

    //!  Le programme d'IRQ qui gere le timer
    void IRQ_Tick_ModeAutonome(void);
    void IRQ_Serial_PC();
    //void IRQ_Serial_XBEE();


private : 
    //! Le tick du sequenceur de tache
    unsigned int Tick;

    void SequenceurModeAutonome();


    void stateflowExperience();
    typedef enum {
        EXPERIENCE_INIT = 0,
        EXPERIENCE_WAIT_START_EVENT,
        EXPERIENCE_IN_PROGRESS,
        EXPERIENCE_FINISHED,
        EXPERIENCE_ERROR,
    }tExperienceState;
    unsigned short _experience_state;

    void commandMotor(float percent);
    void commandLight(float percent);

    typedef enum {
        LED_OFF = 0,
        LED_RED,
        LED_GREEN,
        LED_BLUE
    }tLocalColorLed;
    void commandeLocalRGBLED(char color, float intensity);
};


extern CGlobale Application;


#endif 



