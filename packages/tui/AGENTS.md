# TUI para acesso a informações do transceptor SFP

## Estrutura do Projeto

A TUI (Terminal User Interface) é uma aplicação Python que consome o daemon SFP via socket UNIX e apresenta os dados de forma visual e organizada no terminal.

### Arquitetura

```
tui/
├── menu.py              # Interface principal com prompt_toolkit
├── daemon_client.py     # Cliente síncrono para socket UNIX
├── formatters.py        # Formatadores de dados para exibição
├── config.py            # Configurações (socket path, timeout)
├── requirements.txt     # Dependências Python
└── AGENTS.md           # Este arquivo
```

## Dependências

- **prompt_toolkit**: Biblioteca principal para construção da interface TUI
- **pydantic-settings**: Para gerenciamento de configurações

## Padrões de Código

### Nomenclatura

- Módulos: `snake_case` (ex: `daemon_client.py`)
- Classes: `PascalCase` (ex: `DaemonClient`)
- Funções: `snake_case` (ex: `format_static_data`)
- Constantes: `UPPER_SNAKE_CASE` (ex: `UPDATE_INTERVAL`)

### Estrutura de Dados

A TUI consome os mesmos dados do daemon que a API, seguindo o protocolo do socket UNIX:

- **GET CURRENT**: Estado completo (A0h + A2h + metadados)
- **GET STATIC**: Apenas dados A0h (estáticos)
- **GET DYNAMIC**: Apenas dados A2h (dinâmicos)

### Cliente Socket

- Classe `DaemonClient` para comunicação síncrona com o daemon
- Métodos síncronos (não assíncronos, diferente da API)
- Timeout configurável
- Tratamento de erros de conexão e parsing

### Formatadores

O módulo `formatters.py` contém funções para formatar dados:

- `format_static_data()`: Formata dados A0h (configurações estáticas)
- `format_dynamic_data()`: Formata dados A2h (valores dinâmicos de potência)
- `format_summary_data()`: Formata resumo completo do transceptor
- Funções auxiliares: `format_timestamp()`, `format_float()`, `format_int()`, etc.

### Atualização de Dados

- Thread em background atualiza dados a cada 2 segundos
- Cache thread-safe usando `threading.Lock`
- UI atualiza automaticamente via `refresh_interval`
- Atualização manual com tecla `r`

## Interface

### Menu Principal

A interface possui 4 opções principais:

1. **Configurações do Transceptor**: Exibe dados estáticos A0h
   - Identificação (tipo, conector, variante)
   - Informações do fabricante (nome, part number, OUI, revisão)
   - Características técnicas (encoding, taxa nominal, comprimento de onda)
   - Comprimentos de fibra (SMF, OM1, OM2, OM4)
   - Códigos de compliance

2. **Valores Atuais de Potência Óptica**: Exibe dados dinâmicos A2h
   - Valores medidos (temperatura, tensão, corrente de bias)
   - Potência de transmissão (TX) em dBm e mW
   - Potência de recepção (RX) em dBm e mW
   - Alarmes e warnings

3. **Informações do Transceptor**: Resumo completo
   - Estado do transceptor (PRESENT, ABSENT, ERROR, INIT)
   - Timestamps (primeira detecção, últimas leituras)
   - Resumo de dados estáticos e dinâmicos

4. **Sair**: Encerra a aplicação

### Controles

- **↑/↓**: Navega pelo menu
- **Enter**: Seleciona item (ou sai se "Sair" estiver selecionado)
- **r**: Força atualização manual dos dados
- **q/Escape**: Sai da aplicação

### Status Bar

Barra de status na parte inferior mostra:
- Status da conexão com o daemon
- Tempo desde última atualização
- Erros de conexão (se houver)

## Estilização

A interface usa cores e formatação para melhorar a legibilidade:

- **Título**: Branco em negrito
- **Seta de seleção**: Roxo (`#c084fc`)
- **Seções**: Roxo em negrito
- **Labels**: Cinza claro (`#94a3b8`)
- **Valores**: Branco acinzentado (`#e2e8f0`)
- **Erros**: Vermelho (`#ef4444`)
- **Avisos**: Laranja (`#f59e0b`)
- **Sucesso**: Verde (`#10b981`)

## Execução

```bash
# Instalar dependências
pip install -r requirements.txt

# Executar TUI
python menu.py
```

## Configuração

A TUI lê configurações de variáveis de ambiente (via `pydantic-settings`):

- `SFP_DAEMON_SOCKET`: Caminho do socket UNIX (padrão: `/run/sfp-daemon/sfp.sock`)
- `SOCKET_TIMEOUT`: Timeout para operações de socket em segundos (padrão: `5.0`)

## Tratamento de Erros

- **SFP não encontrado**: Exibe mensagem de erro e instruções
- **Daemon indisponível**: Mostra erro de conexão no status bar
- **Timeout**: Exibe erro após timeout configurado
- **Dados inválidos**: Mostra "N/A" ou mensagem apropriada

## Notas Importantes

- A TUI usa comunicação síncrona (diferente da API que é assíncrona)
- Dados são atualizados automaticamente em background
- Interface é thread-safe usando locks
- Formatação segue padrões do prompt_toolkit
- Todos os campos do daemon são exibidos conforme protocolo do socket
