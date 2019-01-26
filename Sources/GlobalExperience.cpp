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
   \brief LEcture des paramètres EEPROM

   \param --
   \return --
*/
void CGlobale::readEEPROM()
{
    bool state = m_eeprom.getValue("ModeFonctionnement", &Application.ModeFonctionnement);
    _rs232_pc_tx.printf("[%d]:Mode fonctionnement = %d", state, Application.ModeFonctionnement);

   // XBEE n°1
//     m_xbee_settings.APIMODE = '1';
//     strcpy(m_xbee_settings.CHANNEL, "0E");
//     m_xbee_settings.COORDINATOR = '1';
//     m_xbee_settings.COORDINATOR_OPTION = '4';
//     strcpy(m_xbee_settings.PANID, "3321");
//     strcpy(m_xbee_settings.KEY, "6910DEA76FC0328DEBB4307854EDFC42");
//     m_xbee_settings.ID = '1';
//     m_xbee_settings.SECURITY = '1';


}

//___________________________________________________________________________
 /*!
   \brief

   \param --
   \return --
*/
void CGlobale::IRQ_Serial_PC()
{
    char rxData;
   _led2 = !_led2;
   rxData = _rs232_pc_rx.getc();
   _rs232_xbee_network_tx.putc(rxData);
}

/*
void CGlobale::IRQ_Serial_XBEE()
{
    char rxData;
   _led3 = !_led3;
   rxData = _rs232_xbee_network_rx.getc();
   m_xbee.decode(rxData);
   _rs232_pc_tx.putc(rxData);
}
*/
//___________________________________________________________________________
 /*!
   \brief Point d'entrée pour l'execution de toute l'application

   \param --
   \return --
*/
void CGlobale::Run(void)
{
    // Initialise la vitesse de transmission avec le PC
    _rs232_pc_tx.baud(9600);
    _rs232_pc_tx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop
    _rs232_pc_rx.baud(9600);
    _rs232_pc_rx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop

    _rs232_xbee_network_tx.baud(9600);
    _rs232_xbee_network_tx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop
    _rs232_xbee_network_rx.baud(9600);
    _rs232_xbee_network_rx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop

    _rs232_pc_tx.printf("\n\rHello\n\r");

    //_rs232_pc_rx.attach(this, &CGlobale::IRQ_Serial_PC);  	// Callback sur réception d'une donnée sur la RS232

    _rs232_pc_rx.attach(this, &CGlobale::IRQ_Serial_PC);  	// Callback sur réception d'une donnée sur la RS232

  // Attends la montée de toutes les alimentation et l'initialisation de l'écran
  // Temps nécessaire en pratique pour que l'écran tactile ai fini de démarrer
  // avant de commencer à  lui en envoyer des messages (et d'en recevoir) 
  wait_ms(1000);


  // Lecture des paramètres EEPROM et recopie dans les données membres
  // de chaque classe en RAM
  readEEPROM();

  //m_LaBotBox.Start();
  m_messenger_xbee_ntw.start();
  wait(1);

//  m_messenger_xbee_ntw.m_xbee.init(m_xbee_settings);

  //_rs232_xbee_network_rx.puts("+++");
/*
XBEE n°2
    tXbeeSettings xbee_settings;
    xbee_settings.APIMODE = '1';
    strcpy(xbee_settings.CHANNEL, "0E");
    xbee_settings.COORDINATOR = '0'; // -------------
    xbee_settings.COORDINATOR_OPTION = 0x04;
    strcpy(xbee_settings.PANID, "3321");
    strcpy(xbee_settings.KEY, "6910DEA76FC0328DEBB4307854EDFC42");
    xbee_settings.ID = '2';  // ------
    xbee_settings.SECURITY = '1';

 */


  periodicTick.attach(&Application, &CGlobale::IRQ_Tick_ModeAutonome, (float(PERIODE_TICK)/1000.0f));

  while(1) {
      fflush(stdout); // ajout obligatoire ou un wait_us(1) sinon blocage de l'application
      if (Tick) {
          Tick = 0;
          SequenceurModeAutonome();
      }
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
  static bool init_once = 0;

  // ______________________________
  cpt10msec++;
  if (cpt10msec >= TEMPO_10msec) {
  	cpt10msec = 0;

    if (init_once == 0) {
        init_once = 1;

    }
    //m_LaBotBox.Execute();
  }	 

  // ______________________________
  cpt20msec++;
  if (cpt20msec >= TEMPO_20msec) {
	cpt20msec = 0;

    m_leds.setState(LED_4, _Etor_xbee_status);
 }


  // ______________________________
  cpt50msec++;
  if (cpt50msec >= TEMPO_50msec) {
  	cpt50msec = 0;

    // m_messenger_xbee_ntw.execute();
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
    if (m_messenger_xbee_ntw.m_database.m_TimestampMatch.isNewMessage()) {
        _rs232_pc_tx.printf("TimestampMatch message was received");
    }
  }

}

