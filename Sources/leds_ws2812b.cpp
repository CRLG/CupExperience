#include "leds_ws2812b.h"

LEDS_WS2812::LEDS_WS2812(PinName pin)
    : m_gpio(pin),
      m_nb_leds(NB_MAX_OF_LEDS)
{
}

LEDS_WS2812::~LEDS_WS2812()
{
}

// _______________________________________________
void LEDS_WS2812::init()
{
    int i;
    for (i=0; i<NB_MAX_OF_LEDS; i++) {
        LED_WS2812B[i].Ton          = 0;
        LED_WS2812B[i].Toff         = 0;
        LED_WS2812B[i].Timer        = 0;
        LED_WS2812B[i].ColorOn      = BLUE;
        LED_WS2812B[i].ColorOff     = OFF_BLACK;
    }

    m_nop_T1H = 4;
    periodicTask();
}

// _______________________________________________
void LEDS_WS2812::setNumberOfLeds(int nb)
{
    if (nb<NB_MAX_OF_LEDS) m_nb_leds = nb;
}

// =======================================================
//                      API
// =======================================================
void LEDS_WS2812::setState(unsigned short index, unsigned char state)
{
    if (index >= NB_MAX_OF_LEDS) return;
    
    if (state == 0) {   // Force l'etat OFF
        LED_WS2812B[index].Ton  = 0;
        LED_WS2812B[index].Toff = 1;
    }
    else {              // Force l'etat ON
        LED_WS2812B[index].Ton  = 1;
        LED_WS2812B[index].Toff = 0;
    }
}

// _______________________________________________
void LEDS_WS2812::setColor(unsigned short index, unsigned long rgb)
{
    if (index >= NB_MAX_OF_LEDS) return;
    
    LED_WS2812B[index].ColorOn = rgb;
    setState(index, 1);
}

// _______________________________________________
void LEDS_WS2812::configOnOffColor(unsigned short index, unsigned long on_rgb, unsigned long off_rgb)
{
    if (index >= NB_MAX_OF_LEDS) return;

    LED_WS2812B[index].ColorOn  = on_rgb;
    LED_WS2812B[index].ColorOff = off_rgb;
}

// _______________________________________________
void LEDS_WS2812::setPattern(unsigned short index, unsigned char ton, unsigned char toff)
{
    if (index >= NB_MAX_OF_LEDS) return;
    
    if ( (LED_WS2812B[index].Ton == ton) && (LED_WS2812B[index].Toff== toff) ) {
        // Ne rien faire
        return;
    }
    LED_WS2812B[index].Ton  = ton;
    LED_WS2812B[index].Toff = toff;
    LED_WS2812B[index].Timer = 0;

}

// _______________________________________________
void LEDS_WS2812::setAllState(unsigned char state)
{
    for (int i=0; i<m_nb_leds; i++) {
        setState(i, state);
    }
}

// _______________________________________________
void LEDS_WS2812::setAllColor(unsigned long rgb)
{
    for (int i=0; i<m_nb_leds; i++) {
        setColor(i, rgb);
    }
}

// _______________________________________________
void LEDS_WS2812::configAllOnOffColor(unsigned long on_rgb, unsigned long off_rgb)
{
    for (int i=0; i<m_nb_leds; i++) {
        configOnOffColor(i, on_rgb, off_rgb);
    }
}

// _______________________________________________
void LEDS_WS2812::setAllPattern(unsigned char ton, unsigned char toff)
{
    for (int i=0; i<m_nb_leds; i++) {
        setPattern(i, ton, toff);
    }
}

// _______________________________________________
int LEDS_WS2812::getNbOfLeds()
{
    return m_nb_leds;
}



// _______________________________________________
void LEDS_WS2812::compute_led_state(unsigned short index)
{
    if (LED_WS2812B[index].Ton == 0) {
        LED_WS2812B[index].CurrentState = 0;
        return;
    }
    if (LED_WS2812B[index].Toff == 0) {
        LED_WS2812B[index].CurrentState = 1;
        return;
    }
    
    LED_WS2812B[index].CurrentState = (LED_WS2812B[index].Timer < LED_WS2812B[index].Ton);

    unsigned short period = LED_WS2812B[index].Ton + LED_WS2812B[index].Toff;
    if (++LED_WS2812B[index].Timer > period) {
        LED_WS2812B[index].Timer = 0;
    }
}

// _______________________________________________
void LEDS_WS2812::periodicTask()
{
    unsigned short i;
    for (i=0; i<m_nb_leds; i++) {
        compute_led_state(i);
        unsigned long _color = LED_WS2812B[i].CurrentState==0?LED_WS2812B[i].ColorOff:LED_WS2812B[i].ColorOn;
        // envoie la trame pour la led courante
        send_led_frame(_color);
    }
}

// _______________________________________________
// Genere la trame composee de patterns 1 ou 0
// _______________________________________________
//                 LED0                |                LED1 ...
// [     G;         R;        B]       |[     G;         R;        B]
// [ [G7...G0] [R7....R0] [B7...B0] ]  |[ [G7...G0] [R7....R0] [B7...B0] ]
//     8bits     8bits      8bits      |    8bits     8bits      8bits
void LEDS_WS2812::send_led_frame(unsigned long rgb)
{
    unsigned char r = (rgb>>16)&0xFF;
    unsigned char g = (rgb>> 8)&0xFF;
    unsigned char b = (rgb>> 0)&0xFF;
    int i,j;
    bool buff[BITS_PER_LED];

    // Reconstitue un buffer dans l'ordre attendu par les WS2812
    i=0;
    // Green
    for (j=7; j>=0; j--) {
        buff[i++] = ((g>>j)&0x01);
    }
    // Red
    for (j=7; j>=0; j--) {
        buff[i++] = ((r>>j)&0x01);
    }
    // Blue
    for (j=7; j>=0; j--) {
        buff[i++] = ((b>>j)&0x01);
    }

    // Le transfert est fait en section critique pour assurer les timings
    __disable_irq();
    i=0;
    while (i<BITS_PER_LED) {
        generate_pattern(buff[i++]);
    }
    __enable_irq();
}

// _______________________________________________
void LEDS_WS2812::generate_pattern(bool bit)
{
    if (bit == 0)
    {
        // Genere un "1" sur la sortie pendant T0H = 0.4usec
        // Mesure a 422nsec a l'oscillo sur MBED LPC1768
        set_pin_state(1);
        set_pin_state(0);

        // Puis un "0" pendant T0L = 0.85usec
        // Pas besoin de tempo car en pratique, la sortie de la fonction, et les traitements avant d'y revenir font l'affaire
        set_pin_state(0);
    }

    else
    {
        // Genere un "1" sur la sortie pendant T1H = 0.8usec
        set_pin_state(1);
        for (int i=0; i < m_nop_T1H; i++) {
            __NOP();
        }
        // Puis un "0" pendant T1L = 0.45usec
        set_pin_state(0);
    }
}
// _______________________________________________
void LEDS_WS2812::set_pin_state(bool state)
{
    m_gpio = state;
}

