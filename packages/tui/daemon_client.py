"""Cliente síncrono para comunicação com o daemon SFP via socket UNIX"""

import json
import socket
from typing import Optional, Dict, Any
from config import settings


class DaemonClient:
    """Cliente síncrono para comunicação com o daemon SFP"""
    
    def __init__(self, socket_path: Optional[str] = None, timeout: Optional[float] = None):
        """
        Inicializa o cliente
        
        Args:
            socket_path: Caminho do socket UNIX (padrão: settings.sfp_daemon_socket)
            timeout: Timeout em segundos (padrão: settings.socket_timeout)
        """
        self.socket_path = socket_path or settings.sfp_daemon_socket
        self.timeout = timeout or settings.socket_timeout
    
    def _send_command(self, command: str) -> Dict[str, Any]:
        """
        Envia comando ao daemon e retorna resposta parseada
        
        Args:
            command: Comando a enviar (ex: "GET CURRENT")
            
        Returns:
            Dicionário com status_code, status_message e data (JSON parseado)
            
        Raises:
            ConnectionError: Se não conseguir conectar ao socket
            TimeoutError: Se a operação exceder o timeout
            ValueError: Se a resposta não puder ser parseada
        """
        sock = None
        try:
            # Cria socket UNIX
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.settimeout(self.timeout)
            
            # Conecta ao socket
            try:
                sock.connect(self.socket_path)
            except FileNotFoundError:
                raise ConnectionError(f"Socket não encontrado: {self.socket_path}")
            except ConnectionRefusedError:
                raise ConnectionError(f"Conexão recusada: {self.socket_path}")
            
            # Envia comando
            command_bytes = (command + "\n").encode('utf-8')
            sock.sendall(command_bytes)
            
            # Recebe resposta
            response = b""
            while True:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                response += chunk
                # Verifica se recebeu toda a resposta (termina com \n após JSON)
                if response.count(b'\n') >= 2:
                    break
            
            # Parse da resposta
            response_str = response.decode('utf-8', errors='ignore')
            lines = response_str.strip().split('\n', 1)
            
            if len(lines) < 2:
                raise ValueError(f"Resposta inválida do daemon: {response_str[:100]}")
            
            # Parse da linha de status: "STATUS <code> <message>"
            status_line = lines[0].strip()
            if not status_line.startswith("STATUS "):
                raise ValueError(f"Formato de status inválido: {status_line}")
            
            status_parts = status_line[7:].strip().split(' ', 1)
            if len(status_parts) == 0:
                raise ValueError(f"Formato de status inválido: {status_line}")
            
            try:
                status_code = int(status_parts[0])
            except ValueError:
                raise ValueError(f"Código de status inválido: {status_parts[0]}")
            
            status_message = status_parts[1] if len(status_parts) > 1 else "OK"
            
            # Parse do JSON
            json_str = lines[1].strip()
            try:
                data = json.loads(json_str)
            except json.JSONDecodeError as e:
                raise ValueError(f"JSON inválido na resposta: {e}")
            
            return {
                "status_code": status_code,
                "status_message": status_message,
                "data": data
            }
            
        except socket.timeout:
            raise TimeoutError(f"Timeout ao comunicar com daemon")
        except OSError as e:
            raise ConnectionError(f"Erro de conexão: {e}")
        finally:
            if sock:
                sock.close()
    
    def get_current(self) -> Dict[str, Any]:
        """
        Obtém estado completo do SFP (A0h + A2h + metadados)
        
        Returns:
            Dicionário com dados completos do SFP
            
        Raises:
            FileNotFoundError: Se SFP não encontrado
            ConnectionError: Se não conseguir conectar
            TimeoutError: Se exceder timeout
        """
        response = self._send_command("GET CURRENT")
        
        if response["status_code"] == 404:
            raise FileNotFoundError("SFP não encontrado")
        elif response["status_code"] != 200:
            raise RuntimeError(f"Erro do daemon: {response['status_message']}")
        
        return response["data"]
    
    def get_static(self) -> Dict[str, Any]:
        """
        Obtém apenas dados estáticos A0h
        
        Returns:
            Dicionário com dados A0h
        """
        response = self._send_command("GET STATIC")
        
        if response["status_code"] == 404:
            raise FileNotFoundError("SFP não encontrado")
        elif response["status_code"] != 200:
            raise RuntimeError(f"Erro do daemon: {response['status_message']}")
        
        return response["data"]
    
    def get_dynamic(self) -> Dict[str, Any]:
        """
        Obtém apenas dados dinâmicos A2h
        
        Returns:
            Dicionário com dados A2h
        """
        response = self._send_command("GET DYNAMIC")
        
        if response["status_code"] == 404:
            raise FileNotFoundError("SFP não encontrado")
        elif response["status_code"] != 200:
            raise RuntimeError(f"Erro do daemon: {response['status_message']}")
        
        return response["data"]
    
    def get_state(self) -> Dict[str, Any]:
        """
        Obtém apenas estado da FSM e timestamps
        
        Returns:
            Dicionário com estado e timestamps
        """
        response = self._send_command("GET STATE")
        
        if response["status_code"] != 200:
            raise RuntimeError(f"Erro do daemon: {response['status_message']}")
        
        return response["data"]
