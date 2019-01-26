#include "mbed.h"
#include "RessourcesHardware.h"
#include "GlobalExperience.h"

CGlobale Application;

int main()
{
    // Initialise le gestionnaire de temps
    _Global_Timer.start();

    Application.Run();

	 while(1) {
        // Ce code ne devrait jamais être atteint
        // car la  boucle infinie se trouve dans la méthode CGloble::Run()
         wait_ms(100);
	 }
}
