import time
import socket
import subprocess
import netifaces
from PIL import Image, ImageDraw, ImageFont
import Adafruit_GPIO as GPIO
import Adafruit_GPIO.SPI as SPI
import lgpio
from ST7789 import ST7789

# Configuração dos Pinos
DC_PIN = 25
RST_PIN = 27
BLK_PIN = 24
SPI_PORT = 0
SPI_DEVICE = 0
SPI_SPEED_HZ = 40000000

class LGPIOAdapter:
    """Adapta a interface do lgpio para o formato esperado pela biblioteca Adafruit_GPIO/ST7789"""
    def __init__(self):
        self._chip = lgpio.gpiochip_open(0)
        
    def setup(self, pin, mode):
        # Adafruit_GPIO.OUT é 1, IN é 0
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
            
    # ST7789 pode chamar métodos plataforma-específicos, adicionamos stubs se necessário

def get_ip_address():
    """Obtém o endereço IP principal da máquina."""
    try:
        cmd = "hostname -I | awk '{print $1}'"
        ip = subprocess.check_output(cmd, shell=True).decode("utf-8").strip()
        if ip:
            return ip
    except:
        pass
    
    try:
        for iface in ['eth0', 'wlan0', 'en0']:
            if iface in netifaces.interfaces():
                addrs = netifaces.ifaddresses(iface)
                if netifaces.AF_INET in addrs:
                    return addrs[netifaces.AF_INET][0]['addr']
    except:
        pass
        
    return "No IP"

def create_image(width, height, ip_address):
    """Cria a imagem para ser exibida."""
    image = Image.new("RGB", (width, height), (30, 30, 46)) # Dark blue background
    draw = ImageDraw.Draw(image)
    
    try:
        font_title = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 26)
        font_text = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18)
        font_small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14)
    except IOError:
        font_title = ImageFont.load_default()
        font_text = ImageFont.load_default()
        font_small = ImageFont.load_default()

    # --- Branding VIRTUS CC ---
    # --- Branding VIRTUS CC ---
    
    y_start = 70 # Valor padrão caso o logo falhe
    
    # Tenta carregar e desenhar o logo FULL WIDTH
    try:
        logo_path = "packages/display/assets/virtus-cc.png"
        logo = Image.open(logo_path).convert("RGBA")
        
        # Redimensiona para largura total (240px) mantendo aspect ratio
        target_w = width
        aspect = logo.height / logo.width
        new_h = int(target_w * aspect)
        logo = logo.resize((target_w, new_h), Image.LANCZOS)
        
        # Cola o logo no topo (0,0)
        # Usamos paste com máscara para transparência, mas o fundo padrão é azul escuro
        image.paste(logo, (0, 0), logo)
        
        # Define onde começa o texto (altura do logo + 10px padding)
        y_start = new_h + 10
        
        # Opcional: Desenhar uma linha divisória logo abaixo do logo
        draw.line((0, new_h, width, new_h), fill=(148, 163, 184), width=2)
        
    except Exception as e:
        # Fallback se imagem não existir: Cabeçalho VIRTUS CC Texto antigo
        print(f"Erro ao carregar logo: {e}")
        draw.rectangle((0, 0, width, 50), fill=(23, 37, 84))
        draw.text((10, 10), "VIRTUS", font=font_title, fill=(255, 255, 255))
        virtus_width = draw.textlength("VIRTUS", font=font_title) if hasattr(draw, "textlength") else 100
        draw.text((10 + virtus_width + 8, 10), "CC", font=font_title, fill=(6, 182, 212))
        draw.line((0, 50, width, 50), fill=(148, 163, 184), width=2)
        y_start = 70

    # --- Informações ---
    
    # IP
    draw.text((10, y_start), "IP Address:", font=font_small, fill=(148, 163, 184))
    draw.text((10, y_start + 18), ip_address, font=font_text, fill=(255, 255, 255))
    
    # URL Frontend
    y_start += 50
    draw.text((10, y_start), "Frontend:", font=font_small, fill=(148, 163, 184))
    draw.text((10, y_start + 18), f"http://{ip_address}:3000", font=font_small, fill=(100, 255, 100))
    
    # URL API
    y_start += 50
    draw.text((10, y_start), "API:", font=font_small, fill=(148, 163, 184))
    draw.text((10, y_start + 18), f"http://{ip_address}:8000", font=font_small, fill=(100, 255, 100))

    return image

def main():
    print("Inicializando Display ST7789 (User Lib + lgpio adapter)...")
    
    # Adaptador GPIO para evitar erro de detecção de plataforma da Adafruit_GPIO
    gpio_adapter = LGPIOAdapter()
    
    try:
        # Configura Hardware SPI
        spi = SPI.SpiDev(SPI_PORT, SPI_DEVICE, max_speed_hz=SPI_SPEED_HZ)
        
        # Instancia Display com adaptador GPIO customizado
        disp = ST7789(spi, mode=3, rst=RST_PIN, dc=DC_PIN, led=BLK_PIN, gpio=gpio_adapter)
        
        # Inicializa
        disp.begin()
        disp.clear()
        
        # Força Backlight (caso a lib tenha falhado nisso)
        gpio_adapter.set_high(BLK_PIN)
        
        print("Display iniciado. Loop de atualização...")
        
        while True:
            ip = get_ip_address()
            img = create_image(disp.width, disp.height, ip)
            disp.display(img)
            time.sleep(30)
            
    except KeyboardInterrupt:
        print("Encerrando...")
        disp.clear()
        gpio_adapter.cleanup()
    except Exception as e:
        print(f"Erro: {e}")
        gpio_adapter.cleanup()
        raise

if __name__ == "__main__":
    main()
