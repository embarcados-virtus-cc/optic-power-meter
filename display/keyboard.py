import threading
import time

try:
    from evdev import InputDevice, list_devices, ecodes, categorize
except ImportError:
    print("Erro: evdev não encontrado. Instale com: pip install evdev")


class KeyboardHandler:
    def __init__(self):
        self.devices = []
        self._device_paths = set()
        self.queue = []
        self.lock = threading.Lock()
        self._stop = False
        self.shift_pressed = False
        self._find_keyboards()
        threading.Thread(target=self._rescan_loop, daemon=True).start()

    def _find_keyboards(self):
        try:
            all_devices = [InputDevice(path) for path in list_devices()]
            for device in all_devices:
                if device.path in self._device_paths:
                    continue
                low = device.name.lower()
                if any(x in low for x in ["keyboard", "logitech", "usb", "hid"]):
                    if "pwr_button" not in low:
                        self.devices.append(device)
                        self._device_paths.add(device.path)
                        threading.Thread(target=self._run, args=(device,), daemon=True).start()
        except Exception as e:
            print(f"Erro ao listar dispositivos: {e}")

    def _rescan_loop(self):
        while not self._stop:
            time.sleep(3)
            self._find_keyboards()

    def _run(self, device):
        try:
            for event in device.read_loop():
                if self._stop:
                    break
                if event.type == ecodes.EV_KEY:
                    kev = categorize(event)
                    if kev.scancode in [42, 54]:
                        self.shift_pressed = kev.keystate in [kev.key_down, kev.key_hold]
                    if kev.keystate in [kev.key_down, kev.key_hold]:
                        with self.lock:
                            self.queue.append((kev.scancode, self.shift_pressed))
        except Exception as e:
            print(f"Dispositivo {device.path} desconectado: {e}")
        finally:
            with self.lock:
                if device in self.devices:
                    self.devices.remove(device)
                self._device_paths.discard(device.path)

    def get_key(self):
        with self.lock:
            if self.queue:
                return self.queue.pop(0)
        return None, False

    @property
    def has_keyboard(self) -> bool:
        return len(self.devices) > 0

    def stop(self):
        self._stop = True
