import time
import socket
import subprocess
import netifaces
from PIL import Image, ImageDraw, ImageFont
import Adafruit_GPIO.SPI as SPI
from ST7789 import ST7789

# Configuração dos Pinos (Confirmado pelo usuário)
# DC  -> GPIO 25
# RES -> GPIO 27
# BLK -> GPIO 24
# SPI -> Port 0, Device 0 (GPIO 10/11)

DC_PIN = 25
RST_PIN = 27
BLK_PIN = 24
SPI_PORT = 0
SPI_DEVICE = 0
SPI_SPEED_HZ = 40000000 # 40MHz

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
    # Fundo do cabeçalho
    draw.rectangle((0, 0, width, 50), fill=(23, 37, 84))
    
    # Texto VIRTUS (Branco)
    draw.text((10, 10), "VIRTUS", font=font_title, fill=(255, 255, 255))
    
    # Texto CC (Ciano) - Calcula largura para posicionar ao lado
    virtus_width = draw.textlength("VIRTUS", font=font_title) if hasattr(draw, "textlength") else 100
    draw.text((10 + virtus_width + 8, 10), "CC", font=font_title, fill=(6, 182, 212))
    
    # Divisor
    draw.line((0, 50, width, 50), fill=(148, 163, 184), width=2)

    # --- Informações ---
    y_start = 70
    
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
    print("Inicializando Display ST7789 (User Lib)...")
    
    # Configura Hardware SPI
    spi = SPI.SpiDev(SPI_PORT, SPI_DEVICE, max_speed_hz=SPI_SPEED_HZ)
    
    # Instancia Display
    # mode=3 é o padrão da lib e recomendado para ST7789
    disp = ST7789(spi, mode=3, rst=RST_PIN, dc=DC_PIN, led=BLK_PIN)
    
    # Inicializa
    disp.begin()
    disp.clear()
    
    print("Display iniciado. Loop de atualização...")
    
    try:
        while True:
            ip = get_ip_address()
            img = create_image(disp.width, disp.height, ip)
            disp.display(img)
            time.sleep(30)
            
    except KeyboardInterrupt:
        print("Encerrando...")
        disp.clear()

if __name__ == "__main__":
    main()
