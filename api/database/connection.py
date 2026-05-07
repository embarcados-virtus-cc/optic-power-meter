import os
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure
from typing import Optional

def get_mongo_client() -> Optional[MongoClient]:
    """
    Retorna um cliente MongoDB baseado na variável de ambiente MONGO_URI.
    """
    uri = os.getenv("MONGO_URI")
    if not uri:
        # Tenta construir a partir de outras variáveis se MONGO_URI não estiver definida
        user = os.getenv("MONGO_USER", "admin")
        password = os.getenv("MONGO_PASSWORD", "admin123")
        host = os.getenv("MONGO_HOST", "mongo")
        port = os.getenv("MONGO_PORT", "27017")
        db_name = os.getenv("MONGO_DB", "optic_power_meter")
        uri = f"mongodb://{user}:{password}@{host}:{port}/{db_name}?authSource=admin"
        
    try:
        client = MongoClient(uri, serverSelectionTimeoutMS=5000)
        # Verifica se a conexão está ativa
        client.admin.command('ping')
        return client
    except ConnectionFailure as e:
        print(f"Erro ao conectar ao MongoDB: {e}")
        return None
    except Exception as e:
        print(f"Erro inesperado na conexão com MongoDB: {e}")
        return None

def get_database(db_name: Optional[str] = None):
    """
    Retorna a instância do banco de dados.
    """
    client = get_mongo_client()
    if client:
        name = db_name or os.getenv("MONGO_DB", "optic_power_meter")
        return client[name]
    return None
