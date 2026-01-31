# Configurações para Desenvolvimento Contínuo da interface SFP

## Estrutura do Projeto

O projeto está organizado em duas partes principais:

1. **Biblioteca SFP**: Código de baixo nível para leitura de módulos SFP via I²C
   - `i2c.c/h`: Interface I²C do Linux
   - `sfp_8472_a0h.c/h`: Parsing da página A0h (dados estáticos)
   - `sfp_8472_a2h.c/h`: Parsing da página A2h (diagnósticos dinâmicos)
   - `sfp_init.c/h`: Inicialização e leitura completa
   - `main.c`: Aplicação standalone de teste

2. **Daemon SFP**: Processo residente que monitora o SFP continuamente
   - `daemon/`: Diretório com toda a lógica do daemon
   - Expõe dados via UNIX socket em `/run/sfp-daemon/sfp.sock`
   - Protocolo simples: comandos textuais com respostas JSON

## Declaração de Funções

Todas as funções precisam ser declaradas em seu devido header, seguindo o padrão da biblioteca para raspagem de bits, onde temos duas funções:
- Parse: Função responsável para fazer a captura do byte desejado
- Get: Função getter que apenas retorna o valor do byte desejado

*Sempre são necessárias essas duas funções para cada uma das sessões de bytes, no entanto, podem haver algumas funções auxiliares, que devem ser descritas também.*

### Exemplo de declaração de funções:
``` C
void sfp_parse_a0_base_identifier(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
sfp_identifier_t sfp_a0_get_identifier(const sfp_a0h_base_t *a0);
```
*Sempre seguir esse padrão de nomenclatura das funções.*

---

## Lógica de Comentários

Todas as funções precisam de um comentário que explique o que ela está relacionada, bem como o que ela faz. Não é necessário ser um comentário longo e descritivo, apenas algum referencial.

### Exemplo de comentário de função:
``` C
/* ============================================
 * Descrição da Função
 * ============================================ */
```
*Sempre seguir esse padrão de comentaŕios das funções*

---

## Estrutura do Daemon

O daemon está organizado em módulos dentro do diretório `daemon/`:

### Módulos Principais

- **daemon_config.h/c**: Configuração do daemon (paths, timeouts, etc)
  - Suporta arquivo de configuração em `/etc/sfp-daemon.conf`
  - Valores padrão definidos em `daemon_config.h`

- **daemon_state.h/c**: Estado global thread-safe
  - Estrutura `sfp_daemon_state_data_t` com mutex
  - Funções para obter cópia thread-safe do estado
  - Cálculo de hash para detecção de mudança de SFP

- **daemon_fsm.h/c**: Máquina de estados
  - Estados: INIT, ABSENT, PRESENT, ERROR
  - Funções de transição entre estados
  - Logging de transições via syslog

- **daemon_i2c.h/c**: Wrapper I²C para o daemon
  - Detecção de presença (0x50 e 0x51)
  - Leitura de A0h e A2h
  - Reutiliza código existente (`i2c.c`)

- **daemon_socket.h/c**: Servidor UNIX socket
  - Aceita múltiplas conexões (limitado a 10)
  - Protocolo textual: `GET CURRENT`, `GET STATIC`, `GET DYNAMIC`, `GET STATE`, `PING`
  - Respostas em JSON usando cJSON
  - Formato: `STATUS <code> <message>\n<JSON>\n`

- **daemon_main.c**: Entry point e loop principal
  - Daemonização (fork + setsid)
  - Loop principal com polling adaptativo
  - Tratamento de sinais (SIGTERM, SIGINT)
  - Logging via syslog

### Compilação

```bash
# Compilar apenas o daemon
make daemon

# Compilar tudo (incluindo aplicação standalone)
make all

# Instalar daemon
sudo make install-daemon
```

### Dependências

- cJSON: Biblioteca para serialização JSON
  - Instalação: `sudo apt-get install libcjson-dev` (Debian/Ubuntu)
  - Ou via pkg-config: `pkg-config --libs libcjson`

- pthread: Para mutex (thread-safety)
  - Incluído na libc padrão

---

## Padrões de Código

### Nomenclatura

- Funções de parsing: `sfp_parse_*`
- Funções getter: `sfp_*_get_*`
- Funções do daemon: `daemon_*`
- Estruturas: `sfp_*_t` ou `daemon_*_t`

### Thread-Safety

- Sempre usar mutex ao acessar estado global
- Usar `daemon_state_get_copy()` para leitura thread-safe
- Nunca acessar estado diretamente sem lock

### Logging

- Usar syslog para mensagens do daemon
- Níveis: LOG_INFO (normal), LOG_WARNING (erros recuperáveis), LOG_ERR (erros críticos)
- Não usar printf/fprintf no daemon (exceto em modo foreground)

### Tratamento de Erros

- Sempre verificar retornos de funções
- Distinguir entre erros temporários e permanentes
- Implementar retry com backoff exponencial quando apropriado

---

## Protocolo do Socket

### Comandos Suportados

- `GET CURRENT`: Estado completo (A0h + A2h + metadados)
- `GET STATIC`: Apenas dados A0h (estáticos)
- `GET DYNAMIC`: Apenas dados A2h (dinâmicos)
- `GET STATE`: Apenas estado da FSM e timestamps
- `PING`: Health check com uptime

### Formato de Resposta

```
STATUS <code> <message>
<JSON>
```

Códigos de status:
- `200 OK`: Sucesso
- `404 NOT_FOUND`: SFP ausente
- `500 ERROR`: Erro interno
- `400 BAD_REQUEST`: Comando inválido

### Exemplo de Uso

```bash
# Conectar ao socket
nc -U /run/sfp-daemon/sfp.sock

# Enviar comando
GET CURRENT

# Resposta esperada:
# STATUS 200 OK
# {"status":"ok","state":"PRESENT",...}
```

---

## Configuração

Arquivo de configuração opcional: `/etc/sfp-daemon.conf`

Formato:
```
i2c_device=/dev/i2c-1
socket_path=/run/sfp-daemon/sfp.sock
poll_absent_ms=500
poll_present_ms=2000
poll_error_ms=5000
max_i2c_errors=3
max_recovery_attempts=10
max_connections=10
daemonize=true
```

Se o arquivo não existir, valores padrão são usados (definidos em `daemon_config.h`).
