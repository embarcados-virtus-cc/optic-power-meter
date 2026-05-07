import time
import socket
import subprocess
import netifaces
import threading
import os
import re
import json
import shutil
import psutil
from PIL import Image, ImageDraw, ImageFont
import Adafruit_GPIO as GPIO
import Adafruit_GPIO.SPI as SPI
import lgpio
from ST7789 import ST7789

# Tenta importar evdev para o teclado USB
try:
    from evdev import InputDevice, list_devices, ecodes, categorize
except ImportError:
    print("Erro: evdev não encontrado. Instale com: pip install evdev")

# Configuração dos Pinos
DC_PIN = 25
RST_PIN = 27
BLK_PIN = 24
SPI_PORT = 0
SPI_DEVICE = 0
SPI_SPEED_HZ = 40000000

# Cores do Sistema
COLOR_BG = (30, 30, 46)      # Dark Blue
COLOR_TEXT = (255, 255, 255) # White
COLOR_HIGHLIGHT = (6, 182, 212) # Cyan
COLOR_ACCENT = (148, 163, 184) # Gray-Blue
COLOR_ERROR = (248, 113, 113) # Red

class LGPIOAdapter:
    """Adapta a interface do lgpio para o formato esperado pela biblioteca Adafruit_GPIO/ST7789"""
    def __init__(self):
        self._chip = lgpio.gpiochip_open(0)
        
    def setup(self, pin, mode):
        if mode == GPIO.OUT:
            lgpio.gpio_claim_output(self._chip, pin)
        else:
            lgpio.gpio_claim_input(self._chip, pin)
            
    def output(self, pin, value):
        lgpio.gpio_write(self._chip, pin, 1 if value else 0)
        
    def set_high(self, pin):
        lgpio.gpio_write(self._chip, pin, 1)
        
    def set_low(self, pin):
        lgpio.gpio_write(self._chip, pin, 0)
        
    def cleanup(self):
        try:
            lgpio.gpiochip_close(self._chip)
        except:
            pass

class KeyboardHandler:
    """Captura teclas de TODOS os dispositivos USB possíveis para garantir compatibilidade."""
    def __init__(self):
        self.devices = []
        self.queue = []
        self.lock = threading.Lock()
        self._stop = False
        self.shift_pressed = False
        self.find_keyboards()
        if self.devices:
            for device in self.devices:
                t = threading.Thread(target=self._run, args=(device,), daemon=True)
                t.start()

    def find_keyboards(self):
        try:
            all_devices = [InputDevice(path) for path in list_devices()]
            print("Dispositivos de entrada detectados:")
            for device in all_devices:
                print(f" - {device.path}: {device.name}")
                low_name = device.name.lower()
                if any(x in low_name for x in ["keyboard", "logitech", "usb", "hid"]):
                    if "pwr_button" not in low_name:
                        print(f" MONITORANDO: {device.name} em {device.path}")
                        self.devices.append(device)
        except Exception as e:
            print(f"Erro ao listar dispositivos: {e}")

    def _run(self, device):
        try:
            for event in device.read_loop():
                if self._stop: break
                if event.type == ecodes.EV_KEY:
                    key_event = categorize(event)
                    # Monitora Shift (42 = Left Shift, 54 = Right Shift)
                    if key_event.scancode in [42, 54]:
                        self.shift_pressed = (key_event.keystate in [key_event.key_down, key_event.key_hold])
                    
                    if key_event.keystate in [key_event.key_down, key_event.key_hold]:
                        with self.lock:
                            # Adiciona tupla (scancode, shift_state)
                            self.queue.append((key_event.scancode, self.shift_pressed))
                        print(f"DEBUG [{device.name}]: Scancode {key_event.scancode} (Shift: {self.shift_pressed})")
        except Exception as e:
            print(f"Dispositivo {device.path} erro: {e}")

    def get_key(self):
        with self.lock:
            if self.queue:
                return self.queue.pop(0)
        return None, False

    def stop(self):
        self._stop = True

