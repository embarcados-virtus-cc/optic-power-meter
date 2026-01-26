# Optic Power Meter

Sistema para leitura, monitoramento e visualização de dados de transceptores ópticos SFP/SFP+ utilizando Raspberry Pi.

## Intuito do Projeto
O objetivo é fornecer uma ferramenta capaz de ler parâmetros críticos de redes ópticas — como potência de recepção (Rx Power), temperatura, tensão e informações do fabricante — diretamente do hardware, apresentando-os em uma interface gráfica amigável.

## Estrutura e Tecnologias

O projeto é organizado como um monorepo contendo os seguintes pacotes:

### `packages/sfp-interface` (Driver/Backend)
Responsável pela comunicação de baixo nível com o hardware.
- **Linguagem**: C
- **Protocolo**: I2C (comunicação direta com endereços 0xA0 e 0xA2 do SFP).
- **Função**: Leitura bruta da EEPROM, decodificação dos dados conforme norma SFF-8472 e validação de checksums.

### `packages/view` (Frontend)
Interface gráfica para visualização dos dados.
- **Linguagem**: TypeScript
- **Framework**: React, Vite
- **Estilização**: Tailwind CSS, Shadcn/Radix UI
- **Gráficos**: Recharts
- **Função**: Exibe dashboards com gráficos de potência em tempo real, tabelas de diagnóstico e alarmes de status.

## Arquitetura

1. **Hardware**: O módulo SFP é conectado ao barramento I2C do Raspberry Pi.
2. **Camada de Driver (C)**: O código em `sfp-interface` interage com o driver I2C do kernel Linux para extrair os bytes de diagnóstico.
3. **Camada de Aplicação (View)**: A aplicação web consome os dados processados e renderiza as informações para o usuário final.
