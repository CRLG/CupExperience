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
    : m_led1(&_led1),
      m_led2(&_led2),
      m_led3(&_led3),
      m_led4(&_led4),
      m_leds_mbed(&m_led1, &m_led2, &m_led3, &m_led4),
      m_bandeau_led(_Stor_BandeauLED)
{
    ModeFonctionnement = MODE_AUTONOME;
    m_duree_pilotage_moteur = DUREE_PILOTAGE_MOTEUR_NOMINAL;
    m_seuil_detection_depart_secours = SEUIL_DETECTION_DEPART_SECOURS_NOMINAL;
    m_temps_confirmation_depart_secours = TEMPS_CONFIRMATION_DEPART_SECOURS_NOMINAL;
    m_cpt_filtrage_telemetre = 0;
    m_pwm_moteur_on = 50.0f;
    m_pwm_min_bandeau_led = 50.0f;
    m_bandeau_led_speed_up = 1;
    m_bandeau_led_speed_down = 3;
    m_trace_debug_active = true;
    m_motif_bandeau_led = MOTIF_RAMPE;
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
    m_eeprom.getValue("TraceDebugActive", &Application.m_trace_debug_active);
    m_eeprom.getValue("ModeFonctionnement", &Application.ModeFonctionnement);
    m_eeprom.getValue("DureePilotageMoteur", &Application.m_duree_pilotage_moteur);
    m_eeprom.getValue("TempsConfirmationDepartSecours", &Application.m_temps_confirmation_depart_secours);
    m_eeprom.getValue("SeuilDetectionDepartSecours", &Application.m_seuil_detection_depart_secours);
    m_eeprom.getValue("PwmMoteurOn", &Application.m_pwm_moteur_on);
    m_eeprom.getValue("PwmMinBandeauLed", &Application.m_pwm_min_bandeau_led);
    m_eeprom.getValue("BandeauLedSpeedUp", &Application.m_bandeau_led_speed_up);
    m_eeprom.getValue("BandeauLedSpeedDown", &Application.m_bandeau_led_speed_down);
    m_eeprom.getValue("MotifBandeauLed", &Application.m_motif_bandeau_led);

    if (m_trace_debug_active) {
        _rs232_pc_tx.printf("- EEPROM -\n\r");
        _rs232_pc_tx.printf("   > TraceDebugActive = %d\n\r", Application.m_trace_debug_active);
        _rs232_pc_tx.printf("   > ModeFonctionnement = %d\n\r", Application.ModeFonctionnement);
        _rs232_pc_tx.printf("   > DureePilotageMoteur = %d\n\r", Application.m_duree_pilotage_moteur);
        _rs232_pc_tx.printf("   > TempsConfirmationDepartSecours = %f\n\r", Application.m_temps_confirmation_depart_secours);
        _rs232_pc_tx.printf("   > SeuilDetectionDepartSecours = %d\n\r", Application.m_seuil_detection_depart_secours);
        _rs232_pc_tx.printf("   > PwmMoteurOn = %f\n\r", Application.m_pwm_moteur_on);
        _rs232_pc_tx.printf("   > PwmMinBandeauLed = %f\n\r", Application.m_pwm_min_bandeau_led);
        _rs232_pc_tx.printf("   > BandeauLedSpeedUp = %f\n\r", Application.m_bandeau_led_speed_up);
        _rs232_pc_tx.printf("   > BandeauLedSpeedDown = %f\n\r", Application.m_bandeau_led_speed_down);
        _rs232_pc_tx.printf("   > MotifBandeauLed = %d\n\r", Application.m_motif_bandeau_led);
    }
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

    _rs232_pc_tx.printf("\n\rCUP EXPERIENCE 2022\n\r");

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

  m_experience_state = EXPERIENCE_INIT;
  m_experience_state_old = m_experience_state + 1;  //+1 pour que les 2 n'aient pas la même valeur

  m_bandeau_led.init();
  m_couleur_chenillard = BLUE;

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

    m_bandeau_led_experience.compute();
    m_leds_mbed.setState(LED_4, _Etor_xbee_status);


    m_bandeau_led.setPattern(1, 10, 30);
    m_bandeau_led.configOnOffColor(1, PURPLE, BLUE);

    m_bandeau_led.setPattern(16, 50, 25);
    m_bandeau_led.configOnOffColor(16, GREEN, OFF_BLACK);
/*
    m_bandeau_led.setPattern(10, 5, 20);
    m_bandeau_led.configOnOffColor(10, RED, OFF_BLACK);

    m_bandeau_led.setPattern(8, 10, 40);
    m_bandeau_led.configOnOffColor(8, OLIVE, OFF_BLACK);

    m_bandeau_led.setPattern(5, 10, 10);
    m_bandeau_led.configOnOffColor(5, TURQUOISE, OFF_BLACK);
*/
//    m_bandeau_led.setColor(1, PURPLE);
//    m_bandeau_led.setColor(2, BLUE);
//    m_bandeau_led.setColor(3, GREEN);
    m_bandeau_led.periodicTask();
  }


  // ______________________________
  cpt50msec++;
  if (cpt50msec >= TEMPO_50msec) {
  	cpt50msec = 0;

    m_messenger_xbee_ntw.execute();
    traitementTelemetre();
    stateflowExperience();
    m_leds_mbed.compute();

    animeChenillardBandeauLED();
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

    if (m_trace_debug_active) {
        _rs232_pc_tx.printf("m_distance_telemetre = %f\n\r", m_distance_telemetre);
        _rs232_pc_tx.printf("m_cpt_filtrage_telemetre = %f\n\r", m_cpt_filtrage_telemetre);
        _rs232_pc_tx.printf("m_ordre_depart_secours = %d\n\r", m_ordre_depart_secours);
        _rs232_pc_tx.printf("m_TimestampMatch.Timestamp = %d\n\r", m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp);
        _rs232_pc_tx.printf("m_cpt_temps_pilotage_moteur = %f\n\r", m_cpt_temps_pilotage_moteur);
        _rs232_pc_tx.printf("\n\r");
    }
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
#define NBRE_LED_CHENILLARD 10
#define PREMIERE_LED_CHENILLARD 5
void CGlobale::animeChenillardBandeauLED()
{
    for (unsigned int i=0; i<NBRE_LED_CHENILLARD; i++)
    {
        int index_led = PREMIERE_LED_CHENILLARD + i;
        m_bandeau_led.setColor(index_led, OFF_BLACK);
    }

    m_bandeau_led.setColor(m_chenillard_state+PREMIERE_LED_CHENILLARD, m_couleur_chenillard);
    m_chenillard_state++;
    if (m_chenillard_state >= NBRE_LED_CHENILLARD) {
        m_chenillard_state = 0;
        switch(m_couleur_chenillard)
        {
        case BLUE : m_couleur_chenillard = GREEN; break;
        case GREEN : m_couleur_chenillard = RED; break;
        default : m_couleur_chenillard = BLUE; break;
        }
    }
}

//___________________________________________________________________________
void CGlobale::stateflowExperience()
{
    bool cligno_led = !m_messenger_xbee_ntw.m_database.m_node_grosbot.isPresent() && !m_messenger_xbee_ntw.m_database.m_node_legobot.isPresent();
    bool entry_state = (m_experience_state != m_experience_state_old);
    m_experience_state_old = m_experience_state;

    switch(m_experience_state)
    {
        // ________________________________________
        case EXPERIENCE_INIT :
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_WAITING_FOR_START;
            m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp = 0;
            m_messenger_xbee_ntw.m_database.m_RobotLegoStatus.Status = Message_ROBOTLEGO_STATUS::ROBOTLEGO_EN_PREPARATION;
            commandMotor(0);
            m_bandeau_led_experience.setState(false);

            commandeLocalRGBLED(LED_PURPLE, 0.01f);
            m_experience_state =EXPERIENCE_WAIT_START_EVENT;
        break;
        // ________________________________________
        case EXPERIENCE_WAIT_START_EVENT :
            commandeLocalRGBLED(LED_PURPLE, 0.5f, cligno_led);
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_WAITING_FOR_START;
            if (    (m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp > 1)
                 || (m_messenger_xbee_ntw.m_database.m_RobotLegoStatus.Status == Message_ROBOTLEGO_STATUS::ROBOTLEGO_MATCH_EN_COURS)
                 || (m_ordre_depart_secours) // Condition de départ de secours
               )
            {
                m_experience_state = EXPERIENCE_IN_PROGRESS;
                m_cpt_temps_pilotage_moteur = 0;
            }
        break;
        // ________________________________________
        case EXPERIENCE_IN_PROGRESS :
            commandeLocalRGBLED(LED_BLUE, 0.5f, cligno_led);
            commandMotor(m_pwm_moteur_on);
            if (entry_state) {
                switch(m_motif_bandeau_led) {
                    case MOTIF_CLIGNOTANT :
                        m_bandeau_led_experience.setPulseMode(500, 500, INFINITE);
                    break;
                    case MOTIF_PULSE :
                        m_bandeau_led_experience.setPulseMode(100, 1000, INFINITE);
                    break;
                    case MOTIF_RAMPE :
                        m_bandeau_led_experience.setRampUpDownMode(m_pwm_min_bandeau_led, 100., m_bandeau_led_speed_up, m_bandeau_led_speed_down);
                    break;
                    default :
                        m_bandeau_led_experience.setState(true);
                    break;
                }
            }
            if (m_messenger_xbee_ntw.m_database.m_TimestampMatch.Timestamp == Message_TIMESTAMP_MATCH::MATCH_END) {
                m_experience_state = EXPERIENCE_FINISHED;
            }
            // arrêt automatique du moteur au bout d'un certain temps
            if (m_cpt_temps_pilotage_moteur >= m_duree_pilotage_moteur) {
                m_experience_state = EXPERIENCE_FINISHED;
            }
            m_messenger_xbee_ntw.m_database.m_ExperienceStatus.ExperienceStatus = Message_EXPERIENCE_STATUS::EXPERIENCE_IN_PROGRESS;
            m_cpt_temps_pilotage_moteur += PERIODE_APPEL_STATEFLOW_EXPERIENCE;
        break;
        // ________________________________________
        case EXPERIENCE_FINISHED :
            commandMotor(0); // Arrête le moteur mais laisse allumé l'affichage visuel
            if (entry_state) {
                m_bandeau_led_experience.setPulseMode(100, 500, INFINITE);
            }
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
            m_experience_state = EXPERIENCE_INIT;
        break;
    }
}


//___________________________________________________________________________
void CGlobale::commandMotor(float percent)
{
    if (percent > 0) {
        _Mot_Sens1 = 0;
        _Mot_Sens2 = 1;
    }
    else if (percent < 0) {
        _Mot_Sens1 = 1;
        _Mot_Sens2 = 0;
    }
    else { // PWM = 0 : pont en H en court-circuit
        _Mot_Sens1 = 1;
        _Mot_Sens2 = 1;
    }
    _Mot_PWM.write(fabs(percent)/100.0);
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
