"""Configurações da API"""

import os
from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    """Configurações da aplicação"""
    
    # Socket do daemon
    sfp_daemon_socket: str = "/run/sfp-daemon/sfp.sock"
    
    # Servidor HTTP
    api_host: str = "0.0.0.0"
    api_port: int = 8000
    
    # Timeout para operações de socket (segundos)
    socket_timeout: float = 5.0
    
    class Config:
        env_prefix = ""
        case_sensitive = False


settings = Settings()

