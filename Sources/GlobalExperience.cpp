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
   \brief Lecture des paramètres EEPROM

   \param --
   \return --
*/
void CGlobale::readEEPROM()
{
    m_eeprom.getValue("ModeFonctionnement", &Application.ModeFonctionnement);
    //_rs232_pc_tx.printf("[%d]:Mode fonctionnement = %d", state, Application.ModeFonctionnement);
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

//___________________________________________________________________________
 /*!
   \brief Point d'entrée pour l'execution de toute l'application

   \param --
   \return --
*/
void CGlobale::Run(void)
{
    _Estart.mode(PullUp);

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
  wait_ms(1000);

  _experience_state = EXPERIENCE_INIT;
  m_messenger_xbee_ntw.m_database.m_ExperienceStatus.setTransmitPeriod(1000);

  periodicTick.attach(&Application, &CGlobale::IRQ_Tick_ModeAutonome, (float(PERIODE_TICK)/1000.0f));

  while(1) {
      fflush(stdout); // ajout obligatoire ou un wait_us(1) ou fflush(stdout) sinon blocage de l'application
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

    m_messenger_xbee_ntw.execute();
    stateflowExperience();
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

    // test
    if (m_messenger_xbee_ntw.m_database.m_TimestampMatch.isNewMessage()) {
        _rs232_pc_tx.printf("TimestampMatch message was received : %d\n", m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp);
    }
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
    //m_messenger_xbee_ntw.test_RX();
  }
}


//___________________________________________________________________________
void CGlobale::stateflowExperience()
{
    switch(_experience_state)
    {
        // ________________________________________
        case EXPERIENCE_INIT :
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_WAITING_FOR_START;
            m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp = 0;
            commandMotor(0);
            commandLight(0);

            commandeLocalRGBLED(LED_BLUE, 0.01f);
            _experience_state =EXPERIENCE_WAIT_START_EVENT;
        break;
        // ________________________________________
        case EXPERIENCE_WAIT_START_EVENT :
            commandeLocalRGBLED(LED_BLUE, 0.5f);
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_WAITING_FOR_START;
//            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus++; // pour les tests uniquement
            if (    (m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp > 1)
                 || (_Estart.read() == 0) // Condition appui sur le bouton d'activation manuel
               )
            {
                _experience_state = EXPERIENCE_IN_PROGRESS;
            }
        break;
        // ________________________________________
        case EXPERIENCE_IN_PROGRESS :
            commandeLocalRGBLED(LED_GREEN, 0.5f);
            commandMotor(50); // TODO : voir pour un paramètre qui inverse le sens suivant la couleur de l'équipe
            commandLight(100);
            if (m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp == Message_TIMESTAMP_MATCH::MATCH_END) {
                _experience_state = EXPERIENCE_FINISHED;
            }
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_IN_PROGRESS;
        break;
        // ________________________________________
        case EXPERIENCE_FINISHED :
            commandMotor(0); // Arrête le moteur mais laisse allumé l'affichage visuel
            commandLight(100);
            commandeLocalRGBLED(LED_GREEN, 1.0f);
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_FINISHED;
        break;
        // ________________________________________
        case EXPERIENCE_ERROR :
            commandeLocalRGBLED(LED_RED, 1.0f);
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_ERROR;
        break;
        // ________________________________________
        default :
            _experience_state = EXPERIENCE_INIT;
        break;
    }
}


//___________________________________________________________________________
void CGlobale::commandMotor(float percent)
{
    if (percent > 0) {
        _Mot_Sens1 = 1;
        _Mot_Sens2 = 0;
    }
    else if (percent < 0) {
        _Mot_Sens1 = 0;
        _Mot_Sens2 = 1;
    }
    else { // PWM = 0 : pont en H en court-circuit
        _Mot_Sens1 = 1;
        _Mot_Sens2 = 1;
    }
    _Mot_PWM.write(percent/100.0);
}


//___________________________________________________________________________
void CGlobale::commandLight(float percent)
{
    if (percent > 0) {
        _LED_Sens1 = 0;
        _LED_Sens2 = 1;
    }
    else if (percent < 0) {
        _LED_Sens1 = 1;
        _LED_Sens2 = 0;
    }
    else { // PWM = 0 : pont en H en court-circuit
        _LED_Sens1 = 1;
        _LED_Sens2 = 1;
    }
    _LED_PWM.write(percent/100.0);
}

//___________________________________________________________________________
// intensity : 0.0f -> 1.0f
void CGlobale::commandeLocalRGBLED(char color, float intensity)
{
    switch(color)
    {
        case LED_OFF :
            _local_rgb_led_R.write(1.0f);
            _local_rgb_led_G.write(1.0f);
            _local_rgb_led_B.write(1.0f);
        break;

        case LED_RED :
            _local_rgb_led_R.write(1.0f - intensity);
            _local_rgb_led_G.write(1.0f);
            _local_rgb_led_B.write(1.0f);
        break;

        case LED_GREEN :
            _local_rgb_led_R.write(1.0f);
            _local_rgb_led_G.write(1.0f - intensity);
            _local_rgb_led_B.write(1.0f);
        break;

        case LED_BLUE :
            _local_rgb_led_R.write(1.0f);
            _local_rgb_led_G.write(1.0f);
            _local_rgb_led_B.write(1.0f - intensity);
        break;
    }
}
