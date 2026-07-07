import os
import re
import sys
import glob
import time
import threading

# Séquences d'échappement ANSI pour des couleurs riches dans le terminal
COLOR_RESET = "\033[0m"
COLOR_GREEN = "\033[92m"
COLOR_RED = "\033[91m"
COLOR_YELLOW = "\033[93m"
COLOR_BLUE = "\033[94m"
COLOR_CYAN = "\033[96m"
COLOR_BOLD = "\033[1m"

# Temps de départ pour simuler millis() / 1000
start_time = time.time()

def get_block_content(text, start_pos):
    """ Extrait le contenu d'un bloc entre accolades en comptant proprement les paires de { et } """
    brace_count = 0
    content_chars = []
    for i in range(start_pos, len(text)):
        char = text[i]
        if char == '{':
            brace_count += 1
            if brace_count == 1:
                continue
        elif char == '}':
            brace_count -= 1
            if brace_count == 0:
                return "".join(content_chars), i
        if brace_count > 0:
            content_chars.append(char)
    return "", -1

class DynamicCppEngine:
    """ Analyseur dynamique et interpreteur de code C++ pour la simulation """
    def __init__(self):
        self.root_dir = os.path.dirname(os.path.abspath(__file__))
        self.project_dir = os.path.join(self.root_dir, "projet_esp32_validation")
        
        # Fichier principal
        self.main_path = os.path.join(self.root_dir, "main2.cpp")
        if not os.path.exists(self.main_path):
            self.main_path = os.path.join(self.project_dir, "src", "main.cpp")
            
        # Dossier des alarmes et ihm
        self.alarms_dir = os.path.join(self.project_dir, "src", "Application", "alarme")
        self.ihm_dir = os.path.join(self.project_dir, "src", "Application", "ihm")

        # Seuils par défaut (seront mis à jour par l'analyse C++)
        self.threshold_temp_int = 75.0
        self.threshold_courant = 15.0
        self.threshold_temp_ext = 45.0

        # Variables initiales extraites du C++
        self.init_temp_int = 35.0
        self.init_temp_ext = 25.0
        self.init_courant = 2.0
        self.init_com_error = False
        self.loop_delay_ms = 500  # Par défaut
        
        # Tailles des buffers circulaires
        self.max_mesures = 50
        self.max_alarmes = 20
        self.max_system_logs = 20

        # Dictionnaire des fonctions d'alarmes detectees : { nom_fonction: { "active": [actions], "inactive": [actions] } }
        self.detected_functions = {}
        
        # Liste des regles de decision detectees dans loop()
        self.decision_rules = []

    def parse_all(self):
        print(f"{COLOR_CYAN}[+] Debut de l'analyse dynamique du code C++...{COLOR_RESET}")
        self.detected_functions.clear()
        self.decision_rules.clear()

        # 1. Lire enregistrement.h pour en extraire la taille des buffers PSRAM
        header_path = os.path.join(self.ihm_dir, "enregistrement.h")
        if os.path.exists(header_path):
            with open(header_path, 'r', encoding='utf-8') as f:
                header_content = f.read()
            match_mesures = re.search(r"#define\s+MAX_MESURES\s+(\d+)", header_content)
            if match_mesures:
                self.max_mesures = int(match_mesures.group(1))
            match_alarmes = re.search(r"#define\s+MAX_ALARMES\s+(\d+)", header_content)
            if match_alarmes:
                self.max_alarmes = int(match_alarmes.group(1))
            match_sys = re.search(r"#define\s+MAX_SYSTEM_LOGS\s+(\d+)", header_content)
            if match_sys:
                self.max_system_logs = int(match_sys.group(1))
            print(f"     [Taille Buffer C++ detectee] MAX_MESURES = {self.max_mesures}, MAX_ALARMES = {self.max_alarmes}, MAX_SYSTEM_LOGS = {self.max_system_logs}")

        # 2. Scanner le dossier alarme et lire toutes les fonctions dans tous les fichiers .cpp
        if os.path.exists(self.alarms_dir):
            cpp_files = glob.glob(os.path.join(self.alarms_dir, "*.cpp"))
            print(f"  -> Analyse du dossier alarme ({len(cpp_files)} fichiers C++ trouves)...")
            for filepath in cpp_files:
                self._parse_alarm_file(filepath)
        else:
            print(f"  {COLOR_RED}[!] Dossier alarme introuvable a: {self.alarms_dir}{COLOR_RESET}")

        # 3. Analyser main2.cpp / main.cpp (Variables initiales, seuils, delay et regles de decision)
        if os.path.exists(self.main_path):
            print(f"  -> Analyse du fichier principal: {COLOR_BOLD}{os.path.basename(self.main_path)}{COLOR_RESET}")
            self._parse_main_file()
        else:
            print(f"  {COLOR_YELLOW}[!] Fichier principal introuvable.{COLOR_RESET}")
            
        print(f"{COLOR_GREEN}[+] Analyse dynamique terminee avec succes !{COLOR_RESET}\n")

    def _parse_alarm_file(self, filepath):
        """ Détecte toutes les fonctions dans un fichier d'alarme et extrait leurs actions """
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()

        # Nettoyage des commentaires pour éviter les fausses détections
        content_clean = re.sub(r"//.*", "", content)
        content_clean = re.sub(r"/\*.*?\*/", "", content_clean, flags=re.DOTALL)

        # Regex pour trouver les fonctions : void nom(bool parametre)
        funcs = re.finditer(r"void\s+(\w+)\s*\(\s*bool\s+(\w+)\s*\)\s*\{", content_clean)
        for match in funcs:
            func_name = match.group(1)
            param_name = match.group(2)
            open_brace_idx = match.end() - 1
            
            # Récupérer tout le corps de la fonction C++
            body, _ = get_block_content(content_clean, open_brace_idx)
            if not body:
                continue
                
            actions_active = []
            actions_inactive = []
            
            # Recherche du if (active) ou if (param_name)
            if_match = re.search(r"if\s*\(\s*" + param_name + r"\s*\)\s*\{", body)
            if if_match:
                if_open_brace = if_match.end() - 1
                if_body, _ = get_block_content(body, if_open_brace)
                actions_active = self._extract_actions(if_body)
                
            # Recherche du else
            else_match = re.search(r"else\s*\{", body)
            if else_match:
                else_open_brace = else_match.end() - 1
                else_body, _ = get_block_content(body, else_open_brace)
                actions_inactive = self._extract_actions(else_body)
                
            self.detected_functions[func_name] = {
                "active": actions_active,
                "inactive": actions_inactive,
                "file": os.path.basename(filepath)
            }
            print(f"     [Fonction C++ detectee] {COLOR_BOLD}{func_name}{COLOR_RESET} (dans {os.path.basename(filepath)})")
            print(f"       -> Si active: {COLOR_GREEN}{', '.join(actions_active) if actions_active else 'Rien'}{COLOR_RESET}")
            print(f"       -> Si inactive: {COLOR_YELLOW}{', '.join(actions_inactive) if actions_inactive else 'Rien'}{COLOR_RESET}")

    def _extract_actions(self, block_code):
        """ Extrait les actions matérielles appelées dans un bloc de code C++ """
        actions = []
        if "maLedrouge.allumer" in block_code:
            actions.append("LED_RED_ON")
        if "maLedrouge.eteindre" in block_code:
            actions.append("LED_RED_OFF")
        if "maLedverte.allumer" in block_code:
            actions.append("LED_GREEN_ON")
        if "maLedverte.eteindre" in block_code:
            actions.append("LED_GREEN_OFF")
        if "monBuzzer.sonnerCritique" in block_code:
            actions.append("BUZZER_CRITICAL")
        if "monBuzzer.sonnerAvertissement" in block_code:
            actions.append("BUZZER_WARNING")
        if "monBuzzer.eteindre" in block_code:
            actions.append("BUZZER_OFF")
        if "clignotement" in block_code.lower() or "delay" in block_code:
            actions.append("BLINK_EFFECT")
        return actions

    def _parse_main_file(self):
        """ Extrait les variables initiales et les structures de conditions de loop() """
        with open(self.main_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # 1. Extraction des variables de départ
        match_temp_int = re.search(r"temp_int\s*=\s*([\d\.]+)", content)
        if match_temp_int:
            self.init_temp_int = float(match_temp_int.group(1))
            print(f"     [Var Init C++] Temp. Interieure = {self.init_temp_int} °C")
            
        match_temp_ext = re.search(r"temp_ext\s*=\s*([\d\.]+)", content)
        if match_temp_ext:
            self.init_temp_ext = float(match_temp_ext.group(1))
            print(f"     [Var Init C++] Temp. Exterieure = {self.init_temp_ext} °C")
            
        match_courant = re.search(r"courant\s*=\s*([\d\.]+)", content)
        if match_courant:
            self.init_courant = float(match_courant.group(1))
            print(f"     [Var Init C++] Courant = {self.init_courant} A")
            
        match_com = re.search(r"com_error\s*=\s*(true|false)", content)
        if match_com:
            self.init_com_error = (match_com.group(1) == "true")
            print(f"     [Var Init C++] Erreur Com = {'OUI' if self.init_com_error else 'NON'}")

        # 2. Extraction des seuils de comparaison
        match_thresh_int = re.search(r"temp_int\s*>\s*([\d\.]+)", content)
        if match_thresh_int:
            self.threshold_temp_int = float(match_thresh_int.group(1))
            print(f"     [Seuil C++] Temp. Interieure Critique > {self.threshold_temp_int} °C")
            
        match_thresh_ext = re.search(r"temp_ext\s*>\s*([\d\.]+)", content)
        if match_thresh_ext:
            self.threshold_temp_ext = float(match_thresh_ext.group(1))
            print(f"     [Seuil C++] Temp. Exterieure Avertissement > {self.threshold_temp_ext} °C")
            
        match_thresh_curr = re.search(r"courant\s*>\s*([\d\.]+)", content)
        if match_thresh_curr:
            self.threshold_courant = float(match_thresh_curr.group(1))
            print(f"     [Seuil C++] Courant Critique > {self.threshold_courant} A")

        # Extraction du delay(ms)
        match_delay = re.search(r"delay\s*\(\s*(\d+)\s*\)", content)
        if match_delay:
            self.loop_delay_ms = int(match_delay.group(1))
            print(f"     [Frequence C++] delay() detecte = {self.loop_delay_ms} ms")

        # 3. Extraction de la logique decisionnelle dans loop()
        loop_header_match = re.search(r"void\s+loop\s*\(\s*\)\s*\{", content)
        if loop_header_match:
            open_brace_idx = loop_header_match.end() - 1
            loop_body, _ = get_block_content(content, open_brace_idx)
            
            # Recherche des structures if avec support des accolades imbriquees
            ifs = re.finditer(r"if\s*\(([^)]+)\)\s*\{", loop_body)
            for match in ifs:
                cond = match.group(1)
                open_brace = match.end() - 1
                block, _ = get_block_content(loop_body, open_brace)
                
                func_calls = re.findall(r"(\w+)\s*\(\s*(true|false)\s*\)", block)
                if func_calls:
                    for func_name, state in func_calls:
                        if func_name in self.detected_functions and state == "true":
                            py_cond = cond.replace("||", " or ").replace("&&", " and ").replace("!", " not ")
                            self.decision_rules.append({
                                "c_cond": cond.strip(),
                                "py_cond": py_cond.strip(),
                                "target_function": func_name
                            })
                            print(f"     [Decision C++] Si ({COLOR_YELLOW}{cond.strip()}{COLOR_RESET}) -> Appeler {COLOR_BOLD}{func_name}(true){COLOR_RESET}")


class VirtualLed:
    def __init__(self, name, pin, color):
        self.name = name
        self.pin = pin
        self.color = color
        self.state = False

    def allumer(self):
        if not self.state:
            self.state = True
            print(f"\n{COLOR_BOLD}>>> [CARTE ESP32] {self.color}{self.name}{COLOR_RESET} s'allume (Pin {self.pin}) <<<{COLOR_RESET}")

    def eteindre(self):
        if self.state:
            self.state = False
            print(f"\n{COLOR_BOLD}>>> [CARTE ESP32] {self.color}{self.name}{COLOR_RESET} s'eteint (Pin {self.pin}) <<<{COLOR_RESET}")


class VirtualBuzzer:
    def __init__(self, pin):
        self.pin = pin
        self.state = "SILENCE"

    def sonnerAvertissement(self):
        if self.state != "AVERTISSEMENT":
            self.state = "AVERTISSEMENT"
            print(f"\n{COLOR_BOLD}>>> [CARTE ESP32] {COLOR_YELLOW}Buzzer{COLOR_RESET} (Pin {self.pin}) -> ♪ bip... bip... (Avertissement) ♪ <<<{COLOR_RESET}")

    def sonnerCritique(self):
        if self.state != "CRITIQUE":
            self.state = "CRITIQUE"
            print(f"\n{COLOR_BOLD}>>> [CARTE ESP32] {COLOR_RED}Buzzer{COLOR_RESET} (Pin {self.pin}) -> ⚠ bip-bip-bip (URGENCE !) ⚠ <<<{COLOR_RESET}")

    def eteindre(self):
        if self.state != "SILENCE":
            self.state = "SILENCE"
            print(f"\n{COLOR_BOLD}>>> [CARTE ESP32] {COLOR_YELLOW}Buzzer{COLOR_RESET} (Pin {self.pin}) -> SILENCE <<<{COLOR_RESET}")


# Instances matérielles simulées
maLedverte = VirtualLed("LED Verte", 14, COLOR_GREEN)
maLedrouge = VirtualLed("LED Rouge", 15, COLOR_RED)
monBuzzer = VirtualBuzzer(13)


class BoardSimulator:
    def __init__(self, engine):
        self.engine = engine
        self.temp_int = engine.init_temp_int
        self.temp_ext = engine.init_temp_ext
        self.courant = engine.init_courant
        self.com_error = engine.init_com_error
        
        # --- Buffers Circulaires Virtuels (PSRAM Simulee) ---
        self.mesures_logs = []
        self.alarmes_logs = []
        self.system_logs = []
        
        # Suivi des etats precedents pour journaliser uniquement les changements de seuil
        self.alarme_int_active = False
        self.alarme_ext_active = False
        self.alarme_com_active = False
        self.active_alarms = set()
        
        self.lock = threading.RLock()
        
        # Enregistrement du boot
        self.push_system_log("DEMARRAGE_ESP32")

    def push_system_log(self, evt):
        with self.lock:
            ts = int(time.time() - start_time)
            self.system_logs.append({"ts": ts, "evt": evt})
            if len(self.system_logs) > self.engine.max_system_logs:
                self.system_logs.pop(0) # Rotation : suppression du plus ancien

    def push_alarme_log(self, name, val):
        with self.lock:
            ts = int(time.time() - start_time)
            self.alarmes_logs.append({"ts": ts, "name": name, "val": val})
            if len(self.alarmes_logs) > self.engine.max_alarmes:
                self.alarmes_logs.pop(0)

    def push_mesure_log(self):
        with self.lock:
            ts = int(time.time() - start_time)
            self.mesures_logs.append({"ts": ts, "courant": self.courant, "temp_int": self.temp_int, "temp_ext": self.temp_ext})
            if len(self.mesures_logs) > self.engine.max_mesures:
                self.mesures_logs.pop(0)

    def apply_actions(self, actions):
        """ Exécute les actions matérielles simulées """
        for action in actions:
            if action == "LED_RED_ON":
                maLedrouge.allumer()
            elif action == "LED_RED_OFF":
                maLedrouge.eteindre()
            elif action == "LED_GREEN_ON":
                maLedverte.allumer()
            elif action == "LED_GREEN_OFF":
                maLedverte.eteindre()
            elif action == "BUZZER_CRITICAL":
                monBuzzer.sonnerCritique()
            elif action == "BUZZER_WARNING":
                monBuzzer.sonnerAvertissement()
            elif action == "BUZZER_OFF":
                monBuzzer.eteindre()
            elif action == "BLINK_EFFECT":
                print(f"[LED Rouge] -> CLIGNOTEMENT (Avertissement)")
                maLedrouge.state = True

    def update_alarm_logic(self):
        """ Interprète la logique décisionnelle et gère l'enregistrement d'alarmes sur transition """
        with self.lock:
            context = {
                "temp_int": self.temp_int,
                "temp_ext": self.temp_ext,
                "courant": self.courant,
                "com_error": self.com_error,
                "is_alarm_active": False
            }
            
            triggered_functions = set()
            
            # Évaluer chaque règle de décision lue dans le C++
            for rule in self.engine.decision_rules:
                cond_expr = rule["py_cond"]
                func_name = rule["target_function"]
                
                try:
                    is_true = eval(cond_expr, {}, context)
                    if is_true:
                        triggered_functions.add(func_name)
                        context["is_alarm_active"] = True
                        break
                except Exception:
                    pass

            # --- Logique de transition et de journalisation des alarmes ---
            # 1. Alarme critique
            if "declencherAlarmeTempInt" in triggered_functions:
                if not self.alarme_int_active:
                    self.alarme_int_active = True
                    val_critique = max(self.temp_int, self.courant)
                    # Journalisation parallele a la simulation
                    self.push_alarme_log("TEMP_INT_OU_COURANT_CRITIQUE", val_critique)
            else:
                self.alarme_int_active = False

            # 2. Alarme exterieure
            if "declencherAlarmeTempExt" in triggered_functions:
                if not self.alarme_ext_active:
                    self.alarme_ext_active = True
                    self.push_alarme_log("TEMP_EXT_AVERTISSEMENT", self.temp_ext)
            else:
                self.alarme_ext_active = False

            # 3. Alarme com
            if "declencherAlarmeCom" in triggered_functions:
                if not self.alarme_com_active:
                    self.alarme_com_active = True
                    self.push_alarme_log("ERREUR_COM_AVERTISSEMENT", 1.0)
            else:
                self.alarme_com_active = False

            # Appliquer les états des fonctions d'alarmes détectées
            is_any_alarm_active = len(triggered_functions) > 0
            
            if is_any_alarm_active:
                maLedverte.eteindre()
                # 1. Désactiver les alarmes qui ne sont plus déclenchées
                for func_name in (self.active_alarms - triggered_functions):
                    actions = self.engine.detected_functions[func_name]["inactive"]
                    self.apply_actions(actions)
                
                # 2. Activer les nouvelles alarmes déclenchées
                for func_name in triggered_functions:
                    actions = self.engine.detected_functions[func_name]["active"]
                    self.apply_actions(actions)
            else:
                # Désactiver toutes les anciennes alarmes actives
                for func_name in self.active_alarms:
                    actions = self.engine.detected_functions[func_name]["inactive"]
                    self.apply_actions(actions)
                maLedverte.allumer()
                maLedrouge.eteindre()
                monBuzzer.eteindre()
                
            self.active_alarms = triggered_functions

    def print_status(self):
        with self.lock:
            print(f"\n{COLOR_BOLD}=== STATUT DE LA CARTE SIMULEE EN TEMPS REEL ==={COLOR_RESET}")
            print(f" Temp. Interieure : {self.temp_int} °C  (Seuil Critique : > {self.engine.threshold_temp_int}°C)")
            print(f" Temp. Exterieure : {self.temp_ext} °C  (Seuil Avertissement : > {self.engine.threshold_temp_ext}°C)")
            print(f" Courant Batterie : {self.courant} A   (Seuil Critique : > {self.engine.threshold_courant}A)")
            print(f" Erreur Com       : {'OUI' if self.com_error else 'NON'}")
            print("-" * 48)
            print(f" LED Verte        : {'ALLUMEE (Fonctionnement optimal)' if maLedverte.state else 'ETEINTE'}")
            print(f" LED Rouge        : {'ALLUMEE (Alarme active)' if maLedrouge.state else 'ETEINTE'}")
            print(f" Buzzer           : {monBuzzer.state}")
            print("==================================================")

    def dump_all_logs(self):
        """ Extrait et affiche le contenu des logs simulés en PSRAM """
        with self.lock:
            print(f"\n{COLOR_BOLD}=== DUMP DES LOGS DE LA PSRAM SIMULEE ==={COLOR_RESET}")
            
            print(f"\n--- LOGS SYSTEME ({len(self.system_logs)}/{self.engine.max_system_logs} enregistres) ---")
            for idx, entry in enumerate(self.system_logs):
                print(f"  [{idx+1}] Timestamp: {entry['ts']}s | Evenement: {COLOR_CYAN}{entry['evt']}{COLOR_RESET}")
                
            print(f"\n--- LOGS ALARMES ({len(self.alarmes_logs)}/{self.engine.max_alarmes} enregistres) ---")
            for idx, entry in enumerate(self.alarmes_logs):
                print(f"  [{idx+1}] Timestamp: {entry['ts']}s | Alarme: {COLOR_RED}{entry['name']}{COLOR_RESET} | Valeur declenchement: {entry['val']}")
                
            print(f"\n--- LOGS MESURES PERIODIQUES ({len(self.mesures_logs)}/{self.engine.max_mesures} enregistres) ---")
            for idx, entry in enumerate(self.mesures_logs):
                print(f"  [{idx+1}] Timestamp: {entry['ts']}s | Courant: {entry['courant']}A | Temp Int: {entry['temp_int']}°C | Temp Ext: {entry['temp_ext']}°C")
            print("==========================================")


# Flag global pour contrôler le thread
sim_running = True

def esp32_loop_thread(sim):
    """ Thread simulant l'exécution infinie de l'ESP32 """
    global sim_running
    last_mesure_ts = time.time()
    
    while sim_running:
        sim.update_alarm_logic()
        
        # Enregistrement périodique toutes les 5 secondes
        now = time.time()
        if now - last_mesure_ts >= 5.0:
            sim.push_mesure_log()
            last_mesure_ts = now
            
        time.sleep(sim.engine.loop_delay_ms / 1000.0)


def main():
    global sim_running
    print("=" * 60)
    print(f"   {COLOR_BOLD}ESP32 RTOS SIMULATOR - RUNNING loop() IN BACKGROUND{COLOR_RESET}")
    print("=" * 60)
    
    # 1. Scanner et analyser dynamiquement tout le projet C++ (alarme/ et main2.cpp)
    engine = DynamicCppEngine()
    engine.parse_all()
    
    # 2. Initialiser la simulation
    sim = BoardSimulator(engine)
    sim.print_status()

    # 3. Lancer la boucle ESP32 loop() dans un thread d'arrière-plan en continu
    thread = threading.Thread(target=esp32_loop_thread, args=(sim,))
    thread.daemon = True
    thread.start()
    
    print(f"{COLOR_GREEN}[+] Boucle loop() de l'ESP32 demarree en arriere-plan (Frequence: {engine.loop_delay_ms}ms)...{COLOR_RESET}")

    try:
        while True:
            print("\n" + "-" * 50)
            print(f" {COLOR_BOLD}MENU DES VALEURS DE CAPTEURS (MODIFICATION EN DIRECT){COLOR_RESET}")
            print("-" * 50)
            print(" 1. Simuler Surchauffe exterieure (Seuil C++ + 5°C)")
            print(" 2. Simuler Surchauffe interieure (Seuil C++ + 5°C)")
            print(" 3. Simuler Surintensite batterie (Seuil C++ + 3A)")
            print(" 4. Simuler Defaut de communication (Avertissement com)")
            print(" 5. Rétablir conditions normales (RESET de main2.cpp)")
            print(" 6. Saisir des valeurs de capteurs personnalisees")
            print(" 7. Afficher le statut actuel des LEDs & Buzzer")
            print(" 8. Extraire et afficher tous les logs de la PSRAM (DUMP_LOGS)")
            print(" 9. Reanalyser tout le code C++ (Hot-Reload à chaud)")
            print(" 10. Quitter")
            print("-" * 50)
            
            choix = input("Votre choix (1-10) : ").strip()
            
            if choix == '1':
                val = engine.threshold_temp_ext + 5.0
                print(f"\n--> Force Temp. Ext a {val}°C (Seuil C++: {engine.threshold_temp_ext}°C)")
                with sim.lock:
                    sim.temp_ext = val
            elif choix == '2':
                val = engine.threshold_temp_int + 5.0
                print(f"\n--> Force Temp. Int a {val}°C (Seuil C++: {engine.threshold_temp_int}°C)")
                with sim.lock:
                    sim.temp_int = val
            elif choix == '3':
                val = engine.threshold_courant + 3.0
                print(f"\n--> Force Courant a {val}A (Seuil C++: {engine.threshold_courant}A)")
                with sim.lock:
                    sim.courant = val
            elif choix == '4':
                print(f"\n--> Force Erreur communication a ACTIF")
                with sim.lock:
                    sim.com_error = True
                sim.push_system_log("CONNEXION_TECHNICIEN")
            elif choix == '5':
                print(f"\n--> Chargement des valeurs initiales C++...")
                with sim.lock:
                    sim.temp_int = engine.init_temp_int
                    sim.temp_ext = engine.init_temp_ext
                    sim.courant = engine.init_courant
                    sim.com_error = engine.init_com_error
                sim.push_system_log("RESET_SIMULATION")
            elif choix == '6':
                print("\n--- Saisie de valeurs personnalisees ---")
                try:
                    t_int = input("Temp. Interieure (°C) [Entree pour ignorer] : ")
                    t_ext = input("Temp. Exterieure (°C) [Entree pour ignorer] : ")
                    curr = input("Courant (A) [Entree pour ignorer] : ")
                    com = input("Erreur Com (1=Actif, 0=Inactif) [Entree pour ignorer] : ")
                    
                    with sim.lock:
                        if t_int:
                            sim.temp_int = float(t_int)
                        if t_ext:
                            sim.temp_ext = float(t_ext)
                        if curr:
                            sim.courant = float(curr)
                        if com:
                            sim.com_error = (int(com) == 1)
                except ValueError:
                    print(f"{COLOR_RED}[!] Valeur invalide ignoree.{COLOR_RESET}")
                    continue
            elif choix == '7':
                sim.print_status()
                continue
            elif choix == '8':
                # Simule l'extraction de logs pour le technicien
                sim.push_system_log("TECHNICIEN_DUMP_LOGS")
                sim.dump_all_logs()
                continue
            elif choix == '9':
                print(f"\n--> Re-analyse de tout le projet C++ (Hot-Reload)...")
                engine.parse_all()
                with sim.lock:
                    sim.temp_int = engine.init_temp_int
                    sim.temp_ext = engine.init_temp_ext
                    sim.courant = engine.init_courant
                    sim.com_error = engine.init_com_error
            elif choix == '10':
                print("\nFermeture du simulateur. Au revoir !")
                sim_running = False
                break
            else:
                print("Choix invalide.")
                continue

            # Petit delai pour laisser le temps au thread de fond de traiter la modification
            time.sleep(0.2)
            
    except KeyboardInterrupt:
        print("\nSimulateur interrompu.")
        sim_running = False

if __name__ == '__main__':
    main()
