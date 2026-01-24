# Makefile para SFP Interface - Raspberry Pi (Linux)
# Compilação para Linux embarcado usando a interface I2C do kernel

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -fPIC -lm
LDFLAGS = -lm
LIB_TARGET = libsfp.so
LIB_SRCS = sfp_8472_a0h.c sfp_8472_a2h.c i2c.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# Para debug, descomente a linha abaixo
# CFLAGS += -DDEBUG -g

TARGET = sfp_reader
SRCS = main.c sfp_8472_a0h.c sfp_8472_a2h.c i2c.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean install debug lib

lib: $(LIB_TARGET)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(LIB_TARGET): $(LIB_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

debug: CFLAGS += -DDEBUG -g
debug: clean $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(LIB_TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Instalado em /usr/local/bin/$(TARGET)"

# Dependências
main.o: main.c sfp_8472_a0h.h sfp_8472_a2h.h i2c.h
sfp_8472_a0h.o: sfp_8472_a0h.c sfp_8472_a0h.h
sfp_8472_a2h.o: sfp_8472_a2h.c sfp_8472_a2h.h
i2c.o: i2c.c i2c.h
