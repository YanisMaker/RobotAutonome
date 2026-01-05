import socket
from pynput import keyboard
import time

# ================= CONFIGURATION =================
# Remplace par l'IP affichée dans le moniteur série de l'ESP32
ESP32_IP = "192.168.1.39" 
ESP32_PORT = 3333

# ================= CONNEXION TCP =================
try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ESP32_IP, ESP32_PORT))
    # On reste en mode bloquant pour plus de fiabilité sur l'envoi
    sock.setblocking(True) 
    print(f"Connecté à l'ESP32 ({ESP32_IP})")
    print("Contrôle : Flèches (Moteurs/Servo), A/Z (Vitesse), Espace (Reset), ESC (Quitter)")
except Exception as e:
    print(f"Impossible de se connecter : {e}")
    exit()

# ================= ÉTAT DES TOUCHES =================
pressed_keys = set()

# ================= ENVOI DES COMMANDES =================
def send_command(cmd):
    try:
        # On ajoute \n car l'ESP32 utilise readStringUntil('\n')
        message = (cmd + "\n").encode()
        sock.sendall(message)
        print(f"Envoyé : {cmd}")
    except Exception as e:
        print(f"Erreur d'envoi : {e}")

# ================= CALLBACKS CLAVIER =================
def on_press(key):
    global pressed_keys

    # Évite les répétitions si on reste appuyé
    if key in pressed_keys:
        return 
    pressed_keys.add(key)

    try:
        # --- DIRECTIONS (Moteurs) ---
        if key == keyboard.Key.up:
            send_command("F")
        elif key == keyboard.Key.down:
            send_command("B")
        
        # --- ANGLE (Servo) ---
        elif key == keyboard.Key.left:
            send_command("L")
        elif key == keyboard.Key.right:
            send_command("R")

        # --- VITESSE (PWM) ---
        elif hasattr(key, 'char'):
            if key.char == 'a':
                send_command("A")
            elif key.char == 'z':
                send_command("Z")

        # --- ACTIONS SPÉCIALES ---
        elif key == keyboard.Key.space:
            send_command("SPACE")
            
    except AttributeError:
        pass

def on_release(key):
    global pressed_keys

    if key in pressed_keys:
        pressed_keys.remove(key)

    # Arrêt immédiat si on relâche la marche avant ou arrière
    if key == keyboard.Key.up or key == keyboard.Key.down:
        send_command("S")

    # Quitter le script
    if key == keyboard.Key.esc:
        return False

# =================LANCEMENT =================
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    try:
        listener.join()
    except KeyboardInterrupt:
        pass
    finally:
        sock.close()
        print("\nConnexion fermée.")