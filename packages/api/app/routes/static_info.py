"""Rota /api/static - Dados estáticos do SFP (A0h)"""

import logging
from fastapi import APIRouter, HTTPException
from app.daemon_client import DaemonClient

logger = logging.getLogger(__name__)

# Mantendo o mesmo prefixo /api que o current.py usa
router = APIRouter(prefix="/api", tags=["sfp"])


@router.get("/static")
async def get_static():
    """
    Retorna apenas os dados estáticos do SFP (A0h)
    
    Returns:
        JSON com os dados da página A0h
        
    Raises:
        404: SFP não encontrado
        503: Daemon indisponível
        500: Erro interno
    """
    client = DaemonClient()
    
    try:
        data = await client.get_static()
        return data
    except FileNotFoundError:
        logger.warning("SFP não encontrado")
        raise HTTPException(
            status_code=404,
            detail="SFP não encontrado"
        )
    except (ConnectionError, TimeoutError) as e:
        logger.error(f"Erro ao comunicar com daemon: {e}")
        raise HTTPException(
            status_code=503,
            detail=f"Daemon indisponível: {str(e)}"
        )
    except Exception as e:
        logger.error(f"Erro inesperado: {e}", exc_info=True)
        raise HTTPException(
            status_code=500,
            detail=f"Erro interno: {str(e)}"
        )
