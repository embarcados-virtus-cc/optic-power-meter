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
   - Consumido pela API Python (`packages/api`) que expõe endpoints HTTP REST

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
  - Funções de transição entre estados (Correção bug INIT->ABSENT)
  - Logging de transições via syslog

- **daemon_i2c.h/c**: Wrapper I²C para o daemon
  - Detecção de presença (0x50 e 0x51)
  - Leitura de A0h e A2h
  - Reutiliza código existente (`i2c.c`)

- **daemon_socket.h/c**: Servidor UNIX socket
  - Aceita múltiplas conexões (limitado a 10)
  - Configurado com permissões 0666 (acesso não-root permitido)
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

### Estrutura Completa do JSON

#### GET CURRENT / GET STATIC - Campos A0h (Dados Estáticos)

Todos os campos da página A0h são retornados:

- `identifier`: Byte 0 - Tipo numérico (0x03 = SFP, etc)
- `identifier_type`: String do tipo ("SFP/SFP+", "GBIC", "QSFP", etc)
- `ext_identifier`: Byte 1 - Extended Identifier (valor numérico)
- `ext_identifier_valid`: Validação do Extended Identifier
- `connector`: Byte 2 - Tipo de conector (valor numérico)
- `connector_type`: String do tipo de conector ("LC", "SC", etc)
- `compliance_codes`: Objeto com todos os bytes 3-10 decodificados:
  - `byte3_ethernet_infiniband`: Ethernet 10G e InfiniBand
  - `byte4_escon_sonet`: ESCON e SONET
  - `byte5_sonet`: SONET adicional
  - `byte6_ethernet_1g`: Ethernet 1G
  - `byte7_fc_link_length`: Fibre Channel Link Length
  - `byte8_fc_technology`: FC Technology e Cable Technology
  - `byte9_fc_transmission_media`: FC Transmission Media
  - `byte10_fc_channel_speed`: FC Channel Speed
- `encoding`: Byte 11 - Encoding (8B/10B, NRZ, etc)
- `nominal_rate_mbd`: Byte 12 - Taxa nominal em MBd
- `nominal_rate_status`: Status da taxa nominal
- `rate_identifier`: Byte 13 - Rate Identifier
- `smf_length_km`: Byte 14 - Comprimento SMF em km
- `smf_length_status`: Status do comprimento SMF
- `smf_attenuation_db_per_100m`: Atenuação SMF em dB/100m
- `om2_length_m`: Byte 16 - Comprimento OM2 em metros
- `om2_length_status`: Status do comprimento OM2
- `om1_length_m`: Byte 17 - Comprimento OM1 em metros
- `om1_length_status`: Status do comprimento OM1
- `om4_or_copper_length_m`: Byte 18 - Comprimento OM4 ou cobre em metros
- `om4_or_copper_length_status`: Status do comprimento OM4/cobre
- `vendor_name`: Bytes 20-35 - Nome do fabricante
- `vendor_name_valid`: Validação do nome do fabricante
- `ext_compliance_code`: Byte 36 - Código de conformidade estendida
- `ext_compliance_desc`: Descrição do código de conformidade estendida
- `vendor_oui`: Bytes 37-39 - OUI do fabricante (array [byte1, byte2, byte3])
- `vendor_oui_u32`: OUI do fabricante como uint32
- `vendor_oui_valid`: Validação do OUI
- `vendor_pn`: Bytes 40-55 - Part Number do fabricante
- `vendor_pn_valid`: Validação do Part Number
- `vendor_rev`: Bytes 56-59 - Revisão do fabricante
- `variant`: Variante do módulo (OPTICAL, PASSIVE_CABLE, ACTIVE_CABLE)
- `wavelength_nm`: Comprimento de onda em nm (se óptico)
- `cable_compliance`: Cable compliance (se cabo)
- `fc_speed_2_valid`: Validação do Fibre Channel Speed 2
- `fc_speed_2`: Byte 62 - Fibre Channel Speed 2 (se válido)
- `cc_base_valid`: Validação do checksum CC_BASE
- `cc_base`: Byte 63 - Checksum CC_BASE

#### GET CURRENT / GET DYNAMIC - Campos A2h (Diagnósticos)

Todos os campos da página A2h são retornados:

- `temperature_valid`: Validação da temperatura
- `temperature_c`: Temperatura em Celsius
- `temperature_raw`: Valor bruto da temperatura
- `voltage_valid`: Validação da tensão
- `voltage_v`: Tensão em Volts
- `voltage_raw`: Valor bruto da tensão
- `bias_current_valid`: Validação da corrente de bias
- `bias_current_ma`: Corrente de bias em mA
- `bias_current_raw`: Valor bruto da corrente de bias
- `tx_power_valid`: Validação da potência TX
- `tx_power_dbm`: Potência TX em dBm
- `tx_power_mw`: Potência TX em mW
- `tx_power_raw`: Valor bruto da potência TX
- `rx_power_valid`: Validação da potência RX
- `rx_power_dbm`: Potência RX em dBm
- `rx_power_mw`: Potência RX em mW
- `rx_power_raw`: Valor bruto da potência RX
- `alarms`: Objeto com todos os alarmes:
  - `temperature`: {high, low}
  - `voltage`: {high, low}
  - `bias_current`: {high, low}
  - `tx_power`: {high, low}
  - `rx_power`: {high, low}
- `warnings`: Objeto com todos os warnings:
  - `temperature`: {high, low}
  - `voltage`: {high, low}
  - `bias_current`: {high, low}
  - `tx_power`: {high, low}
  - `rx_power`: {high, low}

### Exemplo de Uso

