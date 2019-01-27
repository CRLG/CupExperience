#include "MessengerXbeeNetwork2019.h"
#include "messagebase.h"
#include "GlobalExperience.h"

MessengerXbeeNetwork::MessengerXbeeNetwork()
{
    init(&m_transporter, &m_database);
    m_database.m_TimestampMatch.setTransmitPeriod(100);
}

MessengerXbeeNetwork::~MessengerXbeeNetwork()
{
}

// ______________________________________________
void MessengerXbeeNetwork::IRQ_ReceiveRS232()
{
    char rxData;
    rxData = _rs232_xbee_network_rx.getc();
    _rs232_pc_tx.putc(rxData);
    _led3 = !_led3;
    m_xbee.decode(rxData);
}

// ______________________________________________
void MessengerXbeeNetwork::readEEPROM()
{
    Application.m_eeprom.getValue("XBEE.PANID", (char*)&m_xbee_settings.PANID[0]);
    Application.m_eeprom.getValue("XBEE.CHANNEL", &m_xbee_settings.CHANNEL[0]);
    Application.m_eeprom.getValue("XBEE.ID", (char*)&m_xbee_settings.ID);
    Application.m_eeprom.getValue("XBEE.APIMODE", (char*)&m_xbee_settings.APIMODE);
    Application.m_eeprom.getValue("XBEE.SECURITY", (char*)&m_xbee_settings.SECURITY);
    Application.m_eeprom.getValue("XBEE.KEY", &m_xbee_settings.KEY[0]);
    Application.m_eeprom.getValue("XBEE.COORDINATOR", (char*)&m_xbee_settings.COORDINATOR);
    Application.m_eeprom.getValue("XBEE.COORDINATOR_OPTION", (char*)&m_xbee_settings.COORDINATOR_OPTION);

    debug_settings();
/*
    XBEE n°1
      m_xbee_settings.APIMODE = '1';
      strcpy(m_xbee_settings.CHANNEL, "0E");
      m_xbee_settings.COORDINATOR = '1';
      m_xbee_settings.COORDINATOR_OPTION = '4';
      strcpy(m_xbee_settings.PANID, "3321");
      strcpy(m_xbee_settings.KEY, "6910DEA76FC0328DEBB4307854EDFC42");
      m_xbee_settings.ID = '1';
      m_xbee_settings.SECURITY = '1';

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
}

// ______________________________________________
void MessengerXbeeNetwork::start()
{
    readEEPROM();

    m_database.restart();
    while (_rs232_xbee_network_rx.readable()) _rs232_xbee_network_rx.getc(); // Nettoie tout octet en attente dans le buffer
    _rs232_xbee_network_rx.attach(this, &MessengerXbeeNetwork::IRQ_ReceiveRS232);  	// Callback sur réception d'une donnée sur la RS232

    m_xbee.init(m_xbee_settings);
}
// ______________________________________________
void MessengerXbeeNetwork::stop()
{
    _rs232_xbee_network_rx.attach(NULL); // Supprime l'IRQ sur réception RS232
}

// ______________________________________________
void MessengerXbeeNetwork::execute()
{
    // Test pour simuler le robot principal
    // RX messages
    if (m_database.m_ExperienceStatus.isNewMessage()) {
        Application.m_leds.toggle(LED_1);  // pour essayer
        // ....
    }
    // TX messages
    int current_time = _Global_Timer.read_ms();
    if (m_database.m_TimestampMatch.isTimeToSend(current_time)) {
        m_database.m_TimestampMatch.Timestamp+= 10;
        m_database.m_TimestampMatch.send();
    }
}

// ===================================================
//              MESSENGER OUTPUT
// ===================================================
// ______________________________________________
void MessengerXbeeNetwork::encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address)
{
    for (int i=0; i< buff_size; i++) {
        m_xbee.encode(buff_data, buff_size, dest_address);
    }
}

// ===================================================
//                  MESSENGER EVENTS
// ===================================================
// ______________________________________________
void MessengerXbeeNetwork::newFrameReceived(tMessengerFrame *frame)
{
    //Application.m_leds.toggle(LED_1);
}

// ______________________________________________
void MessengerXbeeNetwork::newMessageReceived(MessageBase *msg)
{
    //Application.m_leds.toggle(LED_2);
}

// ______________________________________________
void MessengerXbeeNetwork::frameTransmited(tMessengerFrame *frame)
{
}

// ______________________________________________
void MessengerXbeeNetwork::messageTransmited(MessageBase *msg)
{
}

// ______________________________________________
void MessengerXbeeNetwork::dataUpdated(char *name, char *val_str)
{
    //Application.m_leds.toggle(LED_3);
}

// ______________________________________________
void MessengerXbeeNetwork::dataChanged(char *name, char *val_str)
{
    //Application.m_leds.toggle(LED_4);
}


// ===================================================
//                  LOCAL METHODS
// ===================================================
// ______________________________________________
void MessengerXbeeNetwork::test_RX()
{
    // Simulate data reception
    // Message_TIMESTAMP_MATCH : ID 0x0001
    unsigned char data[] = { 'T', 0x00, 0x01, 0x02, 0x00, 0x11, 0x14 };
    for (unsigned int i=0; i<sizeof(data);  i++) {
        decode(data[i]);
    }
}

// ______________________________________________
void MessengerXbeeNetwork::test_TX()
{
    // Send message
    Message_EXPERIENCE_STATUS *msg = &m_database.m_ExperienceStatus;
    msg->ExperienceStatus = 0xABCD;
    m_database.m_ExperienceStatus.setDestinationAddress(98);
    msg->send();
    m_database.m_ExperienceStatus.setDestinationAddress(95);  // send once again to another destination
    msg->send();
}

// ______________________________________________
void MessengerXbeeNetwork::debug_settings()
{
    _rs232_pc_tx.printf("\n\rXBEE SETTINGS\r\n");
    _rs232_pc_tx.printf("  >APIMODE=%c\r\n", m_xbee_settings.APIMODE);
    _rs232_pc_tx.printf("  >XBEE.CHANNEL=%c%c\r\n", m_xbee_settings.CHANNEL[0], m_xbee_settings.CHANNEL[1]);
    _rs232_pc_tx.printf("  >XBEE.COORDINATOR=%c\r\n", m_xbee_settings.COORDINATOR);
    _rs232_pc_tx.printf("  >COORDINATOR_OPTION=%c\r\n", m_xbee_settings.COORDINATOR_OPTION);
    _rs232_pc_tx.printf("  >PANID=%c%c%c%c\r\n", m_xbee_settings.PANID[0], m_xbee_settings.PANID[1], m_xbee_settings.PANID[2], m_xbee_settings.PANID[3]);
    _rs232_pc_tx.printf("  >KEY=");
    for (unsigned int i=0; i<sizeof(m_xbee_settings.KEY); i++) {
        printf("%c", m_xbee_settings.KEY[i]);
    }
    printf("\r\n");
    _rs232_pc_tx.printf("  >ID=%c\r\n", m_xbee_settings.ID);
    _rs232_pc_tx.printf("  >SECURITY=%c\r\n", m_xbee_settings.SECURITY);

}