class NetworkManager:
    """Gerencia conexões WiFi e Diagnósticos de Rede."""
    
    @staticmethod
    def get_detailed_info():
        info = {
            "ip": "N/A", "gateway": "N/A", "dns": "N/A", 
            "ssid": "N/A", "signal": "0%", "tx_rx": "0/0 KB"
        }
        try:
            # IP e Interface
            gateways = netifaces.gateways()
            if 'default' in gateways and netifaces.AF_INET in gateways['default']:
                gw_info = gateways['default'][netifaces.AF_INET]
                info["gateway"] = gw_info[0]
                iface = gw_info[1]
                addrs = netifaces.ifaddresses(iface)
                if netifaces.AF_INET in addrs:
                    info["ip"] = addrs[netifaces.AF_INET][0]['addr']
                
                # Tráfego
                io = psutil.net_io_counters(pernic=True).get(iface)
                if io:
                    info["tx_rx"] = f"{io.bytes_sent//1024}/{io.bytes_recv//1024} KB"

            # DNS
            with open("/etc/resolv.conf", "r") as f:
                for line in f:
                    if line.startswith("nameserver"):
                        info["dns"] = line.split()[1]
                        break

            # WiFi Stats
            res = subprocess.run(["nmcli", "-t", "-f", "ACTIVE,SSID,SIGNAL", "dev", "wifi"], capture_output=True, text=True)
            for line in res.stdout.split("\n"):
                if line.startswith("yes"):
                    parts = line.split(":")
                    info["ssid"] = parts[1]
                    info["signal"] = f"{parts[2]}%"
                    break
        except:
            pass
        return info

    @staticmethod
    def test_connectivity():
        try:
            # Testa ping no Google DNS
            res = subprocess.run(["ping", "-c", "1", "-W", "2", "8.8.8.8"], capture_output=True)
            return res.returncode == 0
        except:
            return False

    @staticmethod
    def scan_wifi():
        try:
            subprocess.run(["nmcli", "device", "wifi", "rescan"], capture_output=True)
            output = subprocess.check_output(["nmcli", "-f", "SSID,SIGNAL", "device", "wifi", "list"]).decode("utf-8")
            lines = output.strip().split("\n")[1:]
            networks = []
            seen = set()
            for line in lines:
                parts = line.rsplit(None, 1)
                if not parts: continue
                ssid = parts[0].strip()
                if ssid == "--" or ssid in seen or not ssid: continue
                signal = parts[1] if len(parts) > 1 else "0"
                networks.append({"ssid": ssid, "signal": signal})
                seen.add(ssid)
            return networks[:10]
        except Exception as e:
            return []

    @staticmethod
    def get_known_networks():
        try:
            output = subprocess.check_output(["nmcli", "-t", "-f", "NAME", "connection", "show"]).decode("utf-8")
            return [line.strip() for line in output.split("\n") if line.strip()]
        except:
            return []

    @staticmethod
    def connect_known(ssid):
        try:
            result = subprocess.run(["nmcli", "connection", "up", ssid], capture_output=True, text=True, timeout=20)
            return result.returncode == 0, "Conectado!" if result.returncode == 0 else "Erro Conexão"
        except:
            return False, "Erro Timeout"

    @staticmethod
    def connect_wifi(ssid, password):
        try:
            # Remove conexão antiga para evitar "Not authorized"
            subprocess.run(["nmcli", "connection", "delete", ssid], capture_output=True)
            cmd = ["nmcli", "device", "wifi", "connect", ssid, "password", password]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=20)
            if result.returncode == 0:
                return True, "Conectado!"
            else:
                error_msg = result.stderr.strip()
                if "Secrets were required" in error_msg or "Not authorized" in error_msg:
                    return False, "Senha Incorreta"
                return False, "Erro Conexão"
        except Exception as e:
            return False, "Timeout/Erro"

