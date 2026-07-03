#include "erreur_com.h"
#include "../../Middle_level/composant_interne/led.h"
#include "../../Middle_level/composant_interne/buzzer.h"

void declencherAlarmeCom(bool active) {
    if (active) {
        // En cas d'erreur de communication :
        // Clignotement de la LED rouge, et on joue un bip d'avertissement court
        maLedrouge.allumer();
        delay(100);
        maLedrouge.eteindre();
        delay(100);
        maLedrouge.allumer();
        monBuzzer.sonnerAvertissement();
    } else {
        maLedrouge.eteindre();
        monBuzzer.eteindre();
    }
}