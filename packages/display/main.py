import time
import socket
import subprocess
from PIL import Image, ImageDraw, ImageFont
import netifaces
from st7789 import ST7789

def get_ip_address():
    """Obtém o endereço IP principal da máquina."""
    try:
        # Tenta obter via hostname -I (comum em Linux/RPi)
        cmd = "hostname -I | awk '{print $1}'"
        ip = subprocess.check_output(cmd, shell=True).decode("utf-8").strip()
        if ip:
            return ip
    except:
        pass
    
    # Fallback usando netifaces
    try:
        # Tenta interface eth0 ou wlan0
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
    image = Image.new("RGB", (width, height), (30, 30, 46)) # Dark blue background (similar to TUI)
    draw = ImageDraw.Draw(image)
    
    # Tenta carregar fontes
    try:
        # Tenta fonte padrão do sistema RPi
        font_title = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24)
        font_text = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18)
        font_small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14)
    except IOError:
        # Fallback
        font_title = ImageFont.load_default()
        font_text = ImageFont.load_default()
        font_small = ImageFont.load_default()

    # --- Cabeçalho VIRTUS CC ---
    # Fundo do cabeçalho
    draw.rectangle((0, 0, width, 50), fill=(23, 37, 84)) # Darker blue
    
    # Texto VIRTUS (Branco)
    draw.text((10, 10), "VIRTUS", font=font_title, fill=(255, 255, 255))
    
    # Texto CC (Ciano) - Calcula largura de "VIRTUS" para posicionar "CC"
    virtus_width = draw.textlength("VIRTUS", font=font_title) if hasattr(draw, "textlength") else 100
    draw.text((10 + virtus_width + 5, 10), "CC", font=font_title, fill=(6, 182, 212)) # Cyan
    
    # Linha divisória
    draw.line((0, 50, width, 50), fill=(148, 163, 184), width=2)

    # --- Informações do Sistema ---
    
    # IP Address
    y_offset = 70
    draw.text((10, y_offset), "IP Address:", font=font_small, fill=(148, 163, 184)) # Gray
    draw.text((10, y_offset + 20), ip_address, font=font_text, fill=(255, 255, 255))
    
    # Frontend URL
    y_offset += 50
    draw.text((10, y_offset), "Frontend:", font=font_small, fill=(148, 163, 184))
    draw.text((10, y_offset + 20), f"http://{ip_address}:3000", font=font_small, fill=(100, 255, 100)) # Green
    
    # API URL
    y_offset += 50
    draw.text((10, y_offset), "API:", font=font_small, fill=(148, 163, 184))
    draw.text((10, y_offset + 20), f"http://{ip_address}:8000", font=font_small, fill=(100, 255, 100)) # Green

    return image

def main():
    print("SFP Display Service Starting...")
    
    # Inicializa driver
    # Ajuste os pinos aqui se necessário, conforme AGENTS.md
    disp = ST7789(rotation=0,  # Tente 0, 90, 180, 270 se a imagem estiver rotacionada
                  port=0, 
                  cs=0,       # SPI CE0
                  dc=25,      # GPIO 25
                  backlight=24, # GPIO 24
                  rst=27)     # GPIO 27
                  
    print("Display Initialized.")
    
    try:
        while True:
            ip = get_ip_address()
            img = create_image(disp.width, disp.height, ip)
            disp.display(img)
            
            # Atualiza a cada 30 segundos
            time.sleep(30)
            
    except KeyboardInterrupt:
        print("Stopping...")
        disp.close()
    except Exception as e:
        print(f"Error: {e}")
        disp.close()

if __name__ == "__main__":
    main()