class DeviceDiagnostic:
    """Diagnóstico de Hardware e Boot."""
    
    @staticmethod
    def scan_i2c():
        try:
            # Verifica se o binário i2cdetect existe
            if not shutil.which("i2cdetect"):
                return ["i2c-tools não inst."]
            
            # Tenta rodar i2cdetect no barramento 1 (padrão da Raspberry Pi)
            res = subprocess.run(["i2cdetect", "-y", "1"], capture_output=True, text=True)
            if res.returncode != 0:
                err = res.stderr.lower()
                if "permission denied" in err: return ["Erro Permissão"]
                if "no such file" in err or "not found" in err: return ["I2C Desativado"]
                return ["Erro Barramento"]

            output = res.stdout
            devices = []
            lines = output.strip().split("\n")[1:]
            for line in lines:
                parts = line.split(":", 1)
                if len(parts) < 2: continue
                # Extrai endereços hexadecimais encontrados (ignorando os hífens '--')
                found = re.findall(r"([0-9a-f]{2})", parts[1])
                for addr in found:
                    if addr != "  ": devices.append(addr)
            return devices if devices else ["Vazio"]
        except Exception as e:
            print(f"I2C Scan Error: {e}")
            return ["Erro Fatal"]

    @staticmethod
    def get_system_stats():
        stats = {
            "model": "Raspberry Pi",
            "cpu_temp": "0.0°C",
            "mem_usage": "0%",
            "disk_free": "0GB",
            "os_version": "Unknown",
            "ssh_sessions": 0,
            "ssh_ips": []
        }
        try:
            # Modelo
            with open("/proc/device-tree/model", "r") as f:
                stats["model"] = f.read().strip().replace("\x00", "")
            
            # Temp
            with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
                stats["cpu_temp"] = f"{int(f.read()) / 1000:.1f}°C"
            
            # Mem e Disco
            stats["mem_usage"] = f"{psutil.virtual_memory().percent}%"
            stats["disk_free"] = f"{shutil.disk_usage('/').free // (1024**3)}GB"
            
            # OS
            with open("/etc/os-release", "r") as f:
                for line in f:
                    if line.startswith("PRETTY_NAME"):
                        stats["os_version"] = line.split("=")[1].strip().replace('"', '')
                        break
            
            # SSH Sessions (using psutil to find sshd connections)
            ssh_ips = set()
            for conn in psutil.net_connections(kind='tcp'):
                if conn.laddr.port == 22 and conn.status == 'ESTABLISHED':
                    if conn.raddr:
                        ssh_ips.add(conn.raddr.ip)
            stats["ssh_sessions"] = len(ssh_ips)
            stats["ssh_ips"] = list(ssh_ips)
        except:
            pass
        return stats

    @staticmethod
    def get_boot_info():
        # Caminhos padrão do sistema de boot
        paths = {
            "config": "/boot/config.txt" if os.path.exists("/boot/config.txt") else "/boot/firmware/config.txt",
            "cmdline": "/boot/cmdline.txt" if os.path.exists("/boot/cmdline.txt") else "/boot/firmware/cmdline.txt"
        }
        
        # Verifica se I2C está habilitado no config.txt
        i2c_enabled = False
        try:
            if os.path.exists(paths["config"]):
                with open(paths["config"], "r") as f:
                    content = f.read()
                    if "dtparam=i2c_arm=on" in content and not content.strip().startswith("#"):
                        i2c_enabled = True
        except:
            pass
            
        paths["i2c_enabled"] = i2c_enabled
        return paths

class SFPReader:
    """Comunica com o daemon do SFP via Socket Unix."""
    SOCKET_PATH = "/run/sfp-daemon/sfp.sock"
    _cache = None
    _lock = threading.Lock()
    _last_read = 0

    @staticmethod
    def get_data():
        """Retorna dados do cache ou tenta ler se estiver expirado (>0.5s)"""
        now = time.time()
        with SFPReader._lock:
            if SFPReader._cache and (now - SFPReader._last_read < 0.5):
                return SFPReader._cache
        
        data = SFPReader._fetch_from_socket()
        if data:
            with SFPReader._lock:
                SFPReader._cache = data
                SFPReader._last_read = now
        return data

    @staticmethod
    def _fetch_from_socket():
        try:
            if not os.path.exists(SFPReader.SOCKET_PATH):
                print(f"DEBUG: Socket not found at {SFPReader.SOCKET_PATH}")
                return None
                
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
                s.settimeout(1.0) # Aumentado para 1s para garantir leitura completa
                s.connect(SFPReader.SOCKET_PATH)
                s.sendall(b"GET CURRENT\n")
                
                buffer = ""
                while True:
                    chunk = s.recv(4096).decode("utf-8", errors="replace")
                    if not chunk: break
                    buffer += chunk
                    # O daemon envia uma resposta terminada em \n\n ou quando o JSON fecha
                    if buffer.strip().endswith("}"):
                        break
                
                # O daemon retorna "STATUS 200 OK\n{...}"
                if "{" in buffer:
                    json_str = buffer[buffer.find("{"):]
                    data = json.loads(json_str)
                    return data
            return None
        except Exception as e:
            print(f"DEBUG SFP ERROR: {e}")
            return None

