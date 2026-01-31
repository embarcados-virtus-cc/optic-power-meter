/**
 * @file daemon_config.c
 * @brief Implementação das funções de configuração do daemon
 */

#include "daemon_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

/* ============================================
 * Carrega Configuração
 * ============================================ */
bool daemon_config_load(daemon_config_t *config, const char *config_file)
{
    if (!config) {
        return false;
    }

    /* Inicializa com valores padrão */
    daemon_config_get_defaults(config);

    /* Se não especificou arquivo, usa padrão */
    if (!config_file) {
        config_file = DAEMON_DEFAULT_CONFIG_FILE;
    }

    /* Tenta abrir arquivo de configuração */
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        /* Arquivo não existe ou não pode ser lido - usa padrões */
        syslog(LOG_INFO, "Config file not found, using defaults: %s", config_file);
        return true;
    }

    /* Lê arquivo linha por linha (formato simples: chave=valor) */
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        /* Remove comentários e espaços */
        char *p = line;
        while (*p && (*p == ' ' || *p == '\t')) p++;
        if (*p == '#' || *p == '\n' || *p == '\0') continue;

        /* Remove newline */
        char *nl = strchr(p, '\n');
        if (nl) *nl = '\0';

        /* Procura por '=' */
        char *eq = strchr(p, '=');
        if (!eq) continue;
        *eq++ = '\0';

        /* Remove espaços do valor */
        while (*eq == ' ' || *eq == '\t') eq++;

        /* Processa configurações conhecidas */
        if (strcmp(p, "i2c_device") == 0) {
            strncpy(config->i2c_device, eq, sizeof(config->i2c_device) - 1);
            config->i2c_device[sizeof(config->i2c_device) - 1] = '\0';
        } else if (strcmp(p, "socket_path") == 0) {
            strncpy(config->socket_path, eq, sizeof(config->socket_path) - 1);
            config->socket_path[sizeof(config->socket_path) - 1] = '\0';
        } else if (strcmp(p, "poll_absent_ms") == 0) {
            config->poll_absent_ms = (uint32_t)atoi(eq);
        } else if (strcmp(p, "poll_present_ms") == 0) {
            config->poll_present_ms = (uint32_t)atoi(eq);
        } else if (strcmp(p, "poll_error_ms") == 0) {
            config->poll_error_ms = (uint32_t)atoi(eq);
        } else if (strcmp(p, "max_i2c_errors") == 0) {
            config->max_i2c_errors = (uint32_t)atoi(eq);
        } else if (strcmp(p, "max_recovery_attempts") == 0) {
            config->max_recovery_attempts = (uint32_t)atoi(eq);
        } else if (strcmp(p, "max_connections") == 0) {
            config->max_connections = (uint32_t)atoi(eq);
        } else if (strcmp(p, "daemonize") == 0) {
            config->daemonize = (strcmp(eq, "true") == 0 || strcmp(eq, "1") == 0);
        }
    }

    fclose(fp);
    syslog(LOG_INFO, "Config loaded from: %s", config_file);
    return true;
}

/* ============================================
 * Obtém Configuração Padrão
 * ============================================ */
void daemon_config_get_defaults(daemon_config_t *config)
{
    if (!config) {
        return;
    }

    strncpy(config->i2c_device, DAEMON_DEFAULT_I2C_DEVICE, sizeof(config->i2c_device) - 1);
    config->i2c_device[sizeof(config->i2c_device) - 1] = '\0';

    strncpy(config->socket_path, DAEMON_DEFAULT_SOCKET_PATH, sizeof(config->socket_path) - 1);
    config->socket_path[sizeof(config->socket_path) - 1] = '\0';

    config->poll_absent_ms = DAEMON_POLL_ABSENT_MS;
    config->poll_present_ms = DAEMON_POLL_PRESENT_MS;
    config->poll_error_ms = DAEMON_POLL_ERROR_MS;
    config->max_i2c_errors = DAEMON_MAX_I2C_ERRORS;
    config->max_recovery_attempts = DAEMON_MAX_RECOVERY_ATTEMPTS;
    config->max_connections = DAEMON_MAX_CONNECTIONS;
    config->daemonize = true;
}

