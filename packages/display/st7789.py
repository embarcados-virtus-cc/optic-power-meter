import time
import spidev
import lgpio
import logging

# Constantes de Comandos ST7789
ST7789_SWRESET = 0x01
ST7789_SLPOUT  = 0x11
ST7789_NORON   = 0x13
ST7789_INVOFF  = 0x20
ST7789_INVON   = 0x21
ST7789_DISPOFF = 0x28
ST7789_DISPON  = 0x29
ST7789_CASET   = 0x2A
ST7789_RASET   = 0x2B
ST7789_RAMWR   = 0x2C
ST7789_MADCTL  = 0x36
ST7789_COLMOD  = 0x3A

class ST7789:
    """
    Driver para display LCD ST7789 via SPI.
    Utiliza lgpio para controle de pinos.
    """
    def __init__(self, rotation=0, port=0, cs=0, dc=25, backlight=24, rst=27, spi_speed_hz=40000000):
        """
        Inicializa o display.
        
        Args:
            rotation (int): Rotação do display (0, 90, 180, 270).
            port (int): Porta SPI (0 ou 1).
            cs (int): Chip Select SPI (0 ou 1). Para displays de 7 pinos (sem CS), este parâmetro define qual canal SPI o RPi usa (CE0 ou CE1), mas o pino físico pode ser deixado desconectado.
            dc (int): Pino GPIO para Data/Command (BCM).
            backlight (int): Pino GPIO para Backlight (BCM).
            rst (int): Pino GPIO para Reset (BCM).
            spi_speed_hz (int): Velocidade SPI em Hz.
        """
        self._rotation = rotation
        self._dc = dc
        self._rst = rst
        self._bl = backlight
        
        # Dimensões padrão para GMT130 (240x240)
        self.width = 240
        self.height = 240
        
        # Inicializa GPIO (Chip 0 geralmente é o padrão no RPi)
        try:
            self._h = lgpio.gpiochip_open(0)
            
            # Configura pinos como saída
            lgpio.gpio_claim_output(self._h, self._dc)
            lgpio.gpio_claim_output(self._h, self._rst)
            lgpio.gpio_claim_output(self._h, self._bl)
            
            # Inicializa Backlight (Ligado)
            lgpio.gpio_write(self._h, self._bl, 1)
            
        except Exception as e:
            logging.error(f"Failed to initialize GPIO: {e}")
            raise
        
        # Configuração SPI
        self._spi = spidev.SpiDev()
        self._spi.open(port, cs)
        self._spi.max_speed_hz = spi_speed_hz
        self._spi.mode = 0b00  # CPOL=0, CPHA=0
        
        self.reset()
        self._init_display()

    def reset(self):
        """Reinicia o display via hardware."""
        lgpio.gpio_write(self._h, self._rst, 1)
        time.sleep(0.050)
        lgpio.gpio_write(self._h, self._rst, 0)
        time.sleep(0.050)
        lgpio.gpio_write(self._h, self._rst, 1)
        time.sleep(0.050)

    def command(self, data):
        """Envia byte de comando."""
        lgpio.gpio_write(self._h, self._dc, 0)  # Command mode
        self._spi.writebytes([data])

    def data(self, data):
        """Envia byte(s) de dados."""
        lgpio.gpio_write(self._h, self._dc, 1)  # Data mode
        self._spi.writebytes([data])

    def _init_display(self):
        """Sequência de inicialização do ST7789."""
        self.command(ST7789_SWRESET) # Software reset
        time.sleep(0.150)
        
        self.command(ST7789_SLPOUT)  # Out of sleep mode
        time.sleep(0.500)
        
        self.command(ST7789_COLMOD)  # Set color mode
        self.data(0x55)              # 16-bit color
        
        self.command(ST7789_MADCTL)  # Memory access control (rotation)
        self.data(0x00)              # Default rotation (adjust later if needed)
        
        self.command(ST7789_CASET)   # Column address set
        self.data(0x00)
        self.data(0x00)              # Start column = 0
        self.data((self.width - 1) >> 8)
        self.data((self.width - 1) & 0xFF) # End column
        
        self.command(ST7789_RASET)   # Row address set
        self.data(0x00)
        self.data(0x00)              # Start row = 0
        self.data((self.height - 1) >> 8)
        self.data((self.height - 1) & 0xFF) # End row
        
        self.command(ST7789_INVON)   # Inversion ON (Critical for IPS displays usually)
        time.sleep(0.010)
        
        self.command(ST7789_NORON)   # Normal display on
        time.sleep(0.010)
        
        self.command(ST7789_DISPON)  # Display on
        time.sleep(0.500)

    def set_window(self, x0, y0, x1, y1):
        """Define a janela de desenho."""
        self.command(ST7789_CASET) # Column addr set
        self.data(x0 >> 8)
        self.data(x0 & 0xFF)
        self.data(x1 >> 8)
        self.data(x1 & 0xFF)

        self.command(ST7789_RASET) # Row addr set
        self.data(y0 >> 8)
        self.data(y0 & 0xFF)
        self.data(y1 >> 8)
        self.data(y1 & 0xFF)

        self.command(ST7789_RAMWR) # Write to RAM

    def display(self, image):
        """
        Escreve uma imagem PIL no display.
        A imagem deve ser RGB e ter o mesmo tamanho do display.
        """
        if image.size != (self.width, self.height):
            logging.warning(f"Image size {image.size} does not match display size ({self.width}, {self.height})")
            image = image.resize((self.width, self.height))
        
        # Converte para RGB565
        image_rgb = image.convert('RGB')
        pixel_bytes = self.image_to_data(image_rgb)
        
        self.set_window(0, 0, self.width - 1, self.height - 1)
        
        # Envia em chunks para não estourar buffers SPI (4096 bytes é seguro)
        chunk_size = 4096
        lgpio.gpio_write(self._h, self._dc, 1) # Garante modo dados
        
        for i in range(0, len(pixel_bytes), chunk_size):
            self._spi.writebytes(pixel_bytes[i:i+chunk_size])

    def image_to_data(self, image):
        """Converte imagem PIL RGB para array de bytes RGB565."""
        pixels = list(image.getdata())
        buffer = []
        for r, g, b in pixels:
            # RGB888 -> RGB565
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            buffer.append(rgb565 >> 8)
            buffer.append(rgb565 & 0xFF)
        return buffer

    def close(self):
        """Limpa GPIO e SPI."""
        try:
            self._spi.close()
            lgpio.gpiochip_close(self._h)
        except:
            pass
