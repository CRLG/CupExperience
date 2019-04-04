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
    m_duree_pilotage_moteur = DUREE_PILOTAGE_MOTEUR_NOMINAL;
    m_seuil_detection_depart_secours = SEUIL_DETECTION_DEPART_SECOURS_NOMINAL;
    m_temps_confirmation_depart_secours = TEMPS_CONFIRMATION_DEPART_SECOURS_NOMINAL;
    m_cpt_filtrage_telemetre = 0;

    m_cpt_perte_com_xbee_grobot = 0xFFFF;   // Par défaut, pas de communication
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
    m_eeprom.getValue("DureePilotageMoteur", &Application.m_duree_pilotage_moteur);
    m_eeprom.getValue("TempsConfirmationDepartSecours", &Application.m_temps_confirmation_depart_secours);
    m_eeprom.getValue("SeuilDetectionDepartSecours", &Application.m_seuil_detection_depart_secours);

    _rs232_pc_tx.printf("- EEPROM -\n\r");
    _rs232_pc_tx.printf("   > ModeFonctionnement = %d\n\r", Application.ModeFonctionnement);
    _rs232_pc_tx.printf("   > DureePilotageMoteur = %d\n\r", Application.m_duree_pilotage_moteur);
    _rs232_pc_tx.printf("   > TempsConfirmationDepartSecours = %f\n\r", Application.m_temps_confirmation_depart_secours);
    _rs232_pc_tx.printf("   > SeuilDetectionDepartSecours = %d\n\r", Application.m_seuil_detection_depart_secours);
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
    // Initialise la vitesse de transmission avec le PC
    _rs232_pc_tx.baud(9600);
    _rs232_pc_tx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop
    _rs232_pc_rx.baud(9600);
    _rs232_pc_rx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop

    _rs232_xbee_network_tx.baud(9600);
    _rs232_xbee_network_tx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop
    _rs232_xbee_network_rx.baud(9600);
    _rs232_xbee_network_rx.format(8, Serial::None, 1);   	// 8 bits de données / Pas de parité / 1 bit de stop

    _rs232_pc_tx.printf("\n\rEXPERIENCE ELECTRON 2019\n\r");

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

    m_leds_mbed.setState(LED_4, _Etor_xbee_status);
 }


  // ______________________________
  cpt50msec++;
  if (cpt50msec >= TEMPO_50msec) {
  	cpt50msec = 0;

    m_messenger_xbee_ntw.execute();
    traitementTelemetre();
    stateflowExperience();
    m_leds_mbed.compute();
    m_led_experience.compute();
  }

  // ______________________________
  cpt100msec++;
  if (cpt100msec >= TEMPO_100msec) {
  	cpt100msec = 0;
  	
    if (once == 0) {
        m_leds_mbed.toggle(LED_1);
	}
  }

  // ______________________________
  cpt200msec++;
  if (cpt200msec >= TEMPO_200msec) {
  	cpt200msec = 0;

    // Diagnostic de perte de communication avec GROSBOT
    if (m_messenger_xbee_ntw.m_database.m_TimestampMatch.isNewMessage()) {
        //_rs232_pc_tx.printf("TimestampMatch message was received : %d\n\r", m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp);
        m_cpt_perte_com_xbee_grobot = 0;
    }
    else {
        if (m_cpt_perte_com_xbee_grobot < 0xFFFF) m_cpt_perte_com_xbee_grobot++;
    }
    m_xbee_grosbot_present = m_cpt_perte_com_xbee_grobot < 20;
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

    _rs232_pc_tx.printf("m_distance_telemetre = %f\n\r", m_distance_telemetre);
    _rs232_pc_tx.printf("m_cpt_filtrage_telemetre = %f\n\r", m_cpt_filtrage_telemetre);
    _rs232_pc_tx.printf("m_ordre_depart_secours = %d\n\r", m_ordre_depart_secours);
    _rs232_pc_tx.printf("m_xbee_grosbot_present = %d\n\r", m_xbee_grosbot_present);
    _rs232_pc_tx.printf("m_TimestampMatch.Timestamp = %d\n\r", m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp);
    _rs232_pc_tx.printf("m_cpt_temps_pilotage_moteur = %f\n\r", m_cpt_temps_pilotage_moteur);
    _rs232_pc_tx.printf("\n\r");
  }
}


