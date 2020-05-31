#include "RessourcesHardware.h"
#include "ConfigSpecifiqueCoupe.h"
#include "GlobalExperience.h"  // pour l'accès au module EEPROM
#include "CServoMoteurAX.h"

CServoMoteurAX::CServoMoteurAX()
{
}

CServoMoteurAX::~CServoMoteurAX()
{
}

// ============================================================
//         IMPLEMENTATION DES METHODES VIRTUELLES PURES
//                  (en lien avec le hardware)
// ============================================================
// ______________________________________________________________
tAxErr CServoMoteurAX::read(unsigned char *buff_data, unsigned char size)
{
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAX::write(unsigned char *buff_data, unsigned char size)
{
    for (unsigned char i=0; i<size; i++) {
        _ax12_serial.putc(buff_data[i]);
    }
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAX::flushSerialInput()
{
    while (_ax12_serial.readable()) _ax12_serial.getc();  // flush serial input
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAX::waitTransmitComplete()
{
    //! warning : LPC_UART2 correspond to p27 / p28 MBED UART pins
    while((LPC_UART2->LSR & 0x00000040) == 0){}  // wait for serial tx fifo empty
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAX::setTxEnable(bool state)
{
    return AX_OK;
}

// ______________________________________________________________
void CServoMoteurAX::delay_us(unsigned long delay)
{
    wait_us(delay);
}

// ============================================================
//           REDIRECTION DES FONCTIONS POUR YAKINDU
//              (Ticket Itemis 2018102237000178)
// Problème :
//   Les méthodes de la classe de base ne sont pas accessibles
//    depuis le modèle Yakindu.
// Solution :
//  Rediriger les fonctions accessibles depuis le modèle vers la
//    classe de base.
// ============================================================
// ____________________________________________________________
bool CServoMoteurAX::isPresent(unsigned char id)
{
    return ServoAXBase::isPresent(id);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::setPosition(unsigned char id, unsigned short position)
{
    return ServoAXBase::setPosition(id, position);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::setSpeed(unsigned char id, unsigned short speed)
{
   return ServoAXBase::setSpeed(id, speed);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::setPositionSpeed(unsigned char id, unsigned short position, unsigned short speed)
{
    return ServoAXBase::setPositionSpeed(id, position, speed);
}

// ____________________________________________________________
unsigned short CServoMoteurAX::getPosition(unsigned char id, tAxErr *err_status)
{
    return ServoAXBase::getPosition(id, err_status);
}

// ____________________________________________________________
bool CServoMoteurAX::isMoving(unsigned char id, tAxErr *err_status)
{
    return ServoAXBase::isMoving(id, err_status);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::changeID(unsigned char old_id, unsigned char new_id)
{
    return ServoAXBase::changeID(old_id, new_id);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::enableTorque(unsigned char id, bool on_off)
{
    return ServoAXBase::enableTorque(id, on_off);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::setMode(unsigned char id, unsigned char mode)
{
    return ServoAXBase::setMode(id, mode);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::setLimitPositionMin(unsigned char id, unsigned short pos)
{
    return ServoAXBase::setLimitPositionMin(id, pos);
}

// ____________________________________________________________
tAxErr CServoMoteurAX::setLimitPositionMax(unsigned char id, unsigned short pos)
{
    return ServoAXBase::setLimitPositionMax(id, pos);
}





// ______________________________________________________________
tAxErr CServoMoteurAX::Init(void)
{
    tAxErr err;
    err = readEEPROM(); // Lit l'EEPROM et configure le module

    setTxEnable(false);
    _ax12_serial.baud(1000000);

    return err;
}

// ______________________________________________________________
tAxErr CServoMoteurAX::readEEPROM(void)
{
    return AX_OK;
}
