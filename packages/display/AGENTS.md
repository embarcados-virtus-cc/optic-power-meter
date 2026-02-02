# Display Package - GMT130-V1.0 (ST7789)

Este pacote contém os drivers e a aplicação para controlar o display GMT130-V1.0 na Raspberry Pi.

## Especificações do Hardware

- **Modelo**: GMT130-V1.0
- **Driver Info**: [ST7789](https://www.rhydolabz.com/documents/33/ST7789.pdf)
- **Interface**: SPI (4-wire)
- **Resolução**: 240x240 pixels
- **Tamanho**: 1.3 polegadas
- **Tensão de Operação**: 3.3V

## Pinagem (Raspberry Pi GPIO)

Para conectar o display à Raspberry Pi, utilize a seguinte configuração de pinos. A numeração física (Physical Pin) refere-se ao header de 40 pinos padrão.

| Pino Display | Função | Pino Físico RPi | GPIO (BCM) | Descrição |
|--------------|--------|-----------------|------------|-----------|
| **GND**      | Terra  | 6, 9, 14, 20... | -          | Ground    |
| **VCC**      | Power  | 1 ou 17         | -          | 3.3V Power|
| **SCL**      | Clock  | 23              | GPIO 11    | SPI SCLK  |
| **SDA**      | Data   | 19              | GPIO 10    | SPI MOSI  |
| **RES**      | Reset  | 13              | GPIO 27    | Reset     |
| **DC**       | Data/Cmd| 22             | GPIO 25    | Data/Command Select |
| **BLK**      | Backlight| 18            | GPIO 24    | Controle de Brilho (Opcional) |

> **Nota**: Se o pino BLK (Backlight) for deixado desconectado, o display geralmente liga com brilho máximo, ou pode não ligar dependendo do módulo. Recomenda-se conectar ao 3.3V ou a um pino GPIO para controle.

## Habilitando SPI na Raspberry Pi

Certifique-se de que a interface SPI está habilitada no `raspi-config`:

1. Execute `sudo raspi-config`
2. Vá em `Interface Options`
3. Selecione `SPI`
4. Escolha `Yes` para habilitar
5. Reinicie se necessário (`sudo reboot`)

## Estrutura do Código

- `st7789.py`: Driver de baixo nível para comunicação com o chip ST7789.
- `main.py`: Aplicação principal que desenha a interface e exibe as informações de rede.

## Dependências

Instale as dependências listadas em `requirements.txt`:

```bash
pip install -r requirements.txt
```

As principais bibliotecas são:
- `lgpio`: Para controle dos pinos GPIO (substitui RPi.GPIO para compatibilidade moderna).
- `spidev`: Para comunicação SPI via hardware.
- `Pillow`: Para criação e manipulação de imagens a serem enviadas ao display.
- `netifaces`: Para obter o endereço IP da interface de rede.
