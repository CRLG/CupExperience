#include <stdio.h>
#include "RessourcesHardware.h"
#include "xbeedriver.h"
#include "messengerinterfacebase.h"

XbeeDriver::XbeeDriver() :
    m_messenger_interface(NULL),
    m_trace_debug_active(false)
{
}

XbeeDriver::~XbeeDriver()
{
}


void XbeeDriver::setMessengerInterface(MessengerInterfaceBase *messenger_iface)
{
    m_messenger_interface = messenger_iface;
}

void XbeeDriver::activeDebug(bool on_off)
{
    m_trace_debug_active = on_off;
}

// ________________________________________________________________
void XbeeDriver::readyBytes(unsigned char *buff_data, unsigned char buff_size, unsigned short source_id)
{
    if (m_messenger_interface) m_messenger_interface->decode(buff_data, buff_size, source_id);
}

// ________________________________________________________________
void XbeeDriver::write(unsigned char *buff_data, unsigned char buff_size)
{
    for (unsigned int i=0; i<buff_size; i++)
    {
       _rs232_xbee_network_tx.putc(buff_data[i]);
       if (m_trace_debug_active) {
        _rs232_pc_tx.putc(buff_data[i]);
       }
    }
}

// ________________________________________________________________
void XbeeDriver::delay_us(unsigned long delay)
{
    wait_us(delay);
}