class MenuSystem:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.state = "MAIN_MENU"
        self.selected_index = 0
        self.wifi_list = []
        self.password = ""
        self.status_msg = ""
        self.status_timer = 0
        self.sfp_data = None
        self.last_update = 0
        self.show_password = False
        self.known_networks = []
        self.i2c_devices = []
        
        try:
            self.font_title = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 26)
            self.font_text = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18)
            self.font_small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14)
            self.font_tiny = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 12)
        except IOError:
            self.font_title = self.font_text = self.font_small = self.font_tiny = ImageFont.load_default()

        self.logo = None
        self.logo_h = 0
        try:
            logo_path = "./assets/virtus-cc.png"
            if os.path.exists(logo_path):
                self.logo = Image.open(logo_path).convert("RGBA")
                aspect = self.logo.height / self.logo.width
                new_h = int(self.width * aspect)
                self.logo = self.logo.resize((self.width, new_h), Image.LANCZOS)
                self.logo_h = new_h
        except:
            pass

    def set_status(self, msg, duration=3):
        self.status_msg = msg
        self.status_timer = time.time() + duration

    def draw_header_with_logo(self, image, draw):
        y_start = 70
        if self.logo:
            image.paste(self.logo, (0, 0), self.logo)
            y_start = self.logo_h + 5
            draw.line((0, self.logo_h, self.width, self.logo_h), fill=COLOR_ACCENT, width=1)
        return y_start

    def render(self):
        image = Image.new("RGB", (self.width, self.height), COLOR_BG)
        draw = ImageDraw.Draw(image)
        y = self.draw_header_with_logo(image, draw)
        footer_text = "ESC para Voltar"

        if self.state == "MAIN_MENU":
            items = ["Leitura SFP", "Rede & Debug", "Scan I2C", "Sistema & Boot", "Configurar WiFi", "Reiniciar"]
            for i, item in enumerate(items):
                color = COLOR_HIGHLIGHT if i == self.selected_index else COLOR_TEXT
                prefix = "> " if i == self.selected_index else "  "
                draw.text((10, y + (i * 26)), f"{prefix}{item}", font=self.font_text, fill=color)

        elif self.state == "SFP_VIEW":
            if self.sfp_data:
                a2, a0 = self.sfp_data.get("a2", {}), self.sfp_data.get("a0", {})
                
                # Lista de dados formatada para rolagem
                sfp_items = [
                    (f"RX: {a2.get('rx_power_dbm','N/A')} dBm", COLOR_HIGHLIGHT),
                    (f"TX: {a2.get('tx_bias_ma','N/A')} mA", COLOR_HIGHLIGHT),
                    (f"Temp: {a2.get('temperature_c','N/A')} C", COLOR_TEXT),
                    (f"Volt: {a2.get('voltage_v','N/A')} V", COLOR_TEXT),
                    (f"Vend: {a0.get('vendor_name','Unknown')[:15]}", COLOR_ACCENT),
                    (f"PN: {a0.get('vendor_pn','Unknown')[:15]}", COLOR_ACCENT),
                    (f"Ident: {a0.get('identifier_type','N/A')}", COLOR_TEXT),
                    (f"Wave: {a0.get('wavelength_nm','N/A')} nm", COLOR_TEXT),
                    (f"Conn: {a0.get('connector_type','N/A')}", COLOR_TEXT)
                ]

                max_visible = 5
                start_idx = self.selected_index if self.selected_index < len(sfp_items) else 0
                # Ajusta janela de visualização se necessário
                if self.selected_index >= max_visible:
                    start_idx = self.selected_index - max_visible + 1
                else:
                    start_idx = 0

                for i in range(max_visible):
                    curr_idx = start_idx + i
                    if curr_idx >= len(sfp_items): break
                    text, color = sfp_items[curr_idx]
                    prefix = "> " if curr_idx == self.selected_index else "  "
                    draw.text((10, y + (i * 28)), f"{prefix}{text}", font=self.font_text, fill=color)

                # Indicadores de rolagem (triângulos)
                if len(sfp_items) > max_visible:
                    if start_idx > 0: # Seta Cima
                        draw.polygon([(self.width-15, y-5), (self.width-5, y-5), (self.width-10, y-12)], fill=COLOR_ACCENT)
                    if start_idx + max_visible < len(sfp_items): # Seta Baixo
                        draw.polygon([(self.width-15, y+140), (self.width-5, y+140), (self.width-10, y+147)], fill=COLOR_ACCENT)

                draw.text((10, self.height - 45), "ATUALIZANDO...", font=self.font_small, fill=COLOR_ACCENT)
            else:
                draw.text((10, y), "Erro SFP Daemon", font=self.font_text, fill=COLOR_ERROR)
            draw.text((10, self.height - 25), footer_text, font=self.font_small, fill=COLOR_ACCENT)

        elif self.state == "NET_DEBUG":
            net = NetworkManager.get_detailed_info()
            conn = "OK" if NetworkManager.test_connectivity() else "FAIL"
            draw.text((10, y), f"IP: {net['ip']}", font=self.font_text, fill=COLOR_TEXT)
            draw.text((10, y + 25), f"GW: {net['gateway']}", font=self.font_text, fill=COLOR_ACCENT)
            draw.text((10, y + 50), f"WiFi: {net['signal']}", font=self.font_text, fill=COLOR_HIGHLIGHT)
            draw.text((10, y + 75), f"Web: {conn}", font=self.font_text, fill=COLOR_TEXT)
            # Novas rotas solicitadas
            draw.text((10, y + 100), f"Page: {net['ip']}:8000", font=self.font_text, fill=COLOR_ACCENT)
            draw.text((10, y + 115), f"IO: {net['ip']}:9443", font=self.font_text, fill=COLOR_ACCENT)
            draw.text((10, self.height - 25), footer_text, font=self.font_small, fill=COLOR_ACCENT)

        elif self.state == "I2C_SCAN":
            draw.text((10, y), "Dispositivos I2C:", font=self.font_text, fill=COLOR_HIGHLIGHT)
            dev_str = ", ".join(self.i2c_devices)
            # Quebra a string em múltiplas linhas se for muito longa
            max_chars = 20
            lines = [dev_str[i:i+max_chars] for i in range(0, len(dev_str), max_chars)]
            for i, line in enumerate(lines):
                draw.text((15, y + 30 + (i * 20)), line, font=self.font_text, fill=COLOR_TEXT)
            draw.text((10, self.height - 25), footer_text, font=self.font_small, fill=COLOR_ACCENT)

        elif self.state == "SYS_DIAG":
            sys = DeviceDiagnostic.get_system_stats()
            draw.text((10, y), f"Temp: {sys['cpu_temp']}", font=self.font_text, fill=COLOR_HIGHLIGHT)
            draw.text((10, y + 25), f"Mem: {sys['mem_usage']}", font=self.font_text, fill=COLOR_TEXT)
            draw.text((10, y + 50), f"Disk: {sys['disk_free']}", font=self.font_text, fill=COLOR_TEXT)
            
            # SSH Info
            ssh_status = f"SSH: {sys['ssh_sessions']} Ativos"
            draw.text((10, y + 75), ssh_status, font=self.font_text, fill=COLOR_HIGHLIGHT if sys['ssh_sessions'] > 0 else COLOR_ACCENT)
            
            # Lista IPs SSH se houver
            if sys['ssh_ips']:
                ips_str = ", ".join(sys['ssh_ips'][:2]) # Mostra até 2 IPs
                draw.text((10, y + 100), f"IPs: {ips_str}", font=self.font_small, fill=COLOR_ACCENT)
            else:
                draw.text((10, y + 100), "Nenhum IP SSH", font=self.font_small, fill=COLOR_ACCENT)
            
            draw.text((10, y + 120), f"OS: {sys['os_version'][:15]}", font=self.font_tiny, fill=COLOR_ACCENT)
            draw.text((10, self.height - 25), footer_text, font=self.font_small, fill=COLOR_ACCENT)

        elif self.state == "WIFI_SCAN":
            if not self.wifi_list:
                draw.text((10, y), "Buscando...", font=self.font_text, fill=COLOR_TEXT)
            else:
                # Sistema de rolagem para muitas redes
                max_visible = 5  # Número de redes visíveis ao mesmo tempo
                start_index = 0
                if self.selected_index >= max_visible:
                    start_index = self.selected_index - max_visible + 1
                
                visible_nets = self.wifi_list[start_index:start_index + max_visible]
                
                for i, net in enumerate(visible_nets):
                    actual_index = start_index + i
                    color = COLOR_HIGHLIGHT if actual_index == self.selected_index else COLOR_TEXT
                    prefix = "> " if actual_index == self.selected_index else "  "
                    
                    # Nome da rede truncado para caber na tela com fonte maior
                    ssid_display = net['ssid'][:12] + ".." if len(net['ssid']) > 12 else net['ssid']
                    is_known = net['ssid'] in self.known_networks
                    prefix = "* " if is_known else "> " if actual_index == self.selected_index else "  "
                    if actual_index == self.selected_index: prefix = "> "
                    
                    draw.text((10, y + (i * 30)), f"{prefix}{ssid_display} {net['signal']}%", font=self.font_text, fill=color)
                    if is_known and actual_index != self.selected_index:
                        draw.text((self.width - 40, y + (i * 30)), "(S)", font=self.font_tiny, fill=COLOR_ACCENT)
                
                # Indicador de mais redes
                if len(self.wifi_list) > max_visible:
                    if start_index > 0:
                        draw.polygon([(self.width-15, y-5), (self.width-5, y-5), (self.width-10, y-12)], fill=COLOR_ACCENT) # Seta Cima
                    if start_index + max_visible < len(self.wifi_list):
                        draw.polygon([(self.width-15, y+145), (self.width-5, y+145), (self.width-10, y+152)], fill=COLOR_ACCENT) # Seta Baixo
            
            draw.text((10, self.height - 25), footer_text, font=self.font_small, fill=COLOR_ACCENT)

        elif self.state == "WIFI_INPUT":
            ssid = self.wifi_list[self.selected_index]['ssid']
            draw.text((10, y), f"Senha para: {ssid}", font=self.font_small, fill=COLOR_HIGHLIGHT)
            
            # Caixa de Entrada
            draw.rectangle((10, y + 25, self.width - 10, y + 55), outline=COLOR_HIGHLIGHT, width=2)
            
            # Mostra senha ou asteriscos
            display_pass = self.password if self.show_password else "*" * len(self.password)
            draw.text((15, y + 31), f"{display_pass}|", font=self.font_text, fill=COLOR_TEXT)
            
            # Dicas de Teclas (Botões Virtuais)
            draw.rectangle((10, self.height - 45, 110, self.height - 25), outline=COLOR_ACCENT)
            draw.text((15, self.height - 42), "BACKSPACE: Del", font=self.font_tiny, fill=COLOR_TEXT)
            
            draw.rectangle((120, self.height - 45, 230, self.height - 25), outline=COLOR_ACCENT)
            draw.text((125, self.height - 42), "TAB: Ver/Ocultar", font=self.font_tiny, fill=COLOR_TEXT)
            
            draw.text((10, self.height - 20), "ESC: Cancelar  |  ENTER: Conectar", font=self.font_tiny, fill=COLOR_ACCENT)

        elif self.state == "LOADING":
            draw.text((10, y), "Processando...", font=self.font_text, fill=COLOR_HIGHLIGHT)
            draw.text((10, y + 30), "Por favor, aguarde...", font=self.font_small, fill=COLOR_TEXT)
            # Rodapé informativo
            draw.text((10, self.height - 25), "Operação em curso", font=self.font_small, fill=COLOR_ACCENT)

        if time.time() < self.status_timer:
            draw.rectangle((0, self.height - 25, self.width, self.height), fill=COLOR_HIGHLIGHT)
            draw.text((10, self.height - 22), self.status_msg, font=self.font_small, fill=(0,0,0))

        return image

    def handle_input(self, key_data):
        key, shift = key_data
        if key is None: return
        is_up, is_down = key in [103, 105], key in [108, 106]
        is_enter, is_esc = key in [28, 96], key in [1]
        is_backspace = key == 14

        if self.state == "MAIN_MENU":
            if is_up: self.selected_index = (self.selected_index - 1) % 6
            elif is_down: self.selected_index = (self.selected_index + 1) % 6
            elif is_enter:
                if self.selected_index == 0: 
                    self.state = "LOADING"
                    self.selected_index = 0
                    threading.Thread(target=self._update_sfp, daemon=True).start()
                elif self.selected_index == 1: self.state = "NET_DEBUG"
                elif self.selected_index == 2: 
                    self.state = "LOADING"
                    threading.Thread(target=self._update_i2c, daemon=True).start()
                elif self.selected_index == 3: self.state = "SYS_DIAG"
                elif self.selected_index == 4: 
                    self.state = "LOADING"
                    self.selected_index = 0
                    threading.Thread(target=self._update_wifi, daemon=True).start()
                elif self.selected_index == 5: os.system("sudo reboot")
            elif is_esc: self.state = "MAIN_MENU"

        elif self.state == "SFP_VIEW":
            # Permite rolagem nos dados SFP
            max_items = 9 # Total de itens na lista sfp_items
            if is_up: self.selected_index = (self.selected_index - 1) % max_items
            elif is_down: self.selected_index = (self.selected_index + 1) % max_items
            elif is_esc or is_enter: self.state = "MAIN_MENU"

        elif self.state in ["NET_DEBUG", "SYS_DIAG", "I2C_SCAN"]:
            if is_esc or is_enter: self.state = "MAIN_MENU"

        elif self.state == "WIFI_SCAN":
            if is_up: self.selected_index = (self.selected_index - 1) % len(self.wifi_list) if self.wifi_list else 0
            elif is_down: self.selected_index = (self.selected_index + 1) % len(self.wifi_list) if self.wifi_list else 0
            elif is_enter and self.wifi_list:
                ssid = self.wifi_list[self.selected_index]['ssid']
                if ssid in self.known_networks:
                    self.set_status("Conectando (Salva)...")
                    self.state = "LOADING"
                    threading.Thread(target=self._connect_known_task, args=(ssid,), daemon=True).start()
                else:
                    self.state = "WIFI_INPUT"
                    self.password = ""
                    self.show_password = False
            elif is_esc: self.state = "MAIN_MENU"

        elif self.state == "WIFI_INPUT":
            if is_esc: self.state = "WIFI_SCAN"
            elif key == 15: # TAB para alternar visibilidade da senha
                self.show_password = not self.show_password
            elif is_enter:
                ssid = self.wifi_list[self.selected_index]['ssid']
                self.set_status("Conectando...")
                self.state = "LOADING"
                threading.Thread(target=self._connect_task, args=(ssid, self.password), daemon=True).start()
            elif is_backspace: 
                self.password = self.password[:-1]
            else:
                char = self._map_code_to_char(key, shift)
                if char: self.password += char

    def _map_code_to_char(self, code, shift=False):
        mapping = {
            2:"1", 3:"2", 4:"3", 5:"4", 6:"5", 7:"6", 8:"7", 9:"8", 10:"9", 11:"0", 12:"-", 13:"=",
            16:"q", 17:"w", 18:"e", 19:"r", 20:"t", 21:"y", 22:"u", 23:"i", 24:"o", 25:"p",
            30:"a", 31:"s", 32:"d", 33:"f", 34:"g", 35:"h", 36:"j", 37:"k", 38:"l",
            44:"z", 45:"x", 46:"c", 47:"v", 48:"b", 49:"n", 50:"m", 52:".", 57:" "
        }
        mapping_shift = {
            2:"!", 3:"@", 4:"#", 5:"$", 6:"%", 7:"^", 8:"&", 9:"*", 10:"(", 11:")", 12:"_", 13:"+",
            16:"Q", 17:"W", 18:"E", 19:"R", 20:"T", 21:"Y", 22:"U", 23:"I", 24:"O", 25:"P",
            30:"A", 31:"S", 32:"D", 33:"F", 34:"G", 35:"H", 36:"J", 37:"K", 38:"L",
            44:"Z", 45:"X", 46:"C", 47:"V", 48:"B", 49:"N", 50:"M", 52:">", 57:" "
        }
        res = mapping_shift.get(code) if shift else mapping.get(code)
        return res

    def _update_wifi(self): 
        try:
            self.known_networks = NetworkManager.get_known_networks()
            self.wifi_list = NetworkManager.scan_wifi()
        except Exception as e:
            self.set_status("Erro Scan WiFi")
        finally:
            self.state = "WIFI_SCAN"

    def _update_sfp(self): 
        try:
            self.sfp_data = SFPReader.get_data()
            self.last_update = time.time()
        except Exception as e:
            self.set_status("Erro Leitura SFP")
        finally:
            if self.state == "LOADING":
                self.state = "SFP_VIEW"

    def _update_i2c(self):
        try:
            self.i2c_devices = DeviceDiagnostic.scan_i2c()
        except Exception as e:
            self.set_status("Erro Scan I2C")
        finally:
            self.state = "I2C_SCAN"

    def _connect_task(self, ssid, password):
        try:
            success, msg = NetworkManager.connect_wifi(ssid, password)
            self.set_status(msg)
            # Transição automática imediata em caso de sucesso
            if success:
                self.state = "NET_DEBUG"
            else:
                time.sleep(1.5) # Delay apenas se houver erro para o usuário ler o status
                self.state = "WIFI_SCAN"
        except Exception as e:
            self.set_status("Erro Interno")
            self.state = "WIFI_SCAN"

    def _connect_known_task(self, ssid):
        try:
            success, msg = NetworkManager.connect_known(ssid)
            self.set_status(msg)
            if success:
                self.state = "NET_DEBUG"
            else:
                time.sleep(1.5)
                self.state = "WIFI_SCAN"
        except Exception as e:
            self.set_status("Erro Conexão")
            self.state = "WIFI_SCAN"

