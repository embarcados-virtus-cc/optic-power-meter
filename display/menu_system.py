import math
import os
import subprocess
import threading
import time

from PIL import Image, ImageDraw, ImageFont

from config import (
    BOOT_DURATION,
    COLOR_ACCENT, COLOR_BG, COLOR_ERROR, COLOR_GOOD, COLOR_HIGHLIGHT,
    COLOR_TEXT, COLOR_WARN, I2C_KNOWN, I2C_TTL, NET_TTL, PROJECT_INFO,
    SFP_TTL, SVC_TTL, SYS_TTL, WIFI_TTL, age_str, rx_power_color,
)
from diagnostics import DeviceDiagnostic, ServiceMonitor, compute_sfp_alarms
from network import NetworkManager
from sfp_reader import SFPReader

# ── Constants ──────────────────────────────────────────────────────────────────
_MENU_ITEMS = [
    "Leitura SFP",
    "Alertas SFP",
    "Servicos",
    "Rede & Debug",
    "Scan I2C",
    "Sistema",
    "Config WiFi",
    "Sobre",
    "Reiniciar",
    "Desligar",
]
_MENU_COUNT   = len(_MENU_ITEMS)
_MENU_MAX_VIS = 6

# (2-char label, background color) for each menu item's icon box
_MENU_ICONS = [
    ("SF", (6,  182, 212)),   # Leitura SFP    – cyan
    ("AL", (251, 191, 36)),   # Alertas SFP    – yellow
    ("SV", (74,  222, 128)),  # Status Servicos – green
    ("NT", (99,  179, 237)),  # Rede & Debug   – blue
    ("I2", (167, 139, 250)),  # Scan I2C       – purple
    ("SY", (148, 163, 184)),  # Sistema & Boot – gray
    ("WF", (251, 191, 36)),   # Config WiFi    – yellow
    ("iN", (148, 163, 184)),  # Sobre          – gray
    ("RS", (251, 191, 36)),   # Reiniciar      – yellow
    ("OF", (248, 113, 113)),  # Desligar       – red
]

_ABOUT_MAX_VIS = 9   # visible rows in About screen

# Material Icons codepoints (one per menu item, mirrors _MENU_ICONS order)
_MENU_ICON_CHARS = [
    "",  # equalizer          – Leitura SFP
    "",  # warning            – Alertas SFP
    "",  # settings           – Status Servicos
    "",  # router             – Rede & Debug
    "",  # memory             – Scan I2C
    "",  # computer           – Sistema & Boot
    "",  # wifi               – Configurar WiFi
    "",  # info               – Sobre o Projeto
    "",  # refresh            – Reiniciar
    "",  # power_settings_new – Desligar
]
_SFP_ITEMS_COUNT = 13
_NET_ITEMS_COUNT = 8
_NET_MAX_VIS     = 6
_NET_MAX_SCROLL  = _NET_ITEMS_COUNT - _NET_MAX_VIS   # = 2


