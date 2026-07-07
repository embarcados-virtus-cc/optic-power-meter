import time

# ── Hardware pins ──────────────────────────────────────────────────────────────
DC_PIN       = 25
RST_PIN      = 27
BLK_PIN      = 24
SPI_PORT     = 0
SPI_DEVICE   = 0
SPI_SPEED_HZ = 40_000_000

# ── Color palette ──────────────────────────────────────────────────────────────
COLOR_BG        = (30, 30, 46)
COLOR_TEXT      = (255, 255, 255)
COLOR_HIGHLIGHT = (6, 182, 212)
COLOR_ACCENT    = (148, 163, 184)
COLOR_ERROR     = (248, 113, 113)
COLOR_GOOD      = (74, 222, 128)
COLOR_WARN      = (251, 191, 36)

# ── Boot splash duration (seconds) ────────────────────────────────────────────
BOOT_DURATION = 3.2

# ── Auto-refresh TTLs (seconds) ────────────────────────────────────────────────
SFP_TTL  = 2.0
NET_TTL  = 5.0
SYS_TTL  = 3.0
I2C_TTL  = 10.0
WIFI_TTL = 30.0
SVC_TTL  = 5.0

# ── I2C known device descriptions ─────────────────────────────────────────────
I2C_KNOWN = {
    "50": "SFP A0h",
    "51": "SFP A2h",
    "3c": "OLED/SSD1306",
    "68": "RTC/MPU6050",
    "76": "BME280",
    "77": "BME280",
    "48": "ADS1115",
}

# ── Project info (shown on About screen) ─────────────────────────────────────
PROJECT_INFO = {
    "name":    "SFP Optic Power Meter",
    "version": "v1.0",
    "org":     "Virtus CC – Embarcados",
    "devs":    [
        {"name": "Pedro Sousa", "github": "pwsousa"},
        # Adicione mais desenvolvedores: {"name": "Nome", "github": "username"},
    ],
    "email":   "pwsousa2003@gmail.com",
    "repo":    "github.com/embarcados-virtus-cc",
    "hw":      "Raspberry Pi + SFP+",
    "license": "MIT",
}

# ── Services to monitor: (id, label, type)
# type "systemd" → systemctl is-active
# type "docker"  → docker inspect container status
MONITORED_SERVICES = [
    ("sfp-daemon",  "SFP Daemon", "systemd"),
    ("optic-gui",   "Nginx / GUI", "docker"),
    ("optic-mongo", "MongoDB",     "docker"),
    ("optic-api",   "API",         "docker"),
]


# ── Helpers ───────────────────────────────────────────────────────────────────

def fmt_uptime(seconds: float) -> str:
    s = int(seconds)
    d, s = divmod(s, 86400)
    h, s = divmod(s, 3600)
    m, _ = divmod(s, 60)
    if d:
        return f"{d}d {h}h {m}m"
    if h:
        return f"{h}h {m}m"
    return f"{m}m"


def rx_power_color(dbm_str) -> tuple:
    try:
        v = float(str(dbm_str))
        if v > -3:
            return COLOR_WARN
        if v >= -20:
            return COLOR_GOOD
        if v >= -28:
            return COLOR_ACCENT
        return COLOR_ERROR
    except (ValueError, TypeError):
        return COLOR_TEXT


def age_str(ts: float) -> str:
    if ts == 0:
        return "..."
    age = int(time.time() - ts)
    return f"{age}s atras" if age < 60 else ">1min"
