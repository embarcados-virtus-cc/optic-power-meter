import os
import re
import shutil
import subprocess

import psutil

from config import MONITORED_SERVICES, fmt_uptime


class DeviceDiagnostic:
    @staticmethod
    def scan_i2c():
        try:
            if not shutil.which("i2cdetect"):
                return ["i2c-tools nao inst."]
            res = subprocess.run(["i2cdetect", "-y", "1"], capture_output=True, text=True)
            if res.returncode != 0:
                err = res.stderr.lower()
                if "permission denied" in err:
                    return ["Erro Permissao"]
                if "no such file" in err or "not found" in err:
                    return ["I2C Desativado"]
                return ["Erro Barramento"]
            devices = []
            for line in res.stdout.strip().split("\n")[1:]:
                parts = line.split(":", 1)
                if len(parts) < 2:
                    continue
                for addr in re.findall(r"([0-9a-f]{2})", parts[1]):
                    devices.append(addr)
            return devices if devices else ["Vazio"]
        except Exception as e:
            print(f"I2C Scan Erro: {e}")
            return ["Erro Fatal"]

    @staticmethod
    def get_system_stats():
        stats = {
            "model": "Raspberry Pi",
            "cpu_temp": "N/A",
            "cpu_usage": "N/A",
            "cpu_freq": "N/A",
            "throttled": "N/A",
            "mem_usage": "N/A",
            "mem_used_mb": 0,
            "mem_total_mb": 0,
            "disk_free": "N/A",
            "disk_total": "N/A",
            "load_avg": "N/A",
            "proc_count": 0,
            "uptime": "N/A",
            "os_version": "Unknown",
            "ssh_sessions": 0,
            "ssh_ips": [],
        }
        try:
            with open("/proc/device-tree/model") as f:
                stats["model"] = f.read().strip().replace("\x00", "")
        except Exception:
            pass

        try:
            with open("/sys/class/thermal/thermal_zone0/temp") as f:
                stats["cpu_temp"] = f"{int(f.read()) / 1000:.1f}°C"
        except Exception:
            pass

        try:
            with open("/proc/uptime") as f:
                stats["uptime"] = fmt_uptime(float(f.read().split()[0]))
        except Exception:
            pass

        try:
            stats["cpu_usage"] = f"{psutil.cpu_percent(interval=0.3):.0f}%"
            freq = psutil.cpu_freq()
            stats["cpu_freq"] = f"{freq.current:.0f}MHz" if freq else "N/A"
            mem = psutil.virtual_memory()
            stats["mem_usage"] = f"{mem.percent:.0f}%"
            stats["mem_used_mb"] = mem.used // (1024 * 1024)
            stats["mem_total_mb"] = mem.total // (1024 * 1024)
            du = shutil.disk_usage("/")
            stats["disk_free"] = f"{du.free // (1024 ** 3)}GB"
            stats["disk_total"] = f"{du.total // (1024 ** 3)}GB"
            la = psutil.getloadavg()
            stats["load_avg"] = f"{la[0]:.2f} {la[1]:.2f} {la[2]:.2f}"
            stats["proc_count"] = len(psutil.pids())
        except Exception:
            pass

        try:
            res = subprocess.run(
                ["vcgencmd", "get_throttled"],
                capture_output=True, text=True, timeout=2,
            )
            raw = res.stdout.strip().split("=")[-1].strip()
            stats["throttled"] = "OK" if raw == "0x0" else "LIMITADO"
        except Exception:
            stats["throttled"] = "N/A"

        try:
            with open("/etc/os-release") as f:
                for line in f:
                    if line.startswith("PRETTY_NAME"):
                        stats["os_version"] = line.split("=")[1].strip().strip('"')
                        break
        except Exception:
            pass

        try:
            ssh_ips = set()
            for conn in psutil.net_connections(kind="tcp"):
                if conn.laddr.port == 22 and conn.status == "ESTABLISHED" and conn.raddr:
                    ssh_ips.add(conn.raddr.ip)
            stats["ssh_sessions"] = len(ssh_ips)
            stats["ssh_ips"] = list(ssh_ips)
        except Exception:
            pass

        return stats

    @staticmethod
    def get_boot_info():
        paths = {
            "config": (
                "/boot/config.txt"
                if os.path.exists("/boot/config.txt")
                else "/boot/firmware/config.txt"
            ),
            "cmdline": (
                "/boot/cmdline.txt"
                if os.path.exists("/boot/cmdline.txt")
                else "/boot/firmware/cmdline.txt"
            ),
        }
        i2c_enabled = False
        try:
            if os.path.exists(paths["config"]):
                with open(paths["config"]) as f:
                    if "dtparam=i2c_arm=on" in f.read():
                        i2c_enabled = True
        except Exception:
            pass
        paths["i2c_enabled"] = i2c_enabled
        return paths


class ServiceMonitor:
    @staticmethod
    def get_status() -> dict:
        results = {}
        for entry in MONITORED_SERVICES:
            svc_name, label, svc_type = entry if len(entry) == 3 else (*entry, "systemd")
            try:
                if svc_type == "docker":
                    res = subprocess.run(
                        ["docker", "inspect", "--format={{.State.Status}}", svc_name],
                        capture_output=True, text=True, timeout=3,
                    )
                    status = res.stdout.strip()
                    active = status == "running"
                else:
                    res = subprocess.run(
                        ["systemctl", "is-active", svc_name],
                        capture_output=True, text=True, timeout=3,
                    )
                    status = res.stdout.strip()
                    active = status == "active"
                results[svc_name] = {"label": label, "active": active, "status": status}
            except Exception:
                results[svc_name] = {"label": label, "active": False, "status": "erro"}
        return results


def compute_sfp_alarms(sfp_data: dict) -> list:
    if not sfp_data:
        return [{"name": "Dados", "value": "—", "status": "SEM DADOS", "ok": False}]

    a2 = sfp_data.get("a2", {})
    alarms = []

    def _check(name, raw, unit, lo, hi, fmt=".2f"):
        try:
            v = float(raw)
            if v < lo:
                st, ok = "BAIXO", False
            elif v > hi:
                st, ok = "ALTO", False
            else:
                st, ok = "OK", True
            alarms.append({"name": name, "value": f"{v:{fmt}} {unit}", "status": st, "ok": ok})
        except (ValueError, TypeError):
            alarms.append({"name": name, "value": "N/A", "status": "N/A", "ok": None})

    _rx = a2.get("rx_power_dbm")
    _check("RX Power",  _rx, "dBm", -28.0, -3.0)
    _check("TX Bias",   a2.get("tx_bias_ma"),   "mA",   0.5,  10.0)
    _check("Temp",      a2.get("temperature_c"), "C",   -5.0,  70.0, ".1f")
    _check("Tensao",    a2.get("voltage_v"),     "V",    3.0,   3.6)

    tx_dbm = a2.get("tx_power_dbm")
    if tx_dbm is not None:
        _check("TX Power", tx_dbm, "dBm", -10.0, 3.0)

    # Explicit alarm flags returned by daemon (if any)
    flags = a2.get("flags") or a2.get("alarms") or {}
    for flag_name, flag_val in flags.items():
        if flag_val:
            alarms.append({
                "name": flag_name.replace("_", " ").title(),
                "value": "—",
                "status": "ATIVO",
                "ok": False,
            })

    return alarms
