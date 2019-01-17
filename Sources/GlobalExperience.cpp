/*! \file CGlobale.cpp
	\brief Classe qui contient toute l'application
*/
#include "mbed.h"
#include "RessourcesHardware.h"
#include "GlobalExperience.h"



//___________________________________________________________________________
 /*!
   \brief Constructeur

   \param --
   \return --
*/
CGlobale::CGlobale() 
{
  ModeFonctionnement = MODE_AUTONOME;
}

//___________________________________________________________________________
 /*!
   \brief Destructeur

   \param --
   \return --
*/
CGlobale::~CGlobale() 
{

}

//___________________________________________________________________________
 /*!
   \brief Point d'entrée pour l'execution de toute l'application

   \param --
   \return --
*/
void CGlobale::Run(void)
{
  // Initialise la vitesse de transmission avec le PC
  _rs232_pc_tx.baud(115200);
  _rs232_pc_tx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop
  _rs232_pc_rx.baud(115200);
  _rs232_pc_rx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop

_led1 = 1;
 _rs232_pc_tx.printf("\n\rHello\n\r");
 
  // Attends la montée de toutes les alimentation et l'initialisation de l'écran
  // Temps nécessaire en pratique pour que l'écran tactile ai fini de démarrer
  // avant de commencer à  lui en envoyer des messages (et d'en recevoir) 
  wait_ms(3000);


 // Lecture des paramètres EEPROM et recopie dans les données membres 
 // de chaque classe en RAM
 m_eeprom.Read();

  _rs232_pc_tx.printf("\n\rCeci est le mode autonome\n\r");
  
   //m_LaBotBox.Start();
   m_messenger_xbee_ntw.start();

   //m_match.Initialise();

   // Initialise une IRQ sur réception RS232 de la caméra
   // Ligne ci-dessous mise en commentaire volontairement tant que la pinoche RX est en l'air (pour éviter d'avoir des IRQ parasites)
   // TODO: à  décommenter dès que la caméra sera branchée
   //_rs232_camera_rx.attach(&Application, &CGlobale::ReceiveRS232_Camera);  	// Callback sur réception d'une donnée sur la RS232
  
   periodicTick.attach(&Application, &CGlobale::IRQ_Tick_ModeAutonome, (float(PERIODE_TICK)/1000.0f));

   while(1) {
       fflush(stdout); // ajout obligatoire ou un wait_us(1) sinon blocage de l'application
       if (Tick) {
           Tick = 0;
           SequenceurModeAutonome();
       }
   }


  while(1) {
  }
}


//___________________________________________________________________________
 /*!
   \brief Fonction d'IRQ du timer

   \param --
   \return --
*/
void CGlobale::IRQ_Tick_ModeAutonome(void)
{
	Tick = 1;
}



//___________________________________________________________________________
 /*!
   \brief Sequenceur de taches en mode autonome

   \param --
   \return --
*/
void CGlobale::SequenceurModeAutonome(void)
{
  static unsigned int cpt10msec = 0;
  static unsigned int cpt20msec = 0;
  static unsigned int cpt50msec = 0;
  static unsigned int cpt100msec = 0;
  static unsigned int cpt200msec = 0;
  static unsigned int cpt500msec = 0;
  static unsigned int cpt1sec = 0;
  static unsigned char once = 0;

  // ______________________________
  cpt10msec++;
  if (cpt10msec >= TEMPO_10msec) {
  	cpt10msec = 0;

    //m_LaBotBox.Execute();
  }	 

  // ______________________________
  cpt20msec++;
  if (cpt20msec >= TEMPO_20msec) {
	cpt20msec = 0;

 }


  // ______________________________
  cpt50msec++;
  if (cpt50msec >= TEMPO_50msec) {
  	cpt50msec = 0;

    m_messenger_xbee_ntw.execute();
    m_leds.compute();
  }

  // ______________________________
  cpt100msec++;
  if (cpt100msec >= TEMPO_100msec) {
  	cpt100msec = 0;
  	
  	if (once == 0) {
		m_leds.toggle(LED_1);
	}
  }

  // ______________________________
  cpt200msec++;
  if (cpt200msec >= TEMPO_200msec) {
  	cpt200msec = 0;

  }
  // ______________________________
  cpt500msec++;
  if (cpt500msec >= TEMPO_500msec) {
  	cpt500msec = 0;
  }
  // ______________________________
  cpt1sec++;
  if (cpt1sec >= TEMPO_1sec) {
  	cpt1sec = 0;

  }

}

