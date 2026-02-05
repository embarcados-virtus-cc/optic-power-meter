# Display Package - GMT130-V1.0 (USANDO BIBLIOTECA ST7789 CUSTOM)

Este pacote contém os drivers e a aplicação para controlar o display GMT130-V1.0 na Raspberry Pi.

> **NOTA**: Este pacote utiliza uma biblioteca `ST7789` fornecida externamente, baseada em `Adafruit_GPIO` e `numpy`.

## Especificações do Hardware

- **Modelo**: GMT130-V1.0
- **Driver Info**: [ST7789](https://www.rhydolabz.com/documents/33/ST7789.pdf)
- **Interface**: SPI (4-wire)
- **Resolução**: 240x240 pixels
- **Tensão de Operação**: 3.3V

## Pinagem (Raspberry Pi GPIO)

Para conectar o display à Raspberry Pi, utilize a seguinte configuração de pinos.

| Pino Display | Função | Pino Físico RPi | GPIO (BCM) | Descrição |
|--------------|--------|-----------------|------------|-----------|
| **VCC**      | Power  | 1 ou 17         | -          | 3.3V Power|
| **GND**      | Terra  | 6, 9, etc.      | -          | Ground    |
| **SCL**      | Clock  | 23              | GPIO 11    | SPI SCLK  |
| **SDA**      | Data   | 19              | GPIO 10    | SPI MOSI  |
| **RES**      | Reset  | 13              | GPIO 27    | Reset     |
| **DC**       | Data/Cmd| 22             | GPIO 25    | Data/Command Select |
| **BLK**      | Backlight| 18            | GPIO 24    | Controle de Brilho |

> **Nota**: O pino CS não é usado (o driver assume Hardware SPI Port 0 Device 0).

## Dependências

Instale as dependências listadas em `requirements.txt`:

```bash
pip install -r requirements.txt
```

Principais pacotes:
- `Adafruit-GPIO`: Controle de GPIO legado.
- `numpy`: Processamento de imagem rápido.
- `Pillow`: Manipulação de imagem.

## Estrutura do Código

- `ST7789/`: Pacote da biblioteca do driver.
- `main.py`: Aplicação principal. Execute com **sudo**.

```bash
cd display
sudo python main.py
```
