/**
 * @file daemon_config.h
 * @brief Configurações do daemon SFP
 */

#ifndef DAEMON_CONFIG_H
#define DAEMON_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <syslog.h>

/* ============================================
 * Configurações de I²C
 * ============================================ */
#define DAEMON_DEFAULT_I2C_DEVICE "/dev/i2c-1"
#define DAEMON_DEFAULT_I2C_ADDR_A0 0x50
#define DAEMON_DEFAULT_I2C_ADDR_A2 0x51

/* ============================================
 * Configurações de Socket
 * ============================================ */
#define DAEMON_DEFAULT_SOCKET_DIR "/run/sfp-daemon"
#define DAEMON_DEFAULT_SOCKET_PATH "/run/sfp-daemon/sfp.sock"
#define DAEMON_DEFAULT_SOCKET_PERMISSIONS 0660
#define DAEMON_MAX_CONNECTIONS 10

/* ============================================
 * Configurações de Polling
 * ============================================ */
#define DAEMON_POLL_ABSENT_MS 500      /* Polling quando ABSENT (ms) */
#define DAEMON_POLL_PRESENT_MS 2000    /* Polling quando PRESENT (ms) */
#define DAEMON_POLL_ERROR_MS 5000       /* Polling quando ERROR (ms) */
#define DAEMON_PRESENCE_CHECK_INTERVAL_MS 5000  /* Verificar presença a cada 5s quando PRESENT */

/* ============================================
 * Configurações de Erro e Recuperação
 * ============================================ */
#define DAEMON_MAX_I2C_ERRORS 3
#define DAEMON_MAX_RECOVERY_ATTEMPTS 10
#define DAEMON_RECOVERY_TIMEOUT_SEC 30

/* ============================================
 * Configurações de Logging
 * ============================================ */
#define DAEMON_LOG_FACILITY LOG_DAEMON
#define DAEMON_LOG_IDENT "sfp-daemon"

/* ============================================
 * Configurações de Arquivo
 * ============================================ */
#define DAEMON_DEFAULT_CONFIG_FILE "/etc/sfp-daemon.conf"

/* ============================================
 * Estrutura de Configuração
 * ============================================ */
typedef struct {
    char i2c_device[256];
    char socket_path[256];
    uint32_t poll_absent_ms;
    uint32_t poll_present_ms;
    uint32_t poll_error_ms;
    uint32_t max_i2c_errors;
    uint32_t max_recovery_attempts;
    uint32_t max_connections;
    bool daemonize;
} daemon_config_t;

/* ============================================
 * Funções de Configuração
 * ============================================ */

/**
 * @brief Carrega configuração do arquivo ou usa valores padrão
 * @param config Ponteiro para estrutura de configuração
 * @param config_file Caminho do arquivo de configuração (NULL para usar padrão)
 * @return true se carregado com sucesso, false caso contrário
 */
bool daemon_config_load(daemon_config_t *config, const char *config_file);

/**
 * @brief Obtém configuração padrão
 * @param config Ponteiro para estrutura de configuração
 */
void daemon_config_get_defaults(daemon_config_t *config);

#endif /* DAEMON_CONFIG_H */
