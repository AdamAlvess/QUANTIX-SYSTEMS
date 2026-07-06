#include "recup_donnee.h"

TMP126 capteurTemp126;

void setup() {
    Serial.begin(115200);

    TMP126_Erreur err = capteurTemp126.init();
    if (err != TMP126_OK) {
        Serial.printf("Erreur init TMP126 : %s\n", TMP126_ErreurVersTexte(err));
        // -> a faire remonter vers Application/alarme/erreur_temp_ext
    }

    // Seuils issus du CDC (§3.6) : alarme si T PCB > 75°C, hystérésis 5°C
    capteurTemp126.configurerSeuils(-10.0f, 75.0f, 5.0f);
}

void loop() {
    TMP126_Mesure mesure;
    TMP126_Erreur err = capteurTemp126.lireMesure(mesure);

    if (err == TMP126_OK) {
        Serial.printf("T = %.3f C | haute=%d basse=%d pente=%d crc=%d @ %lu ms\n",
                      mesure.temperature_c,
                      mesure.alerte_haute,
                      mesure.alerte_basse,
                      mesure.alerte_pente,
                      mesure.erreur_crc,
                      mesure.horodatage_ms);

        if (mesure.alerte_haute) {
            // -> déclencher LED rouge + buzzer (EXF-25) + log PSRAM + notif BLE
        }
    } else {
        Serial.printf("Erreur lecture TMP126 : %s\n", TMP126_ErreurVersTexte(err));
    }

    delay(1000);
}