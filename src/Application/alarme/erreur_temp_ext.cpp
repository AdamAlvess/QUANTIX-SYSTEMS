#include "erreur_temp_ext.h"
#include "../../Middle_level/composant_interne/led.h"
#include "../../Middle_level/composant_interne/buzzer.h"

void declencherAlarmeTempExt(bool active) {
    if (active) {
        // En cas d'erreur de température extérieure (avertissement) :
        // La LED rouge s'allume temporairement ou clignote, et on joue un son d'avertissement
        maLedrouge.allumer();
        monBuzzer.sonnerAvertissement();
    } else {
        maLedrouge.eteindre();
        monBuzzer.eteindre();
    }
}