class MenuSystem:
    def __init__(self, width: int, height: int):
        self.width  = width
        self.height = height

        # ── UI state ──────────────────────────────────────────────────────────
        self.state          = "BOOT"
        self.selected_index = 0        # reused per state
        self._loading_msg   = "Aguarde..."
        self._net_scroll         = 0
        self._alarm_scroll       = 0
        self._about_scroll       = 0
        self._about_items_count  = 0

        # ── WiFi state ────────────────────────────────────────────────────────
        self.wifi_list      = []
        self.known_networks = []
        self._wifi_details: dict = {}
        self._active_ssid:  str  = ""
        self.password       = ""
        self.show_password  = False

        # ── SFP ───────────────────────────────────────────────────────────────
        self.sfp_data    = None
        self.last_update = 0.0
        self._sfp_updating = False
        self._gen_id     = None

        # ── I2C ───────────────────────────────────────────────────────────────
        self.i2c_devices   = []
        self._i2c_updated  = 0.0
        self._i2c_updating = False

        # ── Network ───────────────────────────────────────────────────────────
        self._net_cache   = {}
        self._net_updated = 0.0
        self._net_updating = False

        # ── System ────────────────────────────────────────────────────────────
        self._sys_cache       = {}
        self._sys_updated     = 0.0
        self._sys_updating    = False
        self._sys_scroll      = 0
        self._sys_items_count = 0

        # ── WiFi scan ─────────────────────────────────────────────────────────
        self._wifi_updated  = 0.0
        self._wifi_updating = False

        # ── Services ──────────────────────────────────────────────────────────
        self._svc_cache   = {}
        self._svc_updated = 0.0
        self._svc_updating = False

        # ── Boot animation ────────────────────────────────────────────────────
        self._boot_start = time.time()

        # ── Shutdown animation ────────────────────────────────────────────────
        self._shutdown_start = 0.0
        self._shutdown_mode  = "poweroff"

        # ── Status bar ────────────────────────────────────────────────────────
        self.status_msg      = ""
        self.status_timer    = 0.0
        self.keyboard_warning = False

        # ── Fonts ─────────────────────────────────────────────────────────────
        try:
            fd = "/usr/share/fonts/truetype/dejavu"
            self.font_title = ImageFont.truetype(f"{fd}/DejaVuSans-Bold.ttf", 26)
            self.font_text  = ImageFont.truetype(f"{fd}/DejaVuSans.ttf", 18)
            self.font_small = ImageFont.truetype(f"{fd}/DejaVuSans.ttf", 14)
            self.font_tiny  = ImageFont.truetype(f"{fd}/DejaVuSans.ttf", 12)
        except IOError:
            fb = ImageFont.load_default()
            self.font_title = self.font_text = self.font_small = self.font_tiny = fb

        # ── Material Icons font ──────────────────────────────────────────────────
        self.font_icon    = None
        self.font_icon_sm = None
        try:
            mi_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets", "MaterialIcons-Regular.ttf")
            self.font_icon    = ImageFont.truetype(mi_path, 24)
            self.font_icon_sm = ImageFont.truetype(mi_path, 16)
        except IOError:
            pass

        # ── Logo ──────────────────────────────────────────────────────────────
        self.logo      = None
        self.logo_h    = 0
        self.logo_boot = None
        try:
            path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets", "virtus-cc.png")
            if os.path.exists(path):
                img = Image.open(path).convert("RGBA")
                new_h = min(int(self.width * img.height / img.width), 55)
                self.logo   = img.resize((self.width, new_h), Image.LANCZOS)
                self.logo_h = new_h
                boot_w = 210
                boot_h = min(int(boot_w * img.height / img.width), 45)
                self.logo_boot = img.resize((boot_w, boot_h), Image.LANCZOS)
        except Exception:
            pass

    # ══════════════════════════════════════════════════════════════════════════
    # Background cache helpers
    # ══════════════════════════════════════════════════════════════════════════

    def _trigger_net_update(self):
        if self._net_updating:
            return
        self._net_updating = True
        threading.Thread(target=self._fetch_net, daemon=True).start()

    def _fetch_net(self):
        try:
            data = NetworkManager.get_detailed_info()
            data["online"] = NetworkManager.test_connectivity()
            self._net_cache   = data
            self._net_updated = time.time()
        except Exception as e:
            print(f"Net cache erro: {e}")
        finally:
            self._net_updating = False

    def _trigger_sys_update(self):
        if self._sys_updating:
            return
        self._sys_updating = True
        threading.Thread(target=self._fetch_sys, daemon=True).start()

    def _fetch_sys(self):
        try:
            data = DeviceDiagnostic.get_system_stats()
            data["boot"]  = DeviceDiagnostic.get_boot_info()
            self._sys_cache   = data
            self._sys_updated = time.time()
        except Exception as e:
            print(f"Sys cache erro: {e}")
        finally:
            self._sys_updating = False

    def _trigger_i2c_update(self):
        if self._i2c_updating:
            return
        self._i2c_updating = True
        threading.Thread(target=self._fetch_i2c, daemon=True).start()

    def _fetch_i2c(self):
        try:
            self.i2c_devices  = DeviceDiagnostic.scan_i2c()
            self._i2c_updated = time.time()
        except Exception:
            self.set_status("Erro Scan I2C")
        finally:
            self._i2c_updating = False
            if self.state == "LOADING":
                self.state = "I2C_SCAN"

    def _trigger_wifi_update(self):
        if self._wifi_updating:
            return
        self._wifi_updating = True
        threading.Thread(target=self._fetch_wifi, daemon=True).start()

    def _fetch_wifi(self):
        try:
            self.known_networks  = NetworkManager.get_known_networks()
            self.wifi_list       = NetworkManager.scan_wifi()
            self._wifi_details   = NetworkManager.get_wifi_security()
            self._active_ssid    = NetworkManager.get_active_ssid()
            self._wifi_updated   = time.time()
        except Exception:
            self.set_status("Erro Scan WiFi")
        finally:
            self._wifi_updating = False
            if self.state == "LOADING":
                self.state = "WIFI_SCAN"

    def _trigger_svc_update(self):
        if self._svc_updating:
            return
        self._svc_updating = True
        threading.Thread(target=self._fetch_svc, daemon=True).start()

    def _fetch_svc(self):
        try:
            self._svc_cache   = ServiceMonitor.get_status()
            self._svc_updated = time.time()
        except Exception as e:
            print(f"Svc cache erro: {e}")
        finally:
            self._svc_updating = False

    def _update_sfp(self, next_state: str = "SFP_VIEW"):
        self._sfp_updating = True
        try:
            prev = self.sfp_data
            data = SFPReader.get_data()
            self.sfp_data = data
            self.last_update = time.time()
            if data is None:
                if prev is not None:
                    self.set_status("Módulo SFP removido")
                    self._gen_id = None
            else:
                new_gen = data.get("generation_id")
                if new_gen is not None and self._gen_id is not None and new_gen != self._gen_id:
                    self.set_status("Módulo SFP trocado")
                    self._alarm_scroll  = 0
                    self.selected_index = 0
                self._gen_id = new_gen
        except Exception:
            self.sfp_data = None
            self.set_status("Erro Leitura SFP")
        finally:
            self._sfp_updating = False
            if self.state == "LOADING":
                self.state = next_state

    def _connect_task(self, ssid: str, password: str):
        try:
            success, msg = NetworkManager.connect_wifi(ssid, password)
            self.set_status(msg)
            if success:
                self.state = "NET_DEBUG"
                self._trigger_net_update()
            else:
                time.sleep(1.5)
                self.state = "WIFI_SCAN"
        except Exception:
            self.set_status("Erro Interno")
            self.state = "WIFI_SCAN"

    def _connect_known_task(self, ssid: str):
        try:
            success, msg = NetworkManager.connect_known(ssid)
            self.set_status(msg)
            if success:
                self.state = "NET_DEBUG"
                self._trigger_net_update()
            else:
                # Saved password may be wrong — ask for a new one
                time.sleep(1.5)
                self.password      = ""
                self.show_password = False
                self.state         = "WIFI_INPUT"
        except Exception:
            self.set_status("Erro Conexão")
            self.state = "WIFI_SCAN"

    def _shutdown_task(self):
        time.sleep(2.8)
        if self._shutdown_mode == "reboot":
            subprocess.run(["sudo", "reboot"], check=False)
        else:
            subprocess.run(["sudo", "poweroff"], check=False)

    # ══════════════════════════════════════════════════════════════════════════
    # UI helpers
    # ══════════════════════════════════════════════════════════════════════════

    def set_status(self, msg: str, duration: float = 3):
        self.status_msg   = msg
        self.status_timer = time.time() + duration

    def _draw_header(self, image, draw):
        if self.logo:
            image.paste(self.logo, (0, 0), self.logo)
            draw.line((0, self.logo_h, self.width, self.logo_h), fill=COLOR_ACCENT, width=1)
            return self.logo_h + 5
        return 70

    def _draw_scroll_arrows(self, draw, y_top, y_bottom, can_up, can_down):
        x = self.width - 10
        if can_up:
            draw.polygon([(x-5, y_top+8), (x+5, y_top+8), (x, y_top)], fill=COLOR_ACCENT)
        if can_down:
            draw.polygon([(x-5, y_bottom-8), (x+5, y_bottom-8), (x, y_bottom)], fill=COLOR_ACCENT)

    def _draw_footer(self, draw, text: str, footer_y: int):
        draw.text((10, footer_y), text, font=self.font_tiny, fill=COLOR_ACCENT)

    def _centered_text(self, draw, text: str, cy: int, font, fill):
        tw = draw.textlength(text, font=font)
        draw.text((self.width // 2 - tw // 2, cy), text, font=font, fill=fill)

    def _draw_signal_bars(self, draw, x: int, y_bottom: int, pct: int):
        pct = max(0, min(100, pct))
        levels = 1 if pct <= 25 else (2 if pct <= 50 else (3 if pct <= 75 else 4))
        active = COLOR_GOOD if pct >= 61 else (COLOR_WARN if pct >= 31 else COLOR_ERROR)
        dim    = (55, 55, 72)
        w, gap = 3, 2
        for i, h in enumerate((5, 8, 11, 14)):
            bx   = x + i * (w + gap)
            fill = active if i < levels else dim
            draw.rectangle((bx, y_bottom - h, bx + w, y_bottom), fill=fill)

    def _status_color(self, ok):
        if ok is True:
            return COLOR_GOOD
        if ok is False:
            return COLOR_ERROR
        return COLOR_ACCENT

    def _draw_sfp_disconnected(self, draw, y):
        cy = y + 18
        if self.font_icon:
            mi = _MENU_ICON_CHARS[0]  # equalizer (SFP icon)
            tw = draw.textlength(mi, font=self.font_icon)
            draw.text((self.width // 2 - int(tw) // 2, cy),
                      mi, font=self.font_icon, fill=(80, 40, 40))
            cy += 32
        self._centered_text(draw, "Dispositivo não conectado", cy,      self.font_small, COLOR_ERROR)
        self._centered_text(draw, "Verifique o módulo SFP",   cy + 20,  self.font_tiny,  COLOR_ACCENT)
        self._centered_text(draw, "ou o sfp-daemon",          cy + 36,  self.font_tiny,  (80, 80, 100))

    # ══════════════════════════════════════════════════════════════════════════
    # Render
    # ══════════════════════════════════════════════════════════════════════════

    def render(self):
        image = Image.new("RGB", (self.width, self.height), COLOR_BG)
        draw  = ImageDraw.Draw(image)
        fy    = self.height - 22   # footer y

        # ── BOOT SPLASH ──────────────────────────────────────────────────────
        if self.state == "BOOT":
            elapsed  = time.time() - self._boot_start
            duration = BOOT_DURATION
            progress = min(1.0, elapsed / duration)

            # Background: black → COLOR_BG over first 0.5s
            fi = min(1.0, elapsed / 0.5)
            draw.rectangle((0, 0, self.width, self.height),
                            fill=(int(COLOR_BG[0]*fi), int(COLOR_BG[1]*fi), int(COLOR_BG[2]*fi)))

            cx = self.width // 2

            # Logo (centered, 210px wide)
            if elapsed >= 0.2 and self.logo_boot:
                lx = (self.width - self.logo_boot.width) // 2
                image.paste(self.logo_boot, (lx, 22), self.logo_boot)

            # Subtitle lines
            if elapsed >= 0.5:
                ta = min(1.0, (elapsed - 0.5) / 0.4)
                tc = (int(255*ta), int(255*ta), int(255*ta))
                ac = (int(COLOR_ACCENT[0]*ta), int(COLOR_ACCENT[1]*ta), int(COLOR_ACCENT[2]*ta))
                self._centered_text(draw, PROJECT_INFO["name"], 80, self.font_text, tc)
                self._centered_text(draw, PROJECT_INFO["org"],  103, self.font_tiny, ac)
                draw.line((cx - 60, 120, cx + 60, 120), fill=(50, 50, 72), width=1)

            # Progress bar + label
            if elapsed >= 0.8:
                bar_prog = min(1.0, (elapsed - 0.8) / (duration - 1.0))
                bx, bw, by_, bh = 50, self.width - 100, 148, 5
                draw.rectangle((bx, by_, bx + bw, by_ + bh), fill=(40, 40, 60))
                fw = int(bw * bar_prog)
                if fw > 0:
                    # Gradient: cyan → green
                    r1, g1, b1 = COLOR_HIGHLIGHT          # (6,182,212)
                    r2, g2, b2 = COLOR_GOOD               # (74,222,128)
                    t = bar_prog
                    gc = (int(r1 + (r2-r1)*t), int(g1 + (g2-g1)*t), int(b1 + (b2-b1)*t))
                    draw.rectangle((bx, by_, bx + fw, by_ + bh), fill=gc)
                pct_txt = f"{int(bar_prog * 100)}%"
                tw_p = draw.textlength(pct_txt, font=self.font_tiny)
                draw.text((cx - tw_p // 2, by_ + bh + 6), pct_txt,
                          font=self.font_tiny, fill=COLOR_ACCENT)

            # Version bottom-right
            if elapsed >= 0.5:
                tw_v = draw.textlength(PROJECT_INFO["version"], font=self.font_tiny)
                draw.text((self.width - tw_v - 8, self.height - 16),
                          PROJECT_INFO["version"], font=self.font_tiny, fill=(55, 55, 72))

            if progress < 1.0:
                return image
            self.state = "MAIN_MENU"
            image = Image.new("RGB", (self.width, self.height), COLOR_BG)
            draw  = ImageDraw.Draw(image)

        y = self._draw_header(image, draw)

        # ── MAIN MENU ────────────────────────────────────────────────────────
        if self.state == "MAIN_MENU":
            # 2-column icon grid
            tile_w, tile_h = 140, 48
            col_gap, row_gap = 10, 2
            n_vis_rows = 3
            max_rows   = (_MENU_COUNT + 1) // 2   # = 5
            left_x     = (self.width - 2 * tile_w - col_gap) // 2  # = 15px margin

            sel_row    = self.selected_index // 2
            start_row  = max(0, min(sel_row, max_rows - n_vis_rows))

            for vi in range(n_vis_rows):
                row = start_row + vi
                for col in range(2):
                    idx = row * 2 + col
                    if idx >= _MENU_COUNT:
                        break
                    is_sel              = idx == self.selected_index
                    icon_char, icon_bg  = _MENU_ICONS[idx]
                    mi_char             = _MENU_ICON_CHARS[idx]
                    label               = _MENU_ITEMS[idx]

                    tx = left_x + col * (tile_w + col_gap)
                    ty = y + vi * (tile_h + row_gap)

                    # Tile background
                    if is_sel:
                        draw.rectangle((tx, ty, tx + tile_w, ty + tile_h), fill=icon_bg)
                        # Subtle inner border
                        draw.rectangle((tx+1, ty+1, tx+tile_w-1, ty+tile_h-1), outline=(255,255,255,40), width=1)
                    else:
                        draw.rectangle((tx, ty, tx + tile_w, ty + tile_h), fill=(38, 38, 54))
                        # Dim left accent
                        draw.rectangle((tx, ty, tx + 3, ty + tile_h), fill=icon_bg)

                    # Material Icon (centered in upper 2/3 of tile)
                    if self.font_icon:
                        ic_color = (20, 20, 32) if is_sel else icon_bg
                        tw_ic = draw.textlength(mi_char, font=self.font_icon)
                        draw.text((tx + (tile_w - tw_ic) // 2, ty + 5),
                                  mi_char, font=self.font_icon, fill=ic_color)
                    else:
                        # Fallback: 2-char text box
                        fc = (20, 20, 32) if is_sel else (0, 0, 0)
                        bx, by_ = tx + tile_w//2 - 14, ty + 5
                        draw.rectangle((bx, by_, bx+28, by_+22), fill=icon_bg if not is_sel else (20,20,32))
                        tw_fb = draw.textlength(icon_char, font=self.font_tiny)
                        draw.text((bx + (28-tw_fb)//2, by_+4), icon_char, font=self.font_tiny, fill=(240,246,252))

                    # Label (centered, bottom of tile)
                    label_color = (20, 20, 32) if is_sel else (
                        (220, 100, 100) if idx == 9 else COLOR_TEXT
                    )
                    tw_lbl = draw.textlength(label, font=self.font_tiny)
                    # Truncate if label too wide for tile
                    while tw_lbl > tile_w - 8 and len(label) > 4:
                        label = label[:-1]
                        tw_lbl = draw.textlength(label + "..", font=self.font_tiny)
                    draw.text((tx + (tile_w - tw_lbl) // 2, ty + tile_h - 14),
                              label, font=self.font_tiny, fill=label_color)

            # Pagination dots (bottom-center)
            total_pages = max(1, max_rows - n_vis_rows + 1)
            if total_pages > 1:
                dot_y  = y + n_vis_rows * (tile_h + row_gap) + 2
                dot_cx = self.width // 2 - (total_pages * 10) // 2
                for p in range(total_pages):
                    dx    = dot_cx + p * 10
                    dfill = COLOR_HIGHLIGHT if p == start_row else (55, 55, 72)
                    draw.ellipse((dx, dot_y, dx + 6, dot_y + 6), fill=dfill)

        # ── SFP VIEW ─────────────────────────────────────────────────────────
        elif self.state == "SFP_VIEW":
            draw.text((10, y), "Leitura SFP", font=self.font_small, fill=COLOR_HIGHLIGHT)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y_list = y + 22
            sfp = self.sfp_data
            if sfp:
                a2, a0 = sfp.get("a2", {}), sfp.get("a0", {})
                _rx_raw = a2.get("rx_power_dbm")
                rx_dbm = round(_rx_raw, 2) if isinstance(_rx_raw, (int, float)) else "N/A"
                try:
                    temp_v    = float(str(a2.get("temperature_c", "0")).replace("°C", ""))
                    temp_color = COLOR_ERROR if temp_v > 70 else (COLOR_WARN if temp_v > 60 else COLOR_TEXT)
                except (ValueError, TypeError):
                    temp_color = COLOR_TEXT
                items = [
                    # (label, value, color, is_header)
                    ("── Optica ────────────────────", "", COLOR_HIGHLIGHT, True),
                    ("RX Power", f"{rx_dbm} dBm",                             rx_power_color(rx_dbm), False),
                    ("TX Bias",  f"{a2.get('tx_bias_ma','N/A')} mA",           COLOR_HIGHLIGHT,        False),
                    ("TX Power", f"{float(a2['tx_power_dbm']):.3f} dBm" if a2.get('tx_power_dbm') is not None else "N/A dBm", COLOR_HIGHLIGHT, False),
                    ("Temp",     f"{a2.get('temperature_c','N/A')} °C",         temp_color,             False),
                    ("Tensao",   f"{float(a2['voltage_v']):.3f} V" if a2.get('voltage_v') is not None else "N/A V", COLOR_TEXT, False),
                    ("── Identificacao ────────────", "", COLOR_HIGHLIGHT, True),
                    ("Fabr",    a0.get("vendor_name","N/A")[:20],               COLOR_ACCENT,           False),
                    ("P/N",     a0.get("vendor_pn","N/A")[:20],                 COLOR_ACCENT,           False),
                    ("S/N",     a0.get("vendor_sn","N/A")[:20],                 COLOR_ACCENT,           False),
                    ("λ",       f"{a0.get('wavelength_nm','N/A')} nm",          COLOR_TEXT,             False),
                    ("Conect",  a0.get("connector_type","N/A"),                  COLOR_TEXT,             False),
                    ("Tipo",    a0.get("identifier_type","N/A"),                 COLOR_TEXT,             False),
                ]
                n, max_vis, row_h = len(items), 6, 22
                start = max(0, self.selected_index - max_vis + 1) if self.selected_index >= max_vis else 0
                for i in range(max_vis):
                    idx = start + i
                    if idx >= n:
                        break
                    label, value, color, is_hdr = items[idx]
                    ry = y_list + i * row_h
                    if is_hdr:
                        draw.text((10, ry + 4), label, font=self.font_tiny, fill=COLOR_HIGHLIGHT)
                    else:
                        draw.text((10, ry + 4), f"{label}:", font=self.font_tiny, fill=COLOR_ACCENT)
                        draw.text((78, ry + 3), value, font=self.font_small, fill=color)
                self._draw_scroll_arrows(draw, y_list, y_list + max_vis * row_h, start > 0, start + max_vis < n)
                draw.text((10, fy - 14), f"Atualizado: {age_str(self.last_update)}", font=self.font_tiny, fill=COLOR_ACCENT)
            else:
                self._draw_sfp_disconnected(draw, y_list)
            self._draw_footer(draw, "ESC: Voltar  ▲▼: rolar", fy)

        # ── SFP ALARMS ───────────────────────────────────────────────────────
        elif self.state == "SFP_ALARMS":
            draw.text((10, y), "Alertas SFP", font=self.font_small, fill=COLOR_HIGHLIGHT)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y_list = y + 22
            sfp = self.sfp_data
            if not sfp:
                self._draw_sfp_disconnected(draw, y_list)
                self._draw_footer(draw, "ESC: Voltar", fy)
            else:
                alarms    = compute_sfp_alarms(sfp)
                n_fail    = sum(1 for a in alarms if a["ok"] is False)
                hdr_color = COLOR_ERROR if n_fail else COLOR_GOOD
                hdr_txt   = f"{n_fail} falha(s)" if n_fail else "Tudo OK"
                tw_hdr    = draw.textlength(hdr_txt, font=self.font_small)
                draw.text((self.width - tw_hdr - 8, y), hdr_txt,
                          font=self.font_small, fill=hdr_color)
                n, max_vis, row_h = len(alarms), 5, 27
                start = self._alarm_scroll
                for i in range(max_vis):
                    idx = start + i
                    if idx >= n:
                        break
                    a     = alarms[idx]
                    color = self._status_color(a["ok"])
                    ry    = y_list + i * row_h
                    if a["ok"] is False:
                        draw.rectangle((0, ry - 1, self.width, ry + row_h - 2), fill=(55, 25, 25))
                    draw.text((10, ry + 5), a["name"], font=self.font_small, fill=COLOR_TEXT)
                    draw.text((105, ry + 5), a["value"], font=self.font_small, fill=color)
                    st = a["status"]
                    tw = draw.textlength(st, font=self.font_tiny)
                    draw.text((self.width - tw - 6, ry + 7), st, font=self.font_tiny, fill=color)
                self._draw_scroll_arrows(draw, y_list, y_list + max_vis * row_h,
                                         start > 0, start + max_vis < n)
                draw.text((10, fy - 14), f"Atualizado: {age_str(self.last_update)}",
                          font=self.font_tiny, fill=COLOR_ACCENT)
                self._draw_footer(draw, "ESC: Voltar  ▲▼: rolar", fy)

        # ── SERVICES ─────────────────────────────────────────────────────────
        elif self.state == "SERVICES":
            n_active = sum(1 for v in self._svc_cache.values() if v["active"]) if self._svc_cache else 0
            n_total  = len(self._svc_cache) if self._svc_cache else 0
            spin_char = "|/-\\"[int(time.time() * 3) % 4] if self._svc_updating else ""
            draw.text((10, y), f"Servicos  {n_active}/{n_total} ativos", font=self.font_small, fill=COLOR_HIGHLIGHT)
            if spin_char:
                draw.text((self.width - 18, y), spin_char, font=self.font_small, fill=COLOR_WARN)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y_list = y + 22
            if not self._svc_cache:
                self._centered_text(draw, "Verificando...", y_list + 20, self.font_text, COLOR_ACCENT)
            else:
                row_h = 32
                for i, (svc_name, info) in enumerate(self._svc_cache.items()):
                    color  = COLOR_GOOD if info["active"] else COLOR_ERROR
                    status = "ATIVO" if info["active"] else info["status"].upper()
                    ry     = y_list + i * row_h
                    if not info["active"]:
                        draw.rectangle((0, ry - 1, self.width, ry + row_h - 2), fill=(50, 22, 22))
                    dot_cy = ry + row_h // 2
                    draw.ellipse((10, dot_cy - 5, 20, dot_cy + 5), fill=color)
                    draw.text((28, ry + 8), info["label"], font=self.font_small, fill=COLOR_TEXT)
                    tw = draw.textlength(status, font=self.font_tiny)
                    draw.text((self.width - tw - 8, ry + 10), status, font=self.font_tiny, fill=color)
            draw.text((10, fy - 14), f"Atualizado: {age_str(self._svc_updated)}", font=self.font_tiny, fill=COLOR_ACCENT)
            self._draw_footer(draw, "ESC: Voltar", fy)

        # ── NET DEBUG ────────────────────────────────────────────────────────
        elif self.state == "NET_DEBUG":
            net = self._net_cache
            online = net.get("online") if net else None
            if online is True:
                stat_txt, stat_col = "ONLINE", COLOR_GOOD
            elif online is False:
                stat_txt, stat_col = "OFFLINE", COLOR_ERROR
            else:
                stat_txt, stat_col = "...", COLOR_ACCENT
            draw.text((10, y), "Rede & Debug", font=self.font_small, fill=COLOR_HIGHLIGHT)
            tw = draw.textlength(stat_txt, font=self.font_small)
            draw.text((self.width - tw - 8, y), stat_txt, font=self.font_small, fill=stat_col)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y_list = y + 22
            if not net:
                self._centered_text(draw, "Carregando...", y_list + 20, self.font_text, COLOR_ACCENT)
            else:
                ip = net.get("ip", "N/A")
                net_items = [
                    ("IP",   ip,                                                COLOR_TEXT),
                    ("GW",   net.get("gateway", "N/A"),                         COLOR_ACCENT),
                    ("DNS",  net.get("dns", "N/A"),                             COLOR_ACCENT),
                    ("WiFi", f"{net.get('ssid','N/A')}  {net.get('signal','')}", COLOR_HIGHLIGHT),
                    ("Web",  "OK" if net.get("online") else "FAIL",             stat_col),
                    ("IO",   net.get("tx_rx", "N/A"),                           COLOR_ACCENT),
                    ("GUI",  f":8080  http://{ip}",                             COLOR_ACCENT),
                    ("API",  f":8001  http://{ip}",                             COLOR_ACCENT),
                ]
                row_h          = 22
                start          = self._net_scroll
                content_bottom = y_list + _NET_MAX_VIS * row_h
                for i in range(_NET_MAX_VIS):
                    idx = start + i
                    if idx >= _NET_ITEMS_COUNT:
                        break
                    label, value, color = net_items[idx]
                    ry = y_list + i * row_h
                    draw.text((10, ry + 2), f"{label}:", font=self.font_tiny, fill=COLOR_ACCENT)
                    draw.text((46, ry + 2), value,        font=self.font_small, fill=color)
                self._draw_scroll_arrows(draw, y_list, content_bottom, start > 0, start + _NET_MAX_VIS < _NET_ITEMS_COUNT)
                draw.text((10, fy - 14), f"Atualizado: {age_str(self._net_updated)}", font=self.font_tiny, fill=COLOR_ACCENT)
            self._draw_footer(draw, "ESC: Voltar  ▲▼: rolar", fy)

        # ── I2C SCAN ─────────────────────────────────────────────────────────
        elif self.state == "I2C_SCAN":
            spin_char = "|/-\\"[int(time.time() * 3) % 4] if self._i2c_updating else ""
            _err_vals = {"Vazio", "Erro Permissao", "I2C Desativado", "Erro Barramento", "Erro Fatal", "i2c-tools nao inst."}
            devs      = [d for d in self.i2c_devices if d not in _err_vals]
            err_msg   = next((d for d in self.i2c_devices if d in _err_vals), None) if self.i2c_devices else None
            hdr_txt   = f"Scan I2C  {len(devs)} disp." if devs else ("Scan I2C" if not err_msg else f"I2C  {err_msg}")
            draw.text((10, y), hdr_txt, font=self.font_small, fill=COLOR_HIGHLIGHT)
            if spin_char:
                draw.text((self.width - 18, y), spin_char, font=self.font_small, fill=COLOR_WARN)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y_list = y + 22
            if not self.i2c_devices:
                self._centered_text(draw, "Aguardando scan...", y_list + 20, self.font_text, COLOR_ACCENT)
            elif not devs:
                msg = err_msg or "Nenhum dispositivo"
                self._centered_text(draw, msg, y_list + 20, self.font_text, COLOR_ACCENT)
            else:
                row_h = 26
                for i, addr in enumerate(devs[:7]):
                    known      = I2C_KNOWN.get(addr.lower(), "")
                    addr_color = COLOR_HIGHLIGHT if known else COLOR_ACCENT
                    ry         = y_list + i * row_h
                    draw.text((10, ry + 4), f"0x{addr.upper()}", font=self.font_small, fill=addr_color)
                    if known:
                        draw.text((72, ry + 6), known, font=self.font_tiny, fill=COLOR_TEXT)
            draw.text((10, fy - 14), f"Atualizado: {age_str(self._i2c_updated)}", font=self.font_tiny, fill=COLOR_ACCENT)
            self._draw_footer(draw, "ESC: Voltar", fy)

        # ── SYS DIAG ─────────────────────────────────────────────────────────
        elif self.state == "SYS_DIAG":
            s = self._sys_cache
            if not s:
                draw.text((10, y), "Carregando...", font=self.font_text, fill=COLOR_ACCENT)
                self._sys_items_count = 0
            else:
                boot = s.get("boot", {})

                try:
                    temp_v    = float(str(s.get("cpu_temp", "0")).replace("°C", ""))
                    cpu_color = COLOR_ERROR if temp_v > 70 else (COLOR_WARN if temp_v > 60 else COLOR_GOOD)
                except ValueError:
                    cpu_color = COLOR_TEXT

                thr       = s.get("throttled", "N/A")
                thr_color = COLOR_WARN if thr == "LIMITADO" else (COLOR_GOOD if thr == "OK" else COLOR_ACCENT)
                i2c_on    = boot.get("i2c_enabled", False)
                ssh_n     = s.get("ssh_sessions", 0)

                sys_items = [
                    # (text, color, use_bold_font)
                    ("── Hardware ─────────────", COLOR_HIGHLIGHT,  True),
                    (f" Modelo:  {s.get('model','N/A')[:23]}",       COLOR_ACCENT,    False),
                    (f" Temp:    {s.get('cpu_temp','N/A')}  CPU: {s.get('cpu_usage','N/A')}", cpu_color, False),
                    (f" Freq:    {s.get('cpu_freq','N/A')}",          COLOR_TEXT,      False),
                    (f" Thrott:  {thr}",                               thr_color,       False),
                    ("── Memoria & Disco ───────", COLOR_HIGHLIGHT,  True),
                    (f" RAM:     {s.get('mem_used_mb',0)}MB / {s.get('mem_total_mb',0)}MB ({s.get('mem_usage','N/A')})", COLOR_TEXT, False),
                    (f" Disco:   {s.get('disk_free','N/A')} livre / {s.get('disk_total','N/A')}", COLOR_TEXT, False),
                    (f" Load:    {s.get('load_avg','N/A')}",           COLOR_ACCENT,    False),
                    (f" Procs:   {s.get('proc_count',0)}",             COLOR_ACCENT,    False),
                    ("── Sistema ───────────────", COLOR_HIGHLIGHT,  True),
                    (f" Uptime:  {s.get('uptime','N/A')}",             COLOR_TEXT,      False),
                    (f" OS:      {s.get('os_version','N/A')[:23]}",    COLOR_ACCENT,    False),
                    ("── Boot ──────────────────", COLOR_HIGHLIGHT,  True),
                    (f" I2C:     {'Ativado' if i2c_on else 'Desativado'}", COLOR_GOOD if i2c_on else COLOR_ERROR, False),
                    (f" Config:  {os.path.basename(boot.get('config','N/A'))}", COLOR_ACCENT, False),
                    ("── SSH ───────────────────", COLOR_HIGHLIGHT,  True),
                    (f" Sessoes: {ssh_n} ativa(s)", COLOR_HIGHLIGHT if ssh_n > 0 else COLOR_ACCENT, False),
                ] + [(f"  └ {ip}", COLOR_ACCENT, False) for ip in s.get("ssh_ips", [])[:3]]

                self._sys_items_count = len(sys_items)
                max_vis, row_h = 7, 19
                start   = self._sys_scroll
                content_bottom = y + max_vis * row_h

                for i in range(max_vis):
                    idx = start + i
                    if idx >= self._sys_items_count:
                        break
                    text, color, is_header = sys_items[idx]
                    font = self.font_small if not is_header else self.font_tiny
                    draw.text((10, y + i * row_h), text, font=font, fill=color)

                can_up   = start > 0
                can_down = start + max_vis < self._sys_items_count
                self._draw_scroll_arrows(draw, y, content_bottom, can_up, can_down)
                draw.text((10, fy - 14), f"Atualizado: {age_str(self._sys_updated)}", font=self.font_tiny, fill=COLOR_ACCENT)
            self._draw_footer(draw, "ESC: Voltar  ▲▼: rolar", fy)

        # ── WIFI SCAN ────────────────────────────────────────────────────────
        elif self.state == "WIFI_SCAN":
            # Header: count + refresh indicator
            count     = len(self.wifi_list)
            spin_char = "|/-\\"[int(time.time() * 3) % 4] if self._wifi_updating else " "
            hdr_txt   = f"WiFi  {count} rede(s)"
            draw.text((10, y), hdr_txt, font=self.font_small, fill=COLOR_HIGHLIGHT)
            if self._wifi_updating:
                draw.text((self.width - 18, y), spin_char, font=self.font_small, fill=COLOR_WARN)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y_list = y + 22

            if self._wifi_updating and not self.wifi_list:
                self._centered_text(draw, "Buscando redes...", y_list + 25, self.font_text, COLOR_ACCENT)
            elif not self.wifi_list:
                self._centered_text(draw, "Nenhuma rede encontrada", y_list + 25, self.font_text, COLOR_ACCENT)
            else:
                max_vis, row_h = 5, 26
                start = max(0, self.selected_index - max_vis + 1) if self.selected_index >= max_vis else 0
                for i, net in enumerate(self.wifi_list[start: start + max_vis]):
                    actual    = start + i
                    is_sel    = actual == self.selected_index
                    is_active = net["ssid"] == self._active_ssid
                    is_known  = net["ssid"] in self.known_networks
                    row_y     = y_list + i * row_h

                    # Highlight selected row
                    if is_sel:
                        draw.rectangle((0, row_y - 1, self.width, row_y + row_h - 2), fill=(45, 45, 68))

                    # Status prefix
                    if is_active:
                        pfx, pfx_color = "●", COLOR_GOOD
                    elif is_sel:
                        pfx, pfx_color = ">", COLOR_HIGHLIGHT
                    elif is_known:
                        pfx, pfx_color = "*", COLOR_ACCENT
                    else:
                        pfx, pfx_color = " ", COLOR_TEXT
                    draw.text((8, row_y + 5), pfx, font=self.font_small, fill=pfx_color)

                    # Signal bars
                    try:
                        pct = int(net["signal"])
                    except (ValueError, TypeError):
                        pct = 0
                    self._draw_signal_bars(draw, 24, row_y + row_h - 5, pct)

                    # SSID
                    ssid_d     = net["ssid"][:20] + ".." if len(net["ssid"]) > 20 else net["ssid"]
                    ssid_color = COLOR_GOOD if is_active else (COLOR_HIGHLIGHT if is_sel else COLOR_TEXT)
                    draw.text((46, row_y + 5), ssid_d, font=self.font_small, fill=ssid_color)

                    # Security badge (right-aligned)
                    sec = self._wifi_details.get(net["ssid"], "")
                    if not sec or sec == "--":
                        sec_label, sec_color = "Aberta", COLOR_ERROR
                    elif "WPA2" in sec:
                        sec_label, sec_color = "WPA2", COLOR_GOOD
                    elif "WPA" in sec:
                        sec_label, sec_color = "WPA", COLOR_WARN
                    else:
                        sec_label, sec_color = sec[:5], COLOR_ACCENT
                    tw = draw.textlength(sec_label, font=self.font_tiny)
                    draw.text((self.width - tw - 6, row_y + 7), sec_label, font=self.font_tiny, fill=sec_color)

                self._draw_scroll_arrows(
                    draw, y_list, y_list + max_vis * row_h,
                    start > 0, start + max_vis < len(self.wifi_list),
                )
            age = age_str(self._wifi_updated)
            draw.text((10, fy - 14), f"Atual: {age}", font=self.font_tiny, fill=COLOR_ACCENT)
            legend = "● conect  * salva  ESC: voltar"
            self._draw_footer(draw, legend, fy)

        # ── WIFI INPUT ───────────────────────────────────────────────────────
        elif self.state == "WIFI_INPUT":
            ssid = self.wifi_list[self.selected_index]["ssid"]
            draw.text((10, y), "Conectar WiFi", font=self.font_small, fill=COLOR_HIGHLIGHT)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y2 = y + 24
            draw.text((10, y2), "Rede:", font=self.font_tiny, fill=COLOR_ACCENT)
            ssid_d = ssid[:22] + ".." if len(ssid) > 22 else ssid
            draw.text((46, y2), ssid_d, font=self.font_small, fill=COLOR_TEXT)
            draw.text((10, y2 + 22), "Senha:", font=self.font_tiny, fill=COLOR_ACCENT)
            pw_y = y2 + 38
            draw.rectangle((10, pw_y, self.width - 10, pw_y + 30), outline=COLOR_HIGHLIGHT, width=2)
            display_pass = self.password if self.show_password else "●" * len(self.password)
            draw.text((15, pw_y + 6), f"{display_pass}|", font=self.font_small, fill=COLOR_TEXT)
            eye_txt = "Mostrando" if self.show_password else "Ocultando"
            draw.text((10, fy - 20), f"TAB: {eye_txt}  BKSP: apagar", font=self.font_tiny, fill=COLOR_ACCENT)
            self._draw_footer(draw, "ESC: cancelar   ENTER: conectar", fy)

        # ── ABOUT ────────────────────────────────────────────────────────────
        elif self.state == "ABOUT":
            draw.text((10, y), "Sobre o Projeto", font=self.font_small, fill=COLOR_HIGHLIGHT)
            tw_ver = draw.textlength(PROJECT_INFO["version"], font=self.font_tiny)
            draw.text((self.width - tw_ver - 8, y + 3), PROJECT_INFO["version"],
                      font=self.font_tiny, fill=COLOR_ACCENT)
            draw.line((0, y + 18, self.width, y + 18), fill=COLOR_ACCENT, width=1)
            y_list = y + 22

            # ── word-wrap helper ─────────────────────────────────────────────
            # Each item: (text, color, px_indent, font, row_type)
            # row_type: "text" | "section" | "name_icon" | "dev_badge" | "dev_name"
            ai = []

            def _sec(title):
                ai.append((title, COLOR_HIGHLIGHT, 0, self.font_tiny, "section"))

            def _wrap(text, color, px_indent, font):
                if not text.strip():
                    ai.append(("", COLOR_TEXT, 0, self.font_tiny, "text"))
                    return
                avail = self.width - px_indent - 14
                words, line = text.split(), ""
                for word in words:
                    test = (line + " " + word).strip()
                    if draw.textlength(test, font=font) <= avail:
                        line = test
                    else:
                        if line:
                            ai.append((line, color, px_indent, font, "text"))
                        line = word
                if line:
                    ai.append((line, color, px_indent, font, "text"))

            def _menu_entry(idx_m, name, color, desc):
                """Menu item name with inline Material icon + wrapped description."""
                ai.append((idx_m, name, color, None, "name_icon"))
                _wrap(desc, COLOR_ACCENT, 22, self.font_tiny)
                ai.append(("", COLOR_TEXT, 0, self.font_tiny, "text"))

            # ── O Projeto ─────────────────────────────────────────────────────
            _sec("── O Projeto ──────────────────────")
            ai.append((PROJECT_INFO["name"], COLOR_TEXT, 8, self.font_small, "text"))
            _wrap(
                "Ferramenta de monitoramento optico embarcada em Raspberry Pi "
                "rodando Linux aarch64 / kernel 6.x.",
                COLOR_TEXT, 8, self.font_tiny,
            )
            _wrap("", COLOR_TEXT, 0, self.font_tiny)
            _wrap(
                "Le transceptores SFP/SFP+ via I2C usando o padrao SFF-8472 "
                "(registradores A0h e A2h).",
                COLOR_TEXT, 8, self.font_tiny,
            )
            _wrap("", COLOR_TEXT, 0, self.font_tiny)
            _wrap(
                "Interface: display ST7789 320x240 via SPI, teclado USB via evdev. "
                "API REST FastAPI + GUI React.",
                COLOR_TEXT, 8, self.font_tiny,
            )
            ai.append(("", COLOR_TEXT, 0, self.font_tiny, "text"))

            # ── O que cada tela mostra ────────────────────────────────────────
            _sec("── O que cada tela mostra ─────────")
            _menu_entry(0, "Leitura SFP",    (6,182,212),
                "Potencia RX/TX, corrente de polarizacao, temperatura, tensao e dados do fabricante.")
            _menu_entry(1, "Alertas SFP",    (251,191,36),
                "Alarmes configurados com os limites da norma SFF-8472 para cada parametro optico.")
            _menu_entry(2, "Status Servicos",(74,222,128),
                "Estado em tempo real dos servicos systemd: sfp-daemon, Nginx, MongoDB e API.")
            _menu_entry(3, "Rede & Debug",   (99,179,237),
                "IP local, gateway, DNS, SSID ativo, sinal WiFi, IO de rede e URLs de acesso.")
            _menu_entry(4, "Scan I2C",       (167,139,250),
                "Detecta todos os dispositivos no barramento I2C com descricao dos enderecos conhecidos.")
            _menu_entry(5, "Sistema & Boot", COLOR_ACCENT,
                "CPU, RAM, disco, uptime, frequencia, throttle, I2C, SSH e configuracao de boot.")
            _menu_entry(6, "Config WiFi",    (251,191,36),
                "Lista redes disponiveis com sinal e seguranca. Conecta redes salvas ou novas.")

            # ── Desenvolvedores ───────────────────────────────────────────────
            _sec("── Desenvolvido por ────────────────")
            ai.append((PROJECT_INFO["org"], COLOR_HIGHLIGHT, 8, self.font_small, "text"))
            ai.append(("", COLOR_TEXT, 0, self.font_tiny, "text"))
            for dev in PROJECT_INFO["devs"]:
                ai.append((dev["github"], dev["name"], 8, self.font_small, "dev_badge"))
                ai.append((dev["name"], COLOR_ACCENT, 8, self.font_tiny, "dev_name"))
            ai.append(("", COLOR_TEXT, 0, self.font_tiny, "text"))
            ai.append((PROJECT_INFO["email"], COLOR_ACCENT, 8, self.font_tiny, "text"))
            ai.append((PROJECT_INFO["repo"],  COLOR_ACCENT, 8, self.font_tiny, "text"))
            ai.append(("", COLOR_TEXT, 0, self.font_tiny, "text"))

            # ── Tecnologias ───────────────────────────────────────────────────
            _sec("── Tecnologias ─────────────────────")
            _wrap("Python 3, PIL/Pillow, FastAPI, evdev, lgpio, nmcli, "
                  "Raspberry Pi, Linux aarch64.", COLOR_ACCENT, 8, self.font_tiny)
            ai.append((f"Licenca: {PROJECT_INFO['license']}", COLOR_ACCENT, 8, self.font_tiny, "text"))

            # ── Render scrollable list ────────────────────────────────────────
            n_about = len(ai)
            row_h   = 17
            start   = self._about_scroll
            self._about_items_count = n_about

            for i in range(_ABOUT_MAX_VIS):
                idx_a = start + i
                if idx_a >= n_about:
                    break
                item = ai[idx_a]
                ry   = y_list + i * row_h

                rtype = item[4]
                if rtype == "section":
                    draw.text((10, ry + 2), item[0], font=self.font_tiny, fill=COLOR_HIGHLIGHT)

                elif rtype == "name_icon":
                    idx_m, name, color = item[0], item[1], item[2]
                    if self.font_icon_sm:
                        mi_ch = _MENU_ICON_CHARS[idx_m]
                        draw.text((8, ry + 1), mi_ch, font=self.font_icon_sm, fill=color)
                        draw.text((28, ry + 2), name, font=self.font_small, fill=color)
                    else:
                        draw.text((8, ry + 2), name, font=self.font_small, fill=color)

                elif rtype == "dev_badge":
                    github = item[0]
                    bx = 8
                    # GitHub-dark badge
                    draw.rectangle((bx, ry, bx + 26, ry + 15),
                                   fill=(36, 41, 47), outline=(88, 96, 105), width=1)
                    tw_gh = draw.textlength("GH", font=self.font_tiny)
                    draw.text((bx + (26 - tw_gh) // 2, ry + 2), "GH",
                              font=self.font_tiny, fill=(240, 246, 252))
                    draw.text((bx + 30, ry + 1), f"@{github}",
                              font=self.font_small, fill=COLOR_HIGHLIGHT)

                elif rtype == "dev_name":
                    # Full name indented under the badge
                    draw.text((38, ry), item[0], font=self.font_tiny, fill=COLOR_ACCENT)

                else:  # "text"
                    text, color, px_ind, font = item[0], item[1], item[2], item[3]
                    draw.text((10 + px_ind, ry), text, font=font, fill=color)

            can_up   = start > 0
            can_down = start + _ABOUT_MAX_VIS < n_about
            self._draw_scroll_arrows(draw, y_list, y_list + _ABOUT_MAX_VIS * row_h, can_up, can_down)
            self._draw_footer(draw, "ESC: Voltar  ▲▼: rolar", fy)

        # ── SHUTTING DOWN / RESTARTING ────────────────────
        elif self.state == "SHUTTING_DOWN":
            elapsed  = time.time() - self._shutdown_start
            duration = 2.8
            progress = min(1.0, elapsed / duration)

            is_reboot  = self._shutdown_mode == "reboot"
            ring_color = COLOR_WARN  if is_reboot else COLOR_ERROR
            ring_dim   = (50, 40, 10) if is_reboot else (60, 20, 20)
            label_txt  = "Reiniciando..." if is_reboot else "Desligando..."
            mi_char    = _MENU_ICON_CHARS[8] if is_reboot else _MENU_ICON_CHARS[9]

            fade = 1.0 - progress * 0.75
            draw.rectangle((0, 0, self.width, self.height),
                            fill=(int(30 * fade), int(30 * fade), int(46 * fade)))

            cx = self.width  // 2
            cy = self.height // 2 - 10

            r = 44
            bbox = (cx - r, cy - r, cx + r, cy + r)
            draw.arc(bbox, start=0, end=360, fill=ring_dim, width=5)
            if progress > 0.01:
                draw.arc(bbox, start=-90, end=int(-90 + 360 * progress),
                         fill=ring_color, width=5)

            if self.font_icon:
                pulse = abs(math.sin(elapsed * math.pi * 2.5))
                if is_reboot:
                    ic_color = (int(200 + 55 * pulse), int(150 + 41 * pulse), int(20 + 16 * (1.0 - pulse)))
                else:
                    v = int(30 + 40 * (1.0 - pulse))
                    ic_color = (int(200 + 55 * pulse), v, v)
                tw_ic = draw.textlength(mi_char, font=self.font_icon)
                draw.text((cx - int(tw_ic) // 2, cy - 13),
                          mi_char, font=self.font_icon, fill=ic_color)

            self._centered_text(draw, label_txt, cy + r + 10,
                                 self.font_small, ring_color)

            for d in range(3):
                dot_filled = progress >= (d + 1) / 3.0
                dx = cx - 12 + d * 12
                dy = cy + r + 28
                draw.ellipse((dx - 3, dy - 3, dx + 3, dy + 3),
                             fill=ring_color if dot_filled else ring_dim)

        # ── CONFIRM SHUTDOWN ─────────────────────────────────────────────────
        elif self.state == "CONFIRM_SHUTDOWN":
            bx, bw, bh = 12, self.width - 24, 110
            by = y + 6
            draw.rectangle((bx, by, bx + bw, by + bh), fill=(45, 18, 18), outline=COLOR_ERROR, width=2)
            self._centered_text(draw, "Desligar sistema?", by + 14, self.font_text, COLOR_ERROR)
            draw.line((bx + 4, by + 40, bx + bw - 4, by + 40), fill=(75, 28, 28), width=1)
            self._centered_text(draw, "ENTER  Confirmar", by + 54, self.font_small, COLOR_GOOD)
            self._centered_text(draw, "ESC    Cancelar",  by + 78, self.font_small, COLOR_ACCENT)

        # ── LOADING ──────────────────────────────────────────────────────────
        elif self.state == "LOADING":
            spinner = "|/-\\"[int(time.time() * 6) % 4]
            cx = self.width // 2
            self._centered_text(draw, spinner, y + 8, self.font_title, COLOR_HIGHLIGHT)
            for li, line in enumerate(self._loading_msg.split("\n")):
                tw = draw.textlength(line, font=self.font_small)
                draw.text((cx - tw // 2, y + 44 + li * 18), line, font=self.font_small, fill=COLOR_TEXT)
            # Bounce bar
            bar_x, bar_y = 20, y + 90
            bar_w, bar_h = self.width - 40, 8
            draw.rectangle((bar_x, bar_y, bar_x + bar_w, bar_y + bar_h), outline=COLOR_ACCENT, width=1)
            seg = bar_w // 3
            t   = time.time() % 2.0
            pos = int((t / 2.0) * (bar_w - seg)) if t < 1.0 else int(((2.0 - t) / 1.0) * (bar_w - seg))
            draw.rectangle((bar_x + pos, bar_y, bar_x + pos + seg, bar_y + bar_h), fill=COLOR_HIGHLIGHT)

        # ── Status bar overlay ───────────────────────────────────────────────
        if time.time() < self.status_timer:
            draw.rectangle((0, self.height - 25, self.width, self.height), fill=COLOR_HIGHLIGHT)
            draw.text((10, self.height - 22), self.status_msg, font=self.font_small, fill=(0, 0, 0))
        elif self.keyboard_warning:
            warn_color = (251, 191, 36)
            draw.rectangle((0, self.height - 25, self.width, self.height), fill=warn_color)
            msg = "Nenhum teclado USB detectado"
            tw = draw.textlength(msg, font=self.font_small)
            draw.text(((self.width - tw) // 2, self.height - 22), msg,
                      font=self.font_small, fill=(0, 0, 0))

        return image

    # ══════════════════════════════════════════════════════════════════════════
    # Input handling
    # ══════════════════════════════════════════════════════════════════════════

    def handle_input(self, key_data):
        key, shift = key_data
        if key is None:
            return
        is_up    = key in [103, 105]
        is_down  = key in [108, 106]
        is_enter = key in [28, 96]
        is_esc   = key == 1

        # ── BOOT (any key skips to menu) ─────────────────────────────────────
        if self.state == "BOOT":
            if is_enter or is_esc:
                self.state = "MAIN_MENU"
            return

        # ── MAIN MENU (grid: up/down = row, left/right = column) ────────────
        if self.state == "MAIN_MENU":
            if key == 103:    # UP arrow   – move one row up (2 items)
                self.selected_index = max(0, self.selected_index - 2)
            elif key == 108:  # DOWN arrow – move one row down (2 items)
                self.selected_index = min(_MENU_COUNT - 1, self.selected_index + 2)
            elif key == 105:  # LEFT arrow – move left in row
                self.selected_index = max(0, self.selected_index - 1)
            elif key == 106:  # RIGHT arrow – move right in row
                self.selected_index = min(_MENU_COUNT - 1, self.selected_index + 1)
            elif is_enter:
                self._on_menu_enter(self.selected_index)

        # ── SFP VIEW ─────────────────────────────────────────────────────────
        elif self.state == "SFP_VIEW":
            if is_up:
                self.selected_index = (self.selected_index - 1) % _SFP_ITEMS_COUNT
            elif is_down:
                self.selected_index = (self.selected_index + 1) % _SFP_ITEMS_COUNT
            elif is_esc or is_enter:
                self._go_menu()

        # ── SFP ALARMS ───────────────────────────────────────────────────────
        elif self.state == "SFP_ALARMS":
            alarms = compute_sfp_alarms(self.sfp_data)
            max_scroll = max(0, len(alarms) - 5)
            if is_up:
                self._alarm_scroll = max(0, self._alarm_scroll - 1)
            elif is_down:
                self._alarm_scroll = min(max_scroll, self._alarm_scroll + 1)
            elif is_esc or is_enter:
                self._go_menu()

        # ── SERVICES ─────────────────────────────────────────────────────────
        elif self.state == "SERVICES":
            if is_esc or is_enter:
                self._go_menu()

        # ── NET DEBUG ────────────────────────────────────────────────────────
        elif self.state == "NET_DEBUG":
            if is_up:
                self._net_scroll = max(0, self._net_scroll - 1)
            elif is_down:
                self._net_scroll = min(_NET_MAX_SCROLL, self._net_scroll + 1)
            elif is_esc or is_enter:
                self._net_scroll = 0
                self._go_menu()

        # ── SYS DIAG ─────────────────────────────────────────────────────────
        elif self.state == "SYS_DIAG":
            max_scroll = max(0, self._sys_items_count - 7)
            if is_up:
                self._sys_scroll = max(0, self._sys_scroll - 1)
            elif is_down:
                self._sys_scroll = min(max_scroll, self._sys_scroll + 1)
            elif is_esc or is_enter:
                self._sys_scroll = 0
                self._go_menu()

        # ── I2C SCAN ─────────────────────────────────────────────────────────
        elif self.state == "I2C_SCAN":
            if is_esc or is_enter:
                self._go_menu()

        # ── WIFI SCAN ────────────────────────────────────────────────────────
        elif self.state == "WIFI_SCAN":
            if self.wifi_list:
                if is_up:
                    self.selected_index = (self.selected_index - 1) % len(self.wifi_list)
                elif is_down:
                    self.selected_index = (self.selected_index + 1) % len(self.wifi_list)
                elif is_enter:
                    ssid = self.wifi_list[self.selected_index]["ssid"]
                    if ssid in self.known_networks:
                        self.set_status("Conectando (Salva)...")
                        self._loading_msg = f"Conectando a\n{ssid[:20]}..."
                        self.state = "LOADING"
                        threading.Thread(target=self._connect_known_task, args=(ssid,), daemon=True).start()
                    else:
                        self.state      = "WIFI_INPUT"
                        self.password   = ""
                        self.show_password = False
            if is_esc:
                self._go_menu()

        # ── WIFI INPUT ───────────────────────────────────────────────────────
        elif self.state == "WIFI_INPUT":
            if is_esc:
                self.state = "WIFI_SCAN"
            elif key == 15:  # TAB
                self.show_password = not self.show_password
            elif is_enter:
                ssid = self.wifi_list[self.selected_index]["ssid"]
                self.set_status("Conectando...")
                self._loading_msg = f"Conectando a\n{ssid[:20]}..."
                self.state = "LOADING"
                threading.Thread(target=self._connect_task, args=(ssid, self.password), daemon=True).start()
            elif key == 14:  # BACKSPACE
                self.password = self.password[:-1]
            else:
                char = self._map_key(key, shift)
                if char:
                    self.password += char

        # ── ABOUT ────────────────────────────────────────────────────────────
        elif self.state == "ABOUT":
            max_scroll = max(0, self._about_items_count - _ABOUT_MAX_VIS)
            if is_up:
                self._about_scroll = max(0, self._about_scroll - 1)
            elif is_down:
                self._about_scroll = min(max_scroll, self._about_scroll + 1)
            elif is_esc or is_enter:
                self._about_scroll = 0
                self._go_menu()

        # ── CONFIRM SHUTDOWN ─────────────────────────────────────────────────
        elif self.state == "CONFIRM_SHUTDOWN":
            if is_enter:
                self._shutdown_mode  = "poweroff"
                self._shutdown_start = time.time()
                self.state = "SHUTTING_DOWN"
                threading.Thread(target=self._shutdown_task, daemon=True).start()
            elif is_esc:
                self._go_menu()

    def _go_menu(self):
        self.state          = "MAIN_MENU"
        self.selected_index = 0

    def _on_menu_enter(self, idx: int):
        if idx == 0:   # Leitura SFP
            self._loading_msg   = "Lendo modulo SFP..."
            self.state          = "LOADING"
            self.selected_index = 0
            threading.Thread(target=self._update_sfp, args=("SFP_VIEW",), daemon=True).start()

        elif idx == 1:  # Alertas SFP
            if self.sfp_data:
                self.state          = "SFP_ALARMS"
                self._alarm_scroll  = 0
            else:
                self._loading_msg   = "Lendo modulo SFP..."
                self.state          = "LOADING"
                threading.Thread(target=self._update_sfp, args=("SFP_ALARMS",), daemon=True).start()

        elif idx == 2:  # Status Servicos
            self.state = "SERVICES"
            self._trigger_svc_update()

        elif idx == 3:  # Rede & Debug
            self.state = "NET_DEBUG"
            self._trigger_net_update()

        elif idx == 4:  # Scan I2C
            self._loading_msg = "Escaneando barramento\nI2C..."
            self.state        = "LOADING"
            self._trigger_i2c_update()

        elif idx == 5:  # Sistema & Boot
            self.state = "SYS_DIAG"
            self._trigger_sys_update()

        elif idx == 6:  # Configurar WiFi
            self._loading_msg   = "Buscando redes\nWiFi..."
            self.state          = "LOADING"
            self.selected_index = 0
            self._trigger_wifi_update()

        elif idx == 7:  # Sobre o Projeto
            self.state = "ABOUT"

        elif idx == 8:  # Reiniciar
            self._shutdown_mode  = "reboot"
            self._shutdown_start = __import__("time").time()
            self.state = "SHUTTING_DOWN"
            __import__("threading").Thread(target=self._shutdown_task, daemon=True).start()

        elif idx == 9:  # Desligar
            self.state = "CONFIRM_SHUTDOWN"

    def _map_key(self, code: int, shift: bool):
        base = {
            2:"1", 3:"2", 4:"3", 5:"4", 6:"5", 7:"6", 8:"7", 9:"8", 10:"9", 11:"0", 12:"-", 13:"=",
            16:"q", 17:"w", 18:"e", 19:"r", 20:"t", 21:"y", 22:"u", 23:"i", 24:"o", 25:"p",
            30:"a", 31:"s", 32:"d", 33:"f", 34:"g", 35:"h", 36:"j", 37:"k", 38:"l",
            44:"z", 45:"x", 46:"c", 47:"v", 48:"b", 49:"n", 50:"m", 52:".", 57:" ",
        }
        shifted = {
            2:"!", 3:"@", 4:"#", 5:"$", 6:"%", 7:"^", 8:"&", 9:"*", 10:"(", 11:")", 12:"_", 13:"+",
            16:"Q", 17:"W", 18:"E", 19:"R", 20:"T", 21:"Y", 22:"U", 23:"I", 24:"O", 25:"P",
            30:"A", 31:"S", 32:"D", 33:"F", 34:"G", 35:"H", 36:"J", 37:"K", 38:"L",
            44:"Z", 45:"X", 46:"C", 47:"V", 48:"B", 49:"N", 50:"M", 52:">", 57:" ",
        }
        return (shifted if shift else base).get(code)
