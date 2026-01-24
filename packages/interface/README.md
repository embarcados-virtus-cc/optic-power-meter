# SFP Interface para Raspberry Pi (Linux)

Biblioteca em C para leitura de módulos SFP/SFP+ via I2C no Raspberry Pi, conforme especificação **SFF-8472**.

## Pré-requisitos

### 1. Habilitar I2C no Raspberry Pi

```bash
sudo raspi-config
```

Navegue até: `Interface Options` → `I2C` → `Enable`

### 2. Instalar dependências

```bash
sudo apt update
sudo apt install -y i2c-tools libi2c-dev build-essential
```

### 3. Verificar permissões

Adicione seu usuário ao grupo `i2c`:

```bash
sudo usermod -aG i2c $USER
```

**Reinicie a sessão** ou faça logout/login para aplicar.

## Compilação

```bash
make
```

Para compilar com debug (dump hexadecimal da EEPROM):

```bash
make debug
```

## Uso

```bash
# Executar (padrão: /dev/i2c-1)
./sfp_reader

# Especificar outro barramento I2C
./sfp_reader /dev/i2c-0

# Executar como root (se não tiver permissão)
sudo ./sfp_reader
```

## Verificar dispositivo I2C

Antes de executar, verifique se o módulo SFP é detectado:

```bash
i2cdetect -y 1
```

Você deve ver os endereços `50` e `51` (0x50 e 0x51) ocupados.

## Referências

- [SFF-8472 Specification](https://members.snia.org/document/dl/25916)
- [SFF-8024 Specification](https://members.snia.org/document/dl/26423)
- [Linux I2C Documentation](https://www.kernel.org/doc/html/latest/i2c/index.html)
