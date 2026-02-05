"""Configurações da TUI"""

import os
from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    """Configurações da aplicação"""

    # Socket do daemon
    sfp_daemon_socket: str = "/run/sfp-daemon/sfp.sock"

    # Timeout para operações de socket (segundos)
    socket_timeout: float = 5.0


settings = Settings()
