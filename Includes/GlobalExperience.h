/*! \file CGlobale.h
	\brief Classe qui contient toute l'application
*/
#ifndef _GLOBALE_H_
#define _GLOBALE_H_

#include "CLeds.h"
#include "CLed.h"
#include "CEEPROM.h"
#include "MessengerXbeeNetwork2019.h"
#include "xbeedriver.h"
#include "led.h"

typedef enum {
	MODE_AUTONOME = 0,
	MODE_PILOTE_LABOTBOX,
	MODE_PILOTE_TERMINAL
}tModeFonctionnement;

typedef enum {
    MOTIF_ALLUME_FIXE = 0,
    MOTIF_RAMPE,
    MOTIF_PULSE,
    MOTIF_CLIGNOTANT
}tMotifBandeauLed;

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

#define DUREE_PILOTAGE_MOTEUR_NOMINAL   100 // [secondes]

#define PERIODE_APPEL_STATEFLOW_EXPERIENCE  0.05 // [sec]
#define PERIODE_APPEL_GESTION_LED 0.05 // [sec]
#define PERIODE_APPEL_GESTION_TELEMETRE  0.05 // [sec]

#define SEUIL_DETECTION_DEPART_SECOURS_NOMINAL 40 // [cm]
#define TEMPS_CONFIRMATION_DEPART_SECOURS_NOMINAL 0.5 // [sec]
// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CGlobale {
public :
    //! Le mode de fonctionnement (autonome ou piloté via Anaconbot)
    unsigned int ModeFonctionnement;

    unsigned int m_duree_pilotage_moteur;   // [sec]
    float m_cpt_temps_pilotage_moteur;      // [sec]

    //! La gestion d'Anaconbot
    //CLaBotBox m_LaBotBox;
    //! Le gestionnaire d'EEPROM
    CEEPROM m_eeprom;
    //! Bandeau de LED
    Led m_bandeau_led_experience;
    //! Network
    MessengerXbeeNetwork m_messenger_xbee_ntw;
    //! La gestion des Led MBED
     CLed m_led1;
     CLed m_led2;
     CLed m_led3;
     CLed m_led4;
     CLeds m_leds_mbed;

    //! Départ de secours
    //! (VCC_capteur/512) -> 2.54cm
    //! VCC_capteur = 3.3V sur l'expérience
    //! 2.54/(3/512) = 394.085
    //! DistanceTelemetre [cm] = ConversionAnalogiqueNormalise * COEF_TELEMETRE_ULTRASON
#define COEF_TELEMETRE_ULTRASON (3.3 * 394.085)   // *3.3 pour tenir compte dufait que le résultat de la conversion analogique est normalisé entre 0.0 et 1.0
    float m_distance_telemetre;
    float m_cpt_filtrage_telemetre;
    float m_temps_confirmation_depart_secours;
    unsigned int m_seuil_detection_depart_secours;
    bool m_ordre_depart_secours;
    float m_pwm_moteur_on;
    float m_pwm_min_bandeau_led;
    float m_bandeau_led_speed_up;
    float m_bandeau_led_speed_down;
    unsigned int m_trace_debug_active;
    unsigned int m_motif_bandeau_led;

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
    unsigned short m_experience_state;
    unsigned short m_experience_state_old;

    void commandMotor(float percent);

    typedef enum {
        LED_OFF = 0,
        LED_RED,
        LED_GREEN,
        LED_BLUE,
        LED_PURPLE
    }tLocalColorLed;
    void commandeLocalRGBLED(char color, float intensity, bool blink=false);

    void traitementTelemetre();

};


extern CGlobale Application;


#endif 



