#include "erreur_temp_int.h"
#include "../../Middle_level/composant_interne/led.h"
#include "../../Middle_level/composant_interne/buzzer.h"

void declencherAlarmeTempInt(bool active) {
    if (active) {
        // En cas d'erreur de température intérieure critique :
        // La LED rouge s'allume et reste allumée
        maLedrouge.allumer();
        // Le buzzer siffle l'alarme critique (stridente)
        monBuzzer.sonnerCritique();
    } else {
        // Rétablir l'état normal
        maLedrouge.eteindre();
        monBuzzer.eteindre();
    }
}