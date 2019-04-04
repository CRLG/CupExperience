/*! \file Led.cpp
	\brief Classe qui contient toute l'application
*/

#include "RessourcesHardware.h"
#include "led.h"

// ============================================================
//        Gestion d'une LED
// ============================================================
Led::Led()
{
    m_mode = LEDMODE_MANUAL;
}

Led::~Led()
{
}

//___________________________________________________________________________
 /*!
   \brief Fixe l'état d'une LED.
   \return --
   \remark en utilisant cette méthode, la LED passe en mode manuel
*/
void Led::setState(bool state)
{
    setPWM(state==false?0.0:100.0);
}

//___________________________________________________________________________
 /*!
   \brief Change l'état de la LED
   \return --
*/
void Led::toggle()
{
    if (m_current_percent > 50) {
        setState(0.0);
    }
    else {
        setState(100.0);
    }
}

//___________________________________________________________________________
 /*!
   \brief Lit l'état de la LED
   \return l'état On/Off de la LED
*/
float Led::read()
{
    return m_current_percent;
}

//___________________________________________________________________________
 /*!
   \brief Passe la LED en mode impulsion
   \param on_duration : la durée à l'état ON de la LED [msec]
   \param off_duration : la durée à l'état OFF de la LED [msec]
   \param num_cycle : le nombre d'impulsions à jouer (INFINIT = indéfiniement)
   \return --
*/
void Led::setPulseMode(unsigned short on_duration, unsigned short off_duration, unsigned short num_cycle)
{
    m_on_duration = on_duration;
    m_off_duration = off_duration;
    m_num_cycle = num_cycle;

    m_time = 0;
    m_count = 0;

    m_mode = LEDMODE_PULSE;
}

//___________________________________________________________________________
 /*!
   \brief Passe la LED en mode rampe montante/descendante PWM
   \param on_duration : la durée pour atteindre
   \param off_duration : la durée à l'état OFF de la LED [msec]
   \param num_cycle : le nombre d'impulsions à jouer (INFINIT = indéfiniement)
   \return --
*/
void Led::setUpDownMode()
{
    m_mode = LEDMODE_UP_DOWN;
    m_time = 0;
}

//___________________________________________________________________________
 /*!
   \brief Fixe la valeur du PWM en repassant en mode manuel
   \param percent : la valeur du pourcentage
   \return --
*/
void Led::setPWM(signed char percent)
{
    _setPWM(percent);
    m_mode = LEDMODE_MANUAL;
}

//___________________________________________________________________________
 /*!
   \brief Fixe la valeur du PWM sans repasser en mode manuel
   \param percent : la valeur du pourcentage
   \return --
*/
void Led::_setPWM(signed char percent)
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
    m_current_percent = percent;
}

//___________________________________________________________________________
 /*!
   \brief Gestion périodique de la LED
   \return --
   \remark pour gérer l'état de la LED dans les modes PULSE et PATTERN, il est
            nécessaire d'appeler périodiquement cette fonction
*/
void Led::compute()
{
    switch(m_mode)
    {
        // ________________________________________________
        case LEDMODE_PULSE :
            //   |-------------|__________________________|-------------|___________
            //   < on_duration ><       off_duration      >
            if (m_time < m_on_duration) _setPWM(100);
            else _setPWM(0);
            if (m_time >= (m_on_duration + m_off_duration)) {
                if (m_num_cycle != INFINITE) m_count++;
                if (m_count >= m_num_cycle) {
                    setPWM(0);  // repasse en mode manuel LED éteinte
                }
                else {  // Recommence un cycle
                    m_time = 0;
                }
            }
            else {
                m_time += LED_REFRESH_PERIOD;
            }
        break;
        // ________________________________________________
        case LEDMODE_UP_DOWN :

        break;
        // ________________________________________________
        default :
            // ne rien faire
        break;
    }
}
