import asyncio
import struct
from bleak import BleakClient, BleakScanner

# ==========================================
# CONFIGURATION DES UUIDS GATT (A adapter avec l'ESP32)
# ==========================================
# Service Monitoring Temps Réel
SERVICE_MONITORING_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHAR_CURRENT_UUID        = "beb5483e-36e1-4688-b7f5-ea07361b26a8" # Lecture / Notification
CHAR_TEMPERATURES_UUID   = "c2c95358-0001-447a-9cb8-b0bf0a133401" # Lecture / Notification

# Service Configuration / Maintenance
SERVICE_CONFIG_UUID     = "e320d750-2bc4-41d1-8d2b-58d7dc5b49cb"
CHAR_ALARM_THRESHOLDS_UUID = "d195e634-1102-4bf5-bc65-02b80a133402" # Lecture / Ecriture


class AgvMonitoringAPI:
    def __init__(self, target_name="AGV_MONITOR_ESP32"):
        self.target_name = target_name
        self.client = None

    async def connect(self):
        """Scanne les appareils environnants et se connecte à la carte de l'AGV."""
        print(f"🔍 Recherche de l'appareil nommé '{self.target_name}'...")
        devices = await BleakScanner.discover()
        target_device = None
        
        for d in devices:
            if d.name == self.target_name:
                target_device = d
                break
                
        if not target_device:
            print(f"❌ Appareil '{self.target_name}' non trouvé. Vérifiez qu'il est allumé.")
            return False
            
        print(f"🔗 Connexion à {target_device.address}...")
        self.client = BleakClient(target_device.address)
        await self.client.connect()
        print("✅ Connecté avec succès au mode Maintenance de la carte !")
        return True

    async def disconnect(self):
        """Ferme la liaison proprement."""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("🔌 Déconnecté.")

    # ----------------------------------------------------
    # FONCTIONS DE LECTURE (EXF-19)
    # ----------------------------------------------------
    async def read_current(self) -> float:
        """Lit le courant de la batterie 36V transmis par l'INA237."""
        if not self.client.is_connected: return 0.0
        raw_data = await self.client.read_gatt_char(CHAR_CURRENT_UUID)
        # Supposons que l'ESP32 envoie un float (4 octets)
        current = struct.unpack('f', raw_data)[0]
        return current

    async def read_temperatures(self) -> dict:
        """Lit les données thermiques (2 NTC internes + 1 TMP126 ambiante)."""
        if not self.client.is_connected: return {}
        raw_data = await self.client.read_gatt_char(CHAR_TEMPERATURES_UUID)
        # Supposons que l'ESP32 envoie 3 floats : ntc1, ntc2, temp_amb (12 octets)
        ntc1, ntc2, temp_amb = struct.unpack('fff', raw_data)
        return {
            "ntc_pcb_1": ntc1,
            "ntc_pcb_2": ntc2,
            "ambient_chassis": temp_amb
        }

    # ----------------------------------------------------
    # FONCTIONS D'ÉCRITURE (EXF-23)
    # ----------------------------------------------------
    async def update_alarm_thresholds(self, current_max: float, temp_max: float):
        """Permet au technicien d'ajuster les seuils d'alarme depuis sa tablette."""
        if not self.client.is_connected: return
        print(f"💾 Envoi des nouveaux seuils : Courant Max = {current_max}A, Temp Max = {temp_max}°C")
        # Prépare les données au format binaire (2 floats = 8 octets)
        payload = struct.pack('ff', current_max, temp_max)
        await self.client.write_gatt_char(CHAR_ALARM_THRESHOLDS_UUID, payload, response=True)
        print("✅ Nouveaux seuils enregistrés sur la carte.")

    # ----------------------------------------------------
    # GESTION DES NOTIFICATIONS TEMPS RÉEL (Optionnel)
    # ----------------------------------------------------
    def _current_notification_handler(self, sender, data):
        current = struct.unpack('f', data)[0]
        print(f"⚠️ [NOTIF COURANT] Flux temps réel : {current:.2f} A")

    async def start_current_monitoring(self):
        """Active l'envoi automatique de données par l'ESP32 dès que le courant change."""
        await self.client.start_notify(CHAR_CURRENT_UUID, self._current_notification_handler)
        print("🔔 Notifications de courant activées.")


# ==========================================
# EXEMPLE DE SCÉNARIO D'UTILISATION (MAIN)
# ==========================================
async def main():
    print("🤖 MODE SIMULATION LOGICIELLE")
    current_simule = 12.5
    temps_simule = {"ntc_pcb_1": 28.4, "ntc_pcb_2": 29.1, "ambient_chassis": 24.5}
    print(f"Courant (Simulé) : {current_simule} A")
    print(f"Températures (Simulées) : {temps_simule}")
    return # Quitte la fonction pour ne pas lancer la vraie recherche BLE

    api = AgvMonitoringAPI(target_name="AGV_MONITOR_ESP32")
    
    if await api.connect():
        try:
            # 1. Lecture ponctuelle des données (EXF-19)
            print("\n--- 📊 LECTURE INITIALE DES CAPTEURS ---")
            current = await api.read_current()
            temps = await api.read_temperatures()
            print(f"Courant Batterie : {current:.2f} A")
            print(f"Températures PCB : {temps.get('ntc_pcb_1'):.1f}°C / {temps.get('ntc_pcb_2'):.1f}°C")
            print(f"Température Ambiante Châssis (TMP126) : {temps.get('ambient_chassis'):.1f}°C")

            # 2. Modification des seuils réglementaires (EXF-23)
            print("\n--- 🛠️ CONFIGURATION PAR LE TECHNICIEN ---")
            # Le technicien passe par exemple le seuil de courant à 22.5A et température à 75.0°C
            await api.update_alarm_thresholds(current_max=22.5, temp_max=75.0)

            # 3. Écoute du flux de données en continu (Pendant 10 secondes)
            print("\n--- 📉 SURVEILLANCE EN DIRECT ---")
            await api.start_current_monitoring()
            await asyncio.sleep(10) # Laisse tourner pour voir les notifications arriver
            
        finally:
            await api.disconnect()

if __name__ == "__main__":
    asyncio.run(main())