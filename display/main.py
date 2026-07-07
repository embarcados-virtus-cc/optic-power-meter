import os
import sys
import threading
import time

# Ensure display/ directory is on the path when invoked from elsewhere
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import Adafruit_GPIO.SPI as SPI
from ST7789 import ST7789

from config import (
    BLK_PIN, DC_PIN, RST_PIN, SPI_DEVICE, SPI_PORT, SPI_SPEED_HZ,
    I2C_TTL, NET_TTL, SFP_TTL, SVC_TTL, SYS_TTL, WIFI_TTL,
)
from hardware import LGPIOAdapter
from keyboard import KeyboardHandler
from menu_system import MenuSystem


def main():
    print("Iniciando Display Interativo...")
    gpio = None
    kbd  = None
    disp = None
    try:
        gpio = LGPIOAdapter()
        kbd  = KeyboardHandler()
        spi  = SPI.SpiDev(SPI_PORT, SPI_DEVICE, max_speed_hz=SPI_SPEED_HZ)
        disp = ST7789(spi, mode=3, rst=RST_PIN, dc=DC_PIN, led=BLK_PIN, gpio=gpio)
        disp.begin()
        disp.clear()
        gpio.set_high(BLK_PIN)

        menu = MenuSystem(disp.width, disp.height)
        menu.keyboard_warning = not kbd.has_keyboard
        disp.display(menu.render())

        while True:
            key_data     = kbd.get_key()
            needs_update = key_data[0] is not None

            if needs_update:
                menu.handle_input(key_data)

            menu.keyboard_warning = not kbd.has_keyboard

            now = time.time()

            if menu.state in ("SFP_VIEW", "SFP_ALARMS"):
                if not menu._sfp_updating and (now - menu.last_update) > SFP_TTL:
                    threading.Thread(target=menu._update_sfp, daemon=True).start()
                needs_update = True

            elif menu.state == "NET_DEBUG":
                if (now - menu._net_updated) > NET_TTL:
                    menu._trigger_net_update()
                needs_update = True

            elif menu.state == "SYS_DIAG":
                if (now - menu._sys_updated) > SYS_TTL:
                    menu._trigger_sys_update()
                needs_update = True

            elif menu.state == "I2C_SCAN":
                if (now - menu._i2c_updated) > I2C_TTL:
                    menu._trigger_i2c_update()
                needs_update = True

            elif menu.state == "WIFI_SCAN":
                if (now - menu._wifi_updated) > WIFI_TTL:
                    menu._trigger_wifi_update()
                needs_update = True

            elif menu.state == "SERVICES":
                if (now - menu._svc_updated) > SVC_TTL:
                    menu._trigger_svc_update()
                needs_update = True

            elif menu.state in ("LOADING", "SHUTTING_DOWN", "BOOT"):
                needs_update = True

            if time.time() < menu.status_timer:
                needs_update = True

            if needs_update:
                disp.display(menu.render())

            time.sleep(0.05)

    except KeyboardInterrupt:
        print("Encerrando...")
    except Exception as e:
        print(f"Erro Fatal: {e}")
    finally:
        if disp:
            disp.clear()
        if gpio:
            gpio.cleanup()
        if kbd:
            kbd.stop()


if __name__ == "__main__":
    main()
