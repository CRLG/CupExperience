#ifndef _LEDS_WS2812B_H_
#define _LEDS_WS2812B_H_

#include "mbed.h"

class LEDS_WS2812
{
public :
    LEDS_WS2812(PinName pin);
    ~LEDS_WS2812();

    void init();

    void setState(unsigned short index, unsigned char state);
    void setColor(unsigned short index, unsigned long rgb);
    void configOnOffColor(unsigned short index, unsigned long on_rgb, unsigned long off_rgb);
    void setPattern(unsigned short index, unsigned char ton, unsigned char toff);

    void periodicTask();

    // RGB
    #define OFF_BLACK   (0x000000)
    #define RED         (0xFF0000)
    #define GREEN       (0x00FF00)
    #define BLUE        (0x0000FF)
    #define PURPLE      (0x990066)
    #define YELLOW      (0xFF9900)
    #define WHITE       (0xFFFFFF)
    #define CHARTREUSE  (0x7FFF00)
    #define TURQUOISE   (0x40E0D0)
    #define OLIVE       (0x808000)

private :
    void compute_led_state(unsigned short index);
    void set_pin_state(bool state);
    void send_led_frame(unsigned long rgb);
    void generate_pattern(bool bit);

    typedef struct
    {
        unsigned char Toff;             // Durée à l'état OFF
        unsigned char Ton;              // Durée à l'état ON
        unsigned long ColorOff;         // Couleur de l'état OFF
        unsigned long ColorOn;          // Couleur de l'état ON
        unsigned char Timer;            // Compteur de temps
        unsigned char CurrentState;     // Mémorise l'état courant OFF ou ON
    }tWS2812BPattern;

    int m_nop_T1H;

    DigitalOut m_gpio;

    #define NB_OF_LEDS   48     // 48 LED à contrôler
    #define BITS_PER_LED 24     // 1 LED = 24 bits
    tWS2812BPattern LED_WS2812B[NB_OF_LEDS];

};

#endif	/* _LEDS_WS2812B_H_ */
