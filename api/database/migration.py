from pymongo import DESCENDING, ASCENDING
from .connection import get_database
import datetime

def run_migrations():
    """
    Executa as migrações e inicializa os índices do banco de dados.
    """
    db = get_database()
    if db is None:
        print("Aviso: Não foi possível conectar ao banco de dados para executar as migrações.")
        return

    print("Iniciando migrações do banco de dados...")
    
    # Garantir que a coleção de migrações existe e tem índice
    migrations_coll = db["migrations"]
    migrations_coll.create_index([("version", ASCENDING)], unique=True)

    def is_applied(version):
        return migrations_coll.find_one({"version": version}) is not None

    # Migração 1: Índices básicos
    if not is_applied(1):
        print("Executando migração v1: Índices básicos...")
        readings = db["readings"]
        readings.create_index([("timestamp", DESCENDING)])
        log_migration(1, "Criação de índices básicos na coleção readings")
        print("- Índice 'timestamp' criado.")

    # Migração 2: TTL Index para gerenciar espaço em disco (7 dias)
    if not is_applied(2):
        print("Executando migração v2: TTL Index...")
        readings = db["readings"]
        # expireAfterSeconds: 7 dias = 7 * 24 * 60 * 60 = 604800
        readings.create_index([("created_at", ASCENDING)], expireAfterSeconds=604800)
        log_migration(2, "Criação de TTL index (7 dias) no campo created_at")
        print("- TTL Index 'created_at' criado.")

    # Adicione novas migrações aqui seguindo o padrão if not is_applied(X):
    
    print("Migrações concluídas com sucesso.")

def log_migration(version: int, description: str):
    """
    Registra uma migração concluída.
    """
    db = get_database()
    if db:
        db["migrations"].update_one(
            {"version": version},
            {"$set": {
                "version": version,
                "description": description,
                "applied_at": datetime.datetime.utcnow()
            }},
            upsert=True
        )
