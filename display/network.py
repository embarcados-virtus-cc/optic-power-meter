import subprocess

import netifaces
import psutil


class NetworkManager:
    @staticmethod
    def get_detailed_info():
        info = {
            "ip": "N/A", "gateway": "N/A", "dns": "N/A",
            "ssid": "N/A", "signal": "--", "tx_rx": "0/0 KB",
            "iface": "N/A",
        }
        try:
            gateways = netifaces.gateways()
            if "default" in gateways and netifaces.AF_INET in gateways["default"]:
                gw_info = gateways["default"][netifaces.AF_INET]
                info["gateway"] = gw_info[0]
                iface = gw_info[1]
                info["iface"] = iface
                addrs = netifaces.ifaddresses(iface)
                if netifaces.AF_INET in addrs:
                    info["ip"] = addrs[netifaces.AF_INET][0]["addr"]
                io = psutil.net_io_counters(pernic=True).get(iface)
                if io:
                    info["tx_rx"] = f"{io.bytes_sent // 1024}/{io.bytes_recv // 1024} KB"
        except Exception as e:
            print(f"Rede IP erro: {e}")

        try:
            with open("/etc/resolv.conf") as f:
                for line in f:
                    if line.startswith("nameserver"):
                        info["dns"] = line.split()[1]
                        break
        except Exception:
            pass

        try:
            res = subprocess.run(
                ["nmcli", "-t", "-f", "ACTIVE,SSID,SIGNAL", "dev", "wifi"],
                capture_output=True, text=True, timeout=5,
            )
            for line in res.stdout.split("\n"):
                if line.startswith("yes"):
                    parts = line.split(":")
                    info["ssid"] = parts[1] if len(parts) > 1 else "N/A"
                    info["signal"] = f"{parts[2]}%" if len(parts) > 2 else "--"
                    break
        except Exception:
            pass

        return info

    @staticmethod
    def test_connectivity():
        try:
            res = subprocess.run(
                ["ping", "-c", "1", "-W", "2", "8.8.8.8"],
                capture_output=True, timeout=5,
            )
            return res.returncode == 0
        except Exception:
            return False

    @staticmethod
    def scan_wifi():
        try:
            subprocess.run(["nmcli", "device", "wifi", "rescan"], capture_output=True, timeout=10)
            output = subprocess.check_output(
                ["nmcli", "-f", "SSID,SIGNAL", "device", "wifi", "list"], timeout=10
            ).decode("utf-8")
            networks, seen = [], set()
            for line in output.strip().split("\n")[1:]:
                parts = line.rsplit(None, 1)
                if not parts:
                    continue
                ssid = parts[0].strip()
                if ssid == "--" or ssid in seen or not ssid:
                    continue
                signal = parts[1] if len(parts) > 1 else "0"
                networks.append({"ssid": ssid, "signal": signal})
                seen.add(ssid)
            return networks[:10]
        except Exception:
            return []

    @staticmethod
    def get_known_networks():
        try:
            output = subprocess.check_output(
                ["nmcli", "-t", "-f", "NAME", "connection", "show"], timeout=5
            ).decode("utf-8")
            return [ln.strip() for ln in output.split("\n") if ln.strip()]
        except Exception:
            return []

    @staticmethod
    def connect_known(ssid):
        try:
            result = subprocess.run(
                ["nmcli", "connection", "up", ssid],
                capture_output=True, text=True, timeout=20,
            )
            ok = result.returncode == 0
            return ok, "Conectado!" if ok else "Erro Conexão"
        except Exception:
            return False, "Erro Timeout"

    @staticmethod
    def get_wifi_security() -> dict:
        """Returns {ssid: security_string} for all visible networks."""
        try:
            out = subprocess.check_output(
                ["nmcli", "-t", "-e", "no", "-f", "SSID,SECURITY", "device", "wifi", "list"],
                timeout=10,
            ).decode("utf-8")
            result = {}
            for line in out.strip().split("\n"):
                idx = line.rfind(":")
                if idx < 0:
                    continue
                ssid = line[:idx].strip()
                sec  = line[idx + 1:].strip()
                if ssid and ssid not in result:
                    result[ssid] = sec
            return result
        except Exception:
            return {}

    @staticmethod
    def get_active_ssid() -> str:
        """Returns SSID of currently connected WiFi, or ''."""
        try:
            res = subprocess.run(
                ["nmcli", "-t", "-e", "no", "-f", "ACTIVE,SSID", "dev", "wifi"],
                capture_output=True, text=True, timeout=5,
            )
            for line in res.stdout.split("\n"):
                if line.startswith("yes:"):
                    return line[4:].strip()
        except Exception:
            pass
        return ""

    @staticmethod
    def connect_wifi(ssid, password):
        try:
            subprocess.run(["nmcli", "connection", "delete", ssid], capture_output=True)
            result = subprocess.run(
                ["nmcli", "device", "wifi", "connect", ssid, "password", password],
                capture_output=True, text=True, timeout=20,
            )
            if result.returncode == 0:
                return True, "Conectado!"
            err = result.stderr
            if "Secrets were required" in err or "Not authorized" in err:
                return False, "Senha Incorreta"
            return False, "Erro Conexão"
        except Exception:
            return False, "Timeout/Erro"
