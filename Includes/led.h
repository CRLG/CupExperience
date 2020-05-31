/*! \file Leds.h
	\brief Classe qui contient la gestion des LED
*/

#ifndef _BANDEAU_LED_H_
#define _BANDEAU_LED_H_

#include "RessourcesHardware.h"

// Dans ce module, toutes les durées sont en [msec]
#define BANDEAU_LED_REFRESH_PERIOD 20 // msec : période d'appel de la fonction compute depuis le séquenceur

// ============================================================
//        Gestion d'une LED
// ============================================================
//! Classe de gestion d'une LED
class Led
{
    typedef enum {
        LEDMODE_MANUAL = 0,
        LEDMODE_PULSE,
        LEDMODE_RAMP_UP_DOWN
    }tLedMode;

#define INFINITE (unsigned short)0xFFFFFFFF
public :
    Led();
    ~Led();

    void compute();
    // API
    void setState(bool state);
    void setPWM(float percent);
    void _setPWM(float percent);
    void toggle();
    float read();
    void setPulseMode(unsigned short on_duration=500, unsigned short off_duration=500, unsigned short num_cycle=INFINITE);
    void setRampUpDownMode(float pwm_min, float pwm_max, unsigned short speed_up, unsigned short speed_down);

private :
    tLedMode m_mode;
    tLedMode m_mode_old;
    float m_current_pwm;
    unsigned short m_on_duration;
    unsigned short m_off_duration;
    unsigned short m_num_cycle;
    unsigned char m_pattern_mask;

    unsigned long m_count;
    unsigned long m_time;
    unsigned char m_index_array;

    signed char m_ramp_direction;   // +1 up / -1=down
    float m_ramp_min_pwm;
    float m_ramp_max_pwm;
    float m_ramp_speed_up;
    float m_ramp_speed_down;
};

#endif


