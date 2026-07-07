#include "erreur_temp_ext.h"
#include "../../Middle_level/composant_interne/led.h"
#include "../../Middle_level/composant_interne/buzzer.h"
#include "histo_logs.h"

extern void mettreAJourBle(float courant, float ntc1, float ntc2, float tempAmb);

void declencherAlarmeTempExt(bool active) {
    if (active) {
        maLedrouge.allumer();
        monBuzzer.sonnerAvertissement();
        
        mettreAJourBle(0.0, 45.0, 44.2, 40.0);
        HistoLogs::ajouterLog("ALM_TEMP_EXT_ON", 1.0);
    } else {
        maLedrouge.eteindre();
        monBuzzer.eteindre();
        
        mettreAJourBle(0.0, 35.0, 34.5, 25.0);
        HistoLogs::ajouterLog("ALM_TEMP_EXT_OFF", 0.0);
    }
}