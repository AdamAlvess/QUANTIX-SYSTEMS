#include "erreur_com.h"
#include "../../Middle_level/composant_interne/led.h"
#include "../../Middle_level/composant_interne/buzzer.h"
#include "histo_logs.h"

extern void mettreAJourBle(float courant, float ntc1, float ntc2, float tempAmb);

void declencherAlarmeCom(bool active) {
    if (active) {
        // Clignotement local d'avertissement
        maLedrouge.allumer();
        delay(100);
        maLedrouge.eteindre();
        delay(100);
        maLedrouge.allumer();
        monBuzzer.sonnerAvertissement();
        
        // Notification API BLE (Défaut com)
        mettreAJourBle(-1.0, -1.0, -1.0, -1.0);
        HistoLogs::ajouterLog("ALM_COM_ERR_ON", 1.0);
    } else {
        maLedrouge.eteindre();
        monBuzzer.eteindre();
        
        mettreAJourBle(0.0, 35.0, 34.5, 25.0);
        HistoLogs::ajouterLog("ALM_COM_ERR_OFF", 0.0);
    }
}