def main():
    print("Iniciando Display Interativo...")
    gpio_adapter = None
    keyboard = None
    disp = None
    try:
        gpio_adapter = LGPIOAdapter()
        keyboard = KeyboardHandler()
        spi = SPI.SpiDev(SPI_PORT, SPI_DEVICE, max_speed_hz=SPI_SPEED_HZ)
        disp = ST7789(spi, mode=3, rst=RST_PIN, dc=DC_PIN, led=BLK_PIN, gpio=gpio_adapter)
        disp.begin()
        disp.clear()
        gpio_adapter.set_high(BLK_PIN)
        menu = MenuSystem(disp.width, disp.height)
        last_img = menu.render()
        disp.display(last_img)
        while True:
            key_data = keyboard.get_key()
            key = key_data[0]
            needs_update = False
            if key:
                if menu.handle_input(key_data) == "EXIT": break
                needs_update = True
            if menu.state in ["SFP_VIEW", "NET_DEBUG", "SYS_DIAG"] and (time.time() - menu.last_update > 2):
                if menu.state == "SFP_VIEW": menu._update_sfp()
                menu.last_update = time.time()
                needs_update = True
            if time.time() < menu.status_timer: needs_update = True
            if needs_update:
                img = menu.render()
                disp.display(img)
                time.sleep(0.01)
            else:
                time.sleep(0.05)
    except KeyboardInterrupt: print("Encerrando...")
    except Exception as e: print(f"Erro Fatal: {e}")
    finally:
        if disp: disp.clear()
        if gpio_adapter: gpio_adapter.cleanup()
        if keyboard: keyboard.stop()

if __name__ == "__main__": main()
