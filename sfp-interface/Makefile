# Makefile para SFP Interface - Raspberry Pi (Linux)
# Compilação para Linux embarcado usando a interface I2C do kernel

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -fPIC -I. -Idaemon
LDFLAGS = -lm -lpthread

# Flags específicos para arquivos do daemon
DAEMON_CFLAGS = $(CFLAGS) -Idaemon

# cJSON via pkg-config
CJSON_CFLAGS = $(shell pkg-config --cflags libcjson 2>/dev/null || echo "-I/usr/include")
CJSON_LDFLAGS = $(shell pkg-config --libs libcjson 2>/dev/null || echo "-lcjson")

CFLAGS += $(CJSON_CFLAGS)
LDFLAGS += $(CJSON_LDFLAGS)

LIB_TARGET = libsfp.so
LIB_SRCS = a0h.c a2h.c i2c.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# Para debug, descomente a linha abaixo
# CFLAGS += -DDEBUG -g

TARGET = sfp-reader
SRCS = main.c a0h.c a2h.c sfp_init.c i2c.c
OBJS = $(SRCS:.c=.o)

# Daemon
DAEMON_TARGET = sfp-daemon
DAEMON_SRCS = daemon/daemon_main.c \
              daemon/daemon_config.c \
              daemon/daemon_state.c \
              daemon/daemon_fsm.c \
              daemon/daemon_i2c.c \
              daemon/daemon_socket.c \
              a0h.c \
              a2h.c \
              sfp_init.c \
              i2c.c
DAEMON_OBJS = $(DAEMON_SRCS:.c=.o)

.PHONY: all clean install debug lib daemon

lib: $(LIB_TARGET)

all: $(TARGET)

daemon: $(DAEMON_TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(DAEMON_TARGET): $(DAEMON_OBJS)
	$(CC) $(DAEMON_CFLAGS) -o $@ $^ $(LDFLAGS)

$(LIB_TARGET): $(LIB_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Regra especial para arquivos do daemon
daemon/%.o: daemon/%.c
	$(CC) $(DAEMON_CFLAGS) -c -o $@ $<

debug: CFLAGS += -DDEBUG -g
debug: clean $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(LIB_TARGET)
	rm -f $(DAEMON_OBJS) $(DAEMON_TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Instalado em /usr/local/bin/$(TARGET)"

install-daemon: $(DAEMON_TARGET)
	sudo cp $(DAEMON_TARGET) /usr/local/bin/
	@echo "Instalado em /usr/local/bin/$(DAEMON_TARGET)"

# Dependências
main.o: main.c a0h.h a2h.h i2c.h
a0h.o: a0h.c a0h.h
a2h.o: a2h.c a2h.h
i2c.o: i2c.c i2c.h
sfp_init.o: sfp_init.c sfp_init.h a0h.h a2h.h i2c.h

# Dependências do daemon
daemon/daemon_main.o: daemon/daemon_main.c daemon/daemon_config.h daemon/daemon_state.h daemon/daemon_fsm.h daemon/daemon_i2c.h daemon/daemon_socket.h sfp_init.h
daemon/daemon_config.o: daemon/daemon_config.c daemon/daemon_config.h
daemon/daemon_state.o: daemon/daemon_state.c daemon/daemon_state.h a0h.h a2h.h
daemon/daemon_fsm.o: daemon/daemon_fsm.c daemon/daemon_fsm.h daemon/daemon_state.h
daemon/daemon_i2c.o: daemon/daemon_i2c.c daemon/daemon_i2c.h i2c.h a0h.h a2h.h
daemon/daemon_socket.o: daemon/daemon_socket.c daemon/daemon_socket.h daemon/daemon_state.h daemon/daemon_fsm.h daemon/daemon_config.h
