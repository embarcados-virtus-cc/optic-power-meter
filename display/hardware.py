import lgpio
import Adafruit_GPIO as GPIO


class LGPIOAdapter:
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
        except Exception:
            pass
