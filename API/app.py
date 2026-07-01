from flask import Flask, jsonify, render_template, request
import asyncio
import struct
from bleak import BleakClient, BleakScanner

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('app.html')

# UUIDs définis précédemment
CHAR_CURRENT_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
CHAR_TEMPERATURES_UUID = "c2c95358-0001-447a-9cb8-b0bf0a133401"
TARGET_NAME = "AGV_MONITOR_ESP32"

# Variable globale pour simuler ou stocker l'adresse de l'AGV
agv_client = None

# ----------------------------------------------------
# ROUTES API POUR LES REQUÊTES DU NAVIGATEUR
# ----------------------------------------------------
@app.route('/api/connect')
def connect():
    # Note : Flask est synchrone par défaut, on utilise un pont asyncio pour exécuter Bleak
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    success = loop.run_until_complete(fake_or_real_connect())
    if success:
        return jsonify({"status": "✅ Connecté à l'AGV (MODE SIMULATION TECHNIQUE)"})
    return jsonify({"status": "❌ AGV introuvable"})

@app.route('/api/data')
def get_data():
    # Simulation de données pour tester immédiatement votre interface graphique !
    # Une fois l'ESP32 prêt, vous remplacerez cela par les vraies fonctions read_gatt_char de Bleak
    mock_data = {
        "current": 14.2, 
        "temperatures": {
            "ntc_pcb_1": 32.5,
            "ambient_chassis": 26.1
        }
    }
    return jsonify(mock_data)

async def fake_or_real_connect():
    await asyncio.sleep(1) # Simule le temps de recherche Bluetooth
    return True

if __name__ == '__main__':
    # Lance le serveur web local sur le port 5000
    app.run(debug=True, port=5000)