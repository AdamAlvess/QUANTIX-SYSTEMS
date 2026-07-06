from flask import Flask, jsonify, render_template
import asyncio
from API import AgvMonitoringAPI

app = Flask(__name__)

# Instanciation de l'API de votre collègue
agv_api = AgvMonitoringAPI(target_name="AGV_MONITOR_ESP32")

@app.route('/')
def home():
    return render_template('app.html')

@app.route('/api/connect')
def connect():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    success = loop.run_until_complete(agv_api.connect())
    if success:
        return jsonify({"status": "✅ Connecté à l'AGV (Données réelles actives)"})
    return jsonify({"status": "❌ AGV introuvable"})

@app.route('/api/data')
def get_data():
    if not agv_api.client or not agv_api.client.is_connected:
        return jsonify({"error": "Veuillez d'abord initialiser la connexion Bluetooth."}), 400

    try:
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)

        # Récupération de toutes les données réelles
        current = loop.run_until_complete(agv_api.read_current())
        temperatures = loop.run_until_complete(agv_api.read_temperatures())
        mode_code = loop.run_until_complete(agv_api.read_mode())

        # Traduction du code numérique en texte
        if mode_code == 0:
            mode_text = "Fonctionnement nominal"
        elif mode_code == 2:
            mode_text = "Alarme"
        else:
            mode_text = "Maintenance"

        # Construction du JSON final
        real_data = {
            "current": current,
            "temperatures": {
                "ntc_pcb_1": round(temperatures.get("ntc_pcb_1", 0), 1),
                "ambient_chassis": round(temperatures.get("ambient_chassis", 0), 1)
            },
            "mode": mode_text
        }
        return jsonify(real_data)
        
    except Exception as e:
        return jsonify({"error": f"Erreur BLE : {str(e)}"}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5000)