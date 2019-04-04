/*! \file Leds.h
	\brief Classe qui contient la gestion des LED
*/

#ifndef _LED_H_
#define _LED_H_

#include "RessourcesHardware.h"

// Dans ce module, toutes les durées sont en [msec]
#define LED_REFRESH_PERIOD 50 // msec : période d'appel de la fonction compute depuis le séquenceur

// ============================================================
//        Gestion d'une LED
// ============================================================
//! Classe de gestion d'une LED
class Led
{
    typedef enum {
        LEDMODE_MANUAL = 0,
        LEDMODE_PULSE,
        LEDMODE_UP_DOWN
    }tLedMode;

#define INFINITE (unsigned short)0xFFFFFFFF
public :
    Led();
    ~Led();

    void compute();
    // API
    void setState(bool state);
    void setPWM(signed char percent);
    void _setPWM(signed char percent);
    void toggle();
    float read();
    void setPulseMode(unsigned short on_duration=500, unsigned short off_duration=500, unsigned short num_cycle=INFINITE);
    void setUpDownMode();

private :
    tLedMode m_mode;
    float m_current_percent;
    unsigned short m_on_duration;
    unsigned short m_off_duration;
    unsigned short m_num_cycle;
    unsigned char m_pattern_mask;

    unsigned long m_count;
    unsigned long m_time;
    unsigned char m_index_array;
};

#endif


