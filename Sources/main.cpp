#include "mbed.h"
#include "RessourcesHardware.h"
#include "GlobalExperience.h"

CGlobale Application;

void IRQ_Serial_PC()
{
    char rxData;
   _led2 = !_led2;
   rxData = _rs232_pc_rx.getc();
   _rs232_xbee_network_tx.putc(rxData);
}

void IRQ_Serial_XBEE()
{
    char rxData;
   _led3 = !_led3;
   rxData = _rs232_xbee_network_rx.getc();
   _rs232_pc_tx.putc(rxData);
}

// _____________________________________________________________
int main() {
	 
    _led1 = true;
	wait_ms(1);

    //_rs232_pc_rx.attach(&IRQ_Serial_PC);  	// Callback sur réception d'une donnée sur la RS232
    //_rs232_xbee_network_rx.attach(&IRQ_Serial_XBEE);  	// Callback sur réception d'une donnée sur la RS232

    // Initialise le gestionnaire de temps
    _Global_Timer.start();

    Application.Run();


	 while(1) {
        _led1 = !_led1;
        // Ce code ne devrait jamais être atteint
		// car la  boucle infinie se trouve dans la méthode CGloble::Run()
         wait_ms(100);
	 }
}