//___________________________________________________________________________
void CGlobale::traitementTelemetre()
{
    m_distance_telemetre = _Eana1.read() * COEF_TELEMETRE_ULTRASON;

    if (m_distance_telemetre < m_seuil_detection_depart_secours) {
        m_cpt_filtrage_telemetre += PERIODE_APPEL_GESTION_TELEMETRE;
        _led2 = 1;  // La LED du MBED représente l'info brute si le seuil de détection télémètre est atteint
    }
    else {
        m_cpt_filtrage_telemetre = 0;
        _led2 = 0;
    }

    if (m_cpt_filtrage_telemetre > m_temps_confirmation_depart_secours) {
        m_ordre_depart_secours = true;
    }
}

//___________________________________________________________________________
void CGlobale::stateflowExperience()
{
    bool cligno_led = !m_xbee_grosbot_present;
    switch(_experience_state)
    {
        // ________________________________________
        case EXPERIENCE_INIT :
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_WAITING_FOR_START;
            m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp = 0;
            commandMotor(0);
            m_led_experience.setState(false);

            commandeLocalRGBLED(LED_PURPLE, 0.01f);
            _experience_state =EXPERIENCE_WAIT_START_EVENT;
        break;
        // ________________________________________
        case EXPERIENCE_WAIT_START_EVENT :
            commandeLocalRGBLED(LED_PURPLE, 0.5f, cligno_led);
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_WAITING_FOR_START;
            if (    (m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp > 1)
                 || (m_ordre_depart_secours) // Condition de départ de secours
               )
            {
                _experience_state = EXPERIENCE_IN_PROGRESS;
                m_cpt_temps_pilotage_moteur = 0;
            }
        break;
        // ________________________________________
        case EXPERIENCE_IN_PROGRESS :
            commandeLocalRGBLED(LED_BLUE, 0.5f, cligno_led);
            commandMotor(50); // TODO : voir pour un paramètre qui inverse le sens suivant la couleur de l'équipe
            m_led_experience.setState(true);
            if (m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp == Message_TIMESTAMP_MATCH::MATCH_END) {
                _experience_state = EXPERIENCE_FINISHED;
            }
            // arrêt automatique du moteur au bout d'un certain temps
            if (m_cpt_temps_pilotage_moteur >= m_duree_pilotage_moteur) {
                _experience_state = EXPERIENCE_FINISHED;
            }
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_IN_PROGRESS;
            m_cpt_temps_pilotage_moteur += PERIODE_APPEL_STATEFLOW_EXPERIENCE;
        break;
        // ________________________________________
        case EXPERIENCE_FINISHED :
            commandMotor(0); // Arrête le moteur mais laisse allumé l'affichage visuel
            m_led_experience.setState(true);
            commandeLocalRGBLED(LED_GREEN, 1.0f, cligno_led);
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
// intensity : 0.0f -> 1.0f
void CGlobale::commandeLocalRGBLED(char color, float intensity, bool blink)
{
    static unsigned char cpt=0;
    static bool light_on = true;

    // Permet de gérer le clignotement
    if (blink) {
        if (cpt++ > 5) {
            light_on= !light_on;
            cpt = 0;
        }
    }
    else {
        cpt = 0;
        light_on = true;
    }

    switch(color)
    {
        case LED_OFF :
            _local_rgb_led_R.write(1.0f);
            _local_rgb_led_G.write(1.0f);
            _local_rgb_led_B.write(1.0f);
        break;

        case LED_RED :
            _local_rgb_led_R.write(1.0f - intensity*light_on);
            _local_rgb_led_G.write(1.0f);
            _local_rgb_led_B.write(1.0f);
        break;

        case LED_GREEN :
            _local_rgb_led_R.write(1.0f);
            _local_rgb_led_G.write(1.0f - intensity*light_on);
            _local_rgb_led_B.write(1.0f);
        break;

        case LED_BLUE :
            _local_rgb_led_R.write(1.0f);
            _local_rgb_led_G.write(1.0f);
            _local_rgb_led_B.write(1.0f - intensity*light_on);
        break;

        case LED_PURPLE :
            _local_rgb_led_R.write(1.0f - intensity*light_on);
            _local_rgb_led_G.write(1.0f);
            _local_rgb_led_B.write(1.0f - intensity*light_on);
        break;
    }
}
