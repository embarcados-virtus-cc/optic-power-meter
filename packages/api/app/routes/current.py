"""Rota /api/current - Estado completo do SFP"""

import logging
from fastapi import APIRouter, HTTPException
from app.daemon_client import DaemonClient

logger = logging.getLogger(__name__)

router = APIRouter(prefix="/api", tags=["sfp"])


@router.get("/current")
async def get_current():
    """
    Retorna o estado completo do SFP (A0h + A2h + metadados)
    
    Returns:
        JSON com todos os dados do SFP
        
    Raises:
        404: SFP não encontrado
        503: Daemon indisponível
        500: Erro interno
    """
    client = DaemonClient()
    
    try:
        data = await client.get_current()
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