```bash
# Conectar ao socket
nc -U /run/sfp-daemon/sfp.sock

# Enviar comando
GET CURRENT

# Resposta esperada:
# STATUS 200 OK
# {"status":"ok","state":"PRESENT","generation_id":1,...}
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

---

## Referência Rápida - Campos JSON

### Campos Comuns (Todos os Comandos)

- `status`: "ok", "not_found", "error"
- `state`: "INIT", "ABSENT", "PRESENT", "ERROR"
- `generation_id`: ID incremental (muda quando novo SFP é detectado)
- `timestamps`: Objeto com `first_detected`, `last_a0_read`, `last_a2_read`

### Campos A0h (GET CURRENT, GET STATIC)

**Identificação Básica:**
- `identifier` (uint8): Tipo numérico do módulo
- `identifier_type` (string): "SFP/SFP+", "GBIC", "QSFP", etc
- `ext_identifier` (uint8): Extended Identifier
- `ext_identifier_valid` (bool): Validação
- `connector` (uint8): Tipo numérico do conector
- `connector_type` (string): "LC", "SC", "RJ45", etc

**Compliance Codes (Bytes 3-10):**
Cada byte é um objeto com campos booleanos específicos:
- `byte3_ethernet_infiniband`: eth_10g_base_sr, eth_10g_base_lr, infiniband_1x_sx, etc
- `byte4_escon_sonet`: escon_mmf, escon_smf, oc_192_sr, sonet_rs_1, etc
- `byte5_sonet`: oc_12_sm_lr, oc_12_sr, oc_3_sm_lr, etc
- `byte6_ethernet_1g`: eth_base_px, eth_1000_base_t, eth_1000_base_sx, etc
- `byte7_fc_link_length`: fc_very_long_distance, fc_short_distance, etc
- `byte8_fc_technology`: active_cable, passive_cable, shortwave_laser_sn, etc
- `byte9_fc_transmission_media`: twin_axial_pair, multimode_m5, single_mode, etc
- `byte10_fc_channel_speed`: cs_1200_mbps, cs_800_mbps, cs_1600_mbps, etc

**Características Técnicas:**
- `encoding` (uint8): Tipo de encoding
- `nominal_rate_mbd` (uint8): Taxa nominal em MBd
- `nominal_rate_status` (uint8): Status (0=not_specified, 1=valid, 2=extended)
- `rate_identifier` (uint8): Rate Identifier
- `smf_length_km` (uint16): Comprimento SMF em km
- `smf_length_status` (uint8): Status (0=not_supported, 1=valid, 2=extended)
- `smf_attenuation_db_per_100m` (float): Atenuação SMF
- `om2_length_m` (uint16): Comprimento OM2 em metros
- `om2_length_status` (uint8): Status
- `om1_length_m` (uint16): Comprimento OM1 em metros
- `om1_length_status` (uint8): Status
- `om4_or_copper_length_m` (uint16): Comprimento OM4/cobre em metros
- `om4_or_copper_length_status` (uint8): Status

**Informações do Fabricante:**
- `vendor_name` (string): Nome do fabricante
- `vendor_name_valid` (bool): Validação
- `vendor_oui` (array[3]): OUI como array [byte1, byte2, byte3]
- `vendor_oui_u32` (uint32): OUI como número único
- `vendor_oui_valid` (bool): Validação
- `vendor_pn` (string): Part Number
- `vendor_pn_valid` (bool): Validação
- `vendor_rev` (string): Revisão
- `ext_compliance_code` (uint8): Código de conformidade estendida
- `ext_compliance_desc` (string): Descrição do código

**Outros:**
- `variant` (uint8): 0=OPTICAL, 1=PASSIVE_CABLE, 2=ACTIVE_CABLE
- `wavelength_nm` (uint16): Comprimento de onda (se óptico)
- `cable_compliance` (uint8): Cable compliance (se cabo)
- `fc_speed_2` (uint8): Fibre Channel Speed 2
- `fc_speed_2_valid` (bool): Validação
- `cc_base` (uint8): Checksum CC_BASE
- `cc_base_valid` (bool): Validação do checksum

### Campos A2h (GET CURRENT, GET DYNAMIC)

**Valores Medidos:**
- `temperature_c` (float): Temperatura em Celsius
- `temperature_raw` (int16): Valor bruto
- `temperature_valid` (bool): Validação
- `voltage_v` (float): Tensão em Volts
- `voltage_raw` (uint16): Valor bruto
- `voltage_valid` (bool): Validação
- `bias_current_ma` (float): Corrente de bias em mA
- `bias_current_raw` (uint16): Valor bruto
- `bias_current_valid` (bool): Validação
- `tx_power_dbm` (float): Potência TX em dBm
- `tx_power_mw` (float): Potência TX em mW
- `tx_power_raw` (uint16): Valor bruto
- `tx_power_valid` (bool): Validação
- `rx_power_dbm` (float): Potência RX em dBm
- `rx_power_mw` (float): Potência RX em mW
- `rx_power_raw` (uint16): Valor bruto
- `rx_power_valid` (bool): Validação

**Alarmes:**
Objeto `alarms` com sub-objetos:
- `temperature`: {high: bool, low: bool}
- `voltage`: {high: bool, low: bool}
- `bias_current`: {high: bool, low: bool}
- `tx_power`: {high: bool, low: bool}
- `rx_power`: {high: bool, low: bool}

**Warnings:**
Objeto `warnings` com mesma estrutura de `alarms`

### Notas Importantes

- Todos os campos numéricos podem estar ausentes se `valid: false`
- Campos de compliance codes são objetos com múltiplos campos booleanos
- Valores raw são sempre retornados quando disponíveis
- Valores convertidos (Celsius, Volts, dBm, etc) são calculados a partir dos raw
- Timestamps são Unix timestamps (segundos desde epoch)
