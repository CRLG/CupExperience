/*! \file CGlobale.cpp
	\brief Classe qui contient toute l'application
*/
#include "mbed.h"
#include "RessourcesHardware.h"
#include "GlobalExperience.h"
#include "mbed_genie.h"

int flag = 0;        //holds the "power status" of the voltmeter. flag = 0 means voltmeter is "off", flag = 1 means the voltmeter is "on".
//Event handler for the 4d Systems display
// Nico : à voir si on a vraiment besoin de cette partie car on ne fait qu'envoyer des infos vers l'écran et on ne traite pas le retour du tactile
void myGenieEventHandler(void)
{
  genieFrame Event;
  genieDequeueEvent(&Event);
  //event report from an object
  if(Event.reportObject.cmd == GENIE_REPORT_EVENT)
  {
    if (Event.reportObject.object == GENIE_OBJ_4DBUTTON)                // If the Reported Message was from a button
    {
        if (Event.reportObject.index == 0)
        {
             //printf("Off Button pressed!\n\r");
             wait(0.2);
             flag=1;
        }
        if (Event.reportObject.index == 1)
        {
             //printf("On Button pressed!\n\r");
             wait(0.2);
             flag=0;
        }
    }
  }
}

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
    m_bandeau_led_haut(_Stor_BandeauLED_Haut),
    m_bandeau_led_bas(_Stor_BandeauLED_Bas),
    m_bandeau_led_tournant(_Stor_BandeauLED_Tournant),
    m_lcd(p5, p7, p6, p8, p11),
    m_scale(A1,A0,64)//(pinData, pinSCK, gain [128|32|64]) (A1,A0,64);
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
    m_ordre_depart_secours = false;
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

  m_bandeau_led_haut.init();
  m_bandeau_led_haut.setNumberOfLeds(34);
  m_bandeau_led_bas.init();
  m_bandeau_led_bas.setNumberOfLeds(40);
  m_bandeau_led_tournant.init();
  m_bandeau_led_tournant.setNumberOfLeds(6);

  m_couleur_chenillard_haut = BLUE;

  periodicTick.attach(&Application, &CGlobale::IRQ_Tick_ModeAutonome, (float(PERIODE_TICK)/1000.0f));

  m_totalNerf = 0.0;
  init_ecran();
  tare_balance();

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

    if (m_experience_state >= EXPERIENCE_IN_PROGRESS) {
        m_bandeau_led_haut.periodicTask();
        m_bandeau_led_bas.periodicTask();
        m_bandeau_led_tournant.periodicTask();
    }
  }


  // ______________________________
  cpt50msec++;
  if (cpt50msec >= TEMPO_50msec) {
  	cpt50msec = 0;

    m_messenger_xbee_ntw.execute();
    //traitementTelemetre();
    stateflowExperience();
    m_leds_mbed.compute();

    animeExperience();
    pese_compte_affiche_balles();
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
        _rs232_pc_tx.printf("m_CommandeExperience.ExperienceCmd = %d\n\r", m_messenger_xbee_ntw.m_database.m_CommandeExperience.ExperienceCmd);
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
#define NBRE_LED_CHENILLARD 16
#define PREMIERE_LED_CHENILLARD 0
void CGlobale::animeChenillardBandeauLED_Bas()
{
    for (unsigned int i=0; i<NBRE_LED_CHENILLARD; i++)
    {
        int index_led = PREMIERE_LED_CHENILLARD + i;
        m_bandeau_led_bas.setColor(index_led, OFF_BLACK);
    }

    m_bandeau_led_bas.setColor(m_chenillard_bas_state+PREMIERE_LED_CHENILLARD, m_couleur_chenillard_bas);
    m_chenillard_bas_state++;
    if (m_chenillard_bas_state >= NBRE_LED_CHENILLARD) {
        m_chenillard_bas_state = 0;
        switch(m_couleur_chenillard_bas)
        {
        case BLUE : m_couleur_chenillard_bas = GREEN; break;
        case GREEN : m_couleur_chenillard_bas = RED; break;
        default : m_couleur_chenillard_bas = BLUE; break;
        }
    }
}

//___________________________________________________________________________
void CGlobale::animeChenillardBandeauLED_Haut()
{
   m_bandeau_led_haut.setColor(m_chenillard_haut_state, OFF_BLACK);

   m_chenillard_haut_state++;

   if (m_chenillard_haut_state>=m_bandeau_led_haut.getNbOfLeds()) {
       switch(m_couleur_chenillard_haut)
       {
       case BLUE : m_couleur_chenillard_haut = GREEN; break;
       case GREEN : m_couleur_chenillard_haut = RED; break;
       default : m_couleur_chenillard_haut = BLUE; break;
       }
       m_chenillard_haut_state = 0;
   }
   m_bandeau_led_haut.setColor(m_chenillard_haut_state, m_couleur_chenillard_haut);
}

//___________________________________________________________________________
void CGlobale::animeExperience()
{

    // Bandeau du bas
    animeChenillardBandeauLED_Bas();

    // Mise en valeur de la vitrine par allumage en blanc de toutes les LED
    for (int i=18; i<m_bandeau_led_bas.getNbOfLeds(); i++)
    {
        m_bandeau_led_bas.setPattern(i, 10, 50);
        m_bandeau_led_bas.configOnOffColor(i, 0x101010, OFF_BLACK);
    }

    // Bandeau du haut
    animeChenillardBandeauLED_Haut();

    // Bandeau tournant
    m_bandeau_led_tournant.setAllPattern(100, 200);
    m_bandeau_led_tournant.configAllOnOffColor(RED, GREEN);

}

//___________________________________________________________________________
void CGlobale::stateflowExperience()
{
    bool cligno_led = !m_messenger_xbee_ntw.m_database.m_node_grosbot.isPresent();
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
            if ( (m_messenger_xbee_ntw.m_database.m_CommandeExperience.ExperienceCmd== Message_COMMANDE_EXPERIENCE::EXPERIENCE_CMD_START)
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
            if (m_messenger_xbee_ntw.m_database.m_CommandeExperience.ExperienceCmd== Message_COMMANDE_EXPERIENCE::EXPERIENCE_CMD_STOP) {
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


//___________________________________________________________________________
void CGlobale::init_ecran()
{
    SetupGenie();
    genieAttachEventHandler(&myGenieEventHandler);
    genieWriteContrast(15); //set screen contrast to full brightness
}


//___________________________________________________________________________
void CGlobale::tare_balance()
{
    //Tare
    wait(1);
    m_scale.tare();
    m_lcd.locate(0,0);
    m_lcd.printf("Tare ok");
    wait(2);
}

//___________________________________________________________________________
void CGlobale::pese_compte_affiche_balles()
{
    m_weight = m_scale.getGram();

    m_totalNerf = - m_weight/6.07;
    m_lcd.cls();
    m_lcd.locate(0,0);
    m_lcd.printf("Il y a %i balle(s)", m_totalNerf);
    wait(0.5);
    m_yo=m_totalNerf;
    genieWriteObject(GENIE_OBJ_LED_DIGITS, 0x00, m_yo);      //write to Leddigits0 the value of totalNerf
}
