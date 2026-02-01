"""Entry point da API FastAPI"""

import logging
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.config import settings
from app.routes import current

# Configuração de logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)

logger = logging.getLogger(__name__)

# Cria aplicação FastAPI
app = FastAPI(
    title="SFP Power Meter API",
    description="API REST para acesso aos dados do módulo SFP",
    version="1.0.0"
)

# Configura CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Em produção, especificar origens permitidas
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Registra rotas
app.include_router(current.router)


@app.get("/")
async def root():
    """Health check da API"""
    return {
        "status": "ok",
        "service": "SFP Power Meter API",
        "version": "1.0.0"
    }


@app.get("/health")
async def health():
    """Health check detalhado"""
    from app.daemon_client import DaemonClient
    
    client = DaemonClient()
    try:
        ping_data = await client.ping()
        return {
            "status": "ok",
            "daemon": {
                "connected": True,
                "uptime": ping_data.get("uptime_seconds", 0)
            }
        }
    except Exception as e:
        logger.warning(f"Daemon não disponível: {e}")
        return {
            "status": "degraded",
            "daemon": {
                "connected": False,
                "error": str(e)
            }
        }


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "app.main:app",
        host=settings.api_host,
        port=settings.api_port,
        reload=True
    )

