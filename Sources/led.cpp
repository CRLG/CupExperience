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
    m_mode_old = (tLedMode)(LEDMODE_MANUAL + 1); // Pour être certain que les 2 n'ont pas la même valeur au départ
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
    if (m_current_pwm > 50) {
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
    return m_current_pwm;
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
   \param pwm_min : valeur minimum du PWM (bas de la rampe)
   \param pwm_max : valeur maximum du PWM (haut de la rampe)
   \param speed_up : vitesse d'évolution de la rampe positive en [%]
   \param speed_down : vitesse d'évolution de la rampe négative en [%]
   \return --
*/
void Led::setRampUpDownMode(float pwm_min, float pwm_max, unsigned short speed_up, unsigned short speed_down)
{
    m_mode = LEDMODE_RAMP_UP_DOWN;
    m_ramp_min_pwm = pwm_min;
    m_ramp_max_pwm = pwm_max;
    m_ramp_speed_up = speed_up;
    m_ramp_speed_down = speed_down;
}

//___________________________________________________________________________
 /*!
   \brief Fixe la valeur du PWM en repassant en mode manuel
   \param percent : la valeur du pourcentage
   \return --
*/
void Led::setPWM(float percent)
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
void Led::_setPWM(float percent)
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
    m_current_pwm = percent;
    //_rs232_pc_tx.printf("%f %\n\r", m_current_pwm);
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
    bool entry_mode = (m_mode != m_mode_old);

    switch(m_mode)
    {
        // ________________________________________________
        case LEDMODE_PULSE :
            //   |-------------|__________________________|-------------|___________
            //   < on_duration ><       off_duration      >
            if (entry_mode) {
                m_time = 0;
            }
            if (m_time < m_on_duration) _setPWM(100.);
            else _setPWM(0.);
            if (m_time >= (m_on_duration + m_off_duration)) {
                if (m_num_cycle != INFINITE) m_count++;
                if (m_count >= m_num_cycle) {
                    setPWM(0.);  // repasse en mode manuel LED éteinte
                }
                else {  // Recommence un cycle
                    m_time = 0;
                }
            }
            else {
                m_time += BANDEAU_LED_REFRESH_PERIOD;
            }
        break;
        // ________________________________________________
        case LEDMODE_RAMP_UP_DOWN :
            if (entry_mode) {
                m_current_pwm = m_ramp_min_pwm;
                m_ramp_direction = 1;
            }
            else {
                // Augmentation de la luminosité (rampe positive)
                if (m_ramp_direction >= 0) {
                    m_current_pwm += m_ramp_speed_up;
                    if (m_current_pwm >= m_ramp_max_pwm) {
                        m_current_pwm = m_ramp_max_pwm;
                        m_ramp_direction = -1; // change le sens de la rampe
                    }
                }
                // Diminution de la luminosité (rampe négative)
                else {
                    m_current_pwm -= m_ramp_speed_down;
                    if (m_current_pwm <= m_ramp_min_pwm) {
                        m_current_pwm = m_ramp_min_pwm;
                        m_ramp_direction = 1; // change le sens de la rampe
                    }
                }
            }
            _setPWM(m_current_pwm);
        break;
        // ________________________________________________
        default :
            // ne rien faire
        break;
    }
    // old = new
    m_mode_old = m_mode;
}

