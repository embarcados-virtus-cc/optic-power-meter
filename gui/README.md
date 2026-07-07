# GUI — SFP Optic Power Meter

Frontend React com TypeScript. Dashboard em tempo real para monitoramento de transceptores SFP/SFP+. Inclui gráfico de histórico de potência, parâmetros do módulo, alarmes e página de gerenciamento de containers Docker.

## Tecnologias

| Lib | Versão | Uso |
|---|---|---|
| React | 18 | Framework UI |
| TypeScript | 5.x | Tipagem estática |
| Vite | 5 | Bundler |
| TanStack Router | latest | Roteamento file-based |
| TanStack Store | latest | Estado global reativo |
| Tailwind CSS | 3 | Estilos |
| shadcn/ui + Radix | latest | Componentes acessíveis |
| Recharts | latest | Gráficos |
| lucide-react | 0.562 | Ícones |

## Estrutura

```
gui/
├── src/
│   ├── components/
│   │   ├── dashboard/
│   │   │   └── page/
│   │   │       ├── Parameters.tsx    # Cards: corrente, tensão, temp, TX power, qualidade
│   │   │       └── RxPower.tsx       # Gauge + gráfico histórico RX Power
│   │   ├── layout/
│   │   │   └── page/
│   │   │       ├── Header.tsx        # Navbar desktop (Dashboard / Sistema)
│   │   │       └── MobileNav.tsx     # Navbar mobile
│   │   ├── system/
│   │   │   └── ContainerCard.tsx    # Card de container Docker (status, stats, logs, ações)
│   │   └── ui/                      # shadcn/ui base components
│   ├── routes/
│   │   ├── index.tsx                # Dashboard principal
│   │   └── system/
│   │       └── index.tsx            # Página de gerenciamento de containers
│   ├── stores/
│   │   ├── dataStores.ts            # rxPower, parameters, history, status (polling 2s)
│   │   ├── alarmStore.ts            # Alarmes SFP
│   │   └── containerStore.ts        # Lista de containers + stats (polling 5s)
│   ├── lib/
│   │   └── api.ts                   # Cliente HTTP tipado para todos os endpoints
│   └── routeTree.gen.ts             # Gerado automaticamente pelo TanStack Router
├── nginx.conf                        # Proxy reverso: /api/* → API, /* → SPA
├── pnpm-workspace.yaml
├── tsconfig.json
├── vite.config.ts
└── Dockerfile
```

## Desenvolvimento

```bash
cd gui
pnpm install
pnpm dev
```

Acesse: `http://localhost:5173`

> A API deve estar rodando em `http://localhost:8001` (ou ajuste o proxy em `vite.config.ts`).

## Build de Produção

```bash
pnpm build
# Output em dist/
```

Via Docker:

```bash
docker build -t optic-gui ./gui
docker run -p 8080:80 optic-gui
```

## Roteamento

| Rota | Componente | Descrição |
|---|---|---|
| `/` | `routes/index.tsx` | Dashboard principal |
| `/system` | `routes/system/index.tsx` | Gerenciamento de containers |

O TanStack Router gera `routeTree.gen.ts` automaticamente ao rodar `pnpm dev`. **Não editar manualmente.**

## Stores (Estado Global)

### `dataStores.ts`

Polling a cada **2 segundos** em `GET /api/v1/current`.

```typescript
rxPowerStore       // number  — potência RX em dBm
parametersStore    // { corrente, tensao, temperatura, txPower, qualidadeSinal, module }
historyStore       // number[] — últimos 30 pontos de rx_power_dbm
statusStore        // { online: bool, lastError: string | null }
```

Na primeira carga, busca os últimos 30 pontos do endpoint `/api/v1/history`.

### `alarmStore.ts`

Lê dados A2h do módulo para calcular alarmes SFF-8472.

### `containerStore.ts`

Polling a cada **5 segundos** em `GET /api/v1/containers`. Stats individuais são buscados sequencialmente com cancelamento.

## API Client (`lib/api.ts`)

```typescript
api.current()                          // GET /api/v1/current → CurrentReading
api.history(limit)                     // GET /api/v1/history?limit=N
api.static()                           // GET /api/v1/raw/static
api.containers()                       // GET /api/v1/containers
api.containerStats(name)               // GET /api/v1/containers/{name}/stats
api.containerLogs(name, lines)         // GET /api/v1/containers/{name}/logs
api.containerStart(name, key)          // POST /api/v1/containers/{name}/start
api.containerStop(name, key)           // POST /api/v1/containers/{name}/stop
api.containerRestart(name, key)        // POST /api/v1/containers/{name}/restart
```

**Tipos principais:**

```typescript
type CurrentReading = {
  timestamp: string
  rx_power_dbm: number
  tx_power_dbm?: number | null
  temperature_c?: number | null
  voltage_v?: number | null
  bias_ma?: number | null
  signal_quality: string
  module: ModuleInfo
}

type ContainerInfo = {
  id: string
  name: string
  status: string
  image: string
}

type ContainerStats = {
  cpu_percent: number
  memory_mb: number
  memory_percent: number
}
```

## Nginx

O `nginx.conf` serve como reverse proxy:

```nginx
location /api/ {
    proxy_pass http://api:8000/;   # Container optic-api
}

location / {
    try_files $uri $uri/ /index.html;  # SPA fallback
}
```

Isso permite que o frontend em `:8080` acesse a API sem CORS. O cliente usa sempre caminhos relativos (`/api/v1/...`).

## Página de Sistema (`/system`)

Lista todos os containers Docker com:

- **Badge de status**: `running` (verde), `exited` (vermelho), outros (cinza)
- **Imagem e ID** do container
- **CPU%** e **RAM (MB / %)** em tempo real
- **Botões**: Start / Stop / Restart (requerem `CONTAINER_API_KEY` configurada na API)
- **Dialog de logs**: últimas 100 linhas com timestamps

## Troubleshooting

| Problema | Causa | Solução |
|---|---|---|
| Dashboard não atualiza | API offline ou CORS | Verificar `docker compose ps` e nginx |
| `routeTree.gen.ts` desatualizado | Rota adicionada manualmente | Rodar `pnpm dev` para regenerar |
| Build falha com erro TS | `ignoreDeprecations` inválido | Usar `"5.0"` em `tsconfig.json`, não `"6.0"` |
| Ícone não encontrado | lucide-react v0.562 não tem o ícone | Substituir por alternativa disponível (ex: `MemoryStick` → `HardDrive`) |
| Container stats não carregam | Docker socket não montado na API | Verificar `volumes` no docker-compose.yml |
