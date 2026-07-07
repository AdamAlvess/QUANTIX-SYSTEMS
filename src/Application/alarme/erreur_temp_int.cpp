#include "erreur_temp_int.h"
#include "../../Middle_level/composant_interne/led.h"
#include "../../Middle_level/composant_interne/buzzer.h"
#include "histo_logs.h"

// Liaison avec ton API.py (courant, ntc1, ntc2, temp_amb)
extern void mettreAJourBle(float courant, float ntc1, float ntc2, float tempAmb);

void declencherAlarmeTempInt(bool active) {
    if (active) {
        // Matériel local (EXF-25)
        maLedrouge.allumer();
        monBuzzer.sonnerCritique();
        
        // Envoi des données critiques vers l'API BLE (EXF-19 / EXF-25)
        mettreAJourBle(0.0, 76.5, 75.8, 25.0); 
        
        // Sauvegarde dans l'historique (EXF-16 / EXF-25)
        HistoLogs::ajouterLog("ALM_TEMP_INT_ON", 75.0);
    } else {
        maLedrouge.eteindre();
        monBuzzer.eteindre();
        
        mettreAJourBle(0.0, 35.0, 34.5, 25.0);
        HistoLogs::ajouterLog("ALM_TEMP_INT_OFF", 0.0);
    }
}