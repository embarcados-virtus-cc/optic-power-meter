/**
 * @file daemon_socket.c
 * @brief Implementação do servidor socket e serialização JSON
 */

#include "daemon_socket.h"
#include "daemon_fsm.h"
#include "daemon_state.h"
#include <cjson/cJSON.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <sys/stat.h>

/* ============================================
 * Inicializa Servidor Socket
 * ============================================ */
bool daemon_socket_init(daemon_socket_server_t *server, const daemon_config_t *config)
{
    if (!server || !config) {
        return false;
    }
    
    memset(server, 0, sizeof(daemon_socket_server_t));
    strncpy(server->socket_path, config->socket_path, sizeof(server->socket_path) - 1);
    server->socket_path[sizeof(server->socket_path) - 1] = '\0';
    server->server_fd = -1;
    server->num_clients = 0;
    
    for (int i = 0; i < DAEMON_MAX_CONNECTIONS; i++) {
        server->client_fds[i] = -1;
    }
    
    /* Cria diretório do socket se não existir */
    char dir_path[256];
    strncpy(dir_path, server->socket_path, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';
    
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        mkdir(dir_path, 0755);
    }
    
    /* Remove socket antigo se existir */
    unlink(server->socket_path);
    
    /* Cria socket UNIX */
    server->server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        syslog(LOG_ERR, "Failed to create socket: %s", strerror(errno));
        return false;
    }
    
    /* Configura endereço */
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, server->socket_path, sizeof(addr.sun_path) - 1);
    
    /* Bind */
    if (bind(server->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
        close(server->server_fd);
        return false;
    }
    
    /* Configura permissões */
    chmod(server->socket_path, DAEMON_DEFAULT_SOCKET_PERMISSIONS);
    
    /* Listen */
    if (listen(server->server_fd, DAEMON_MAX_CONNECTIONS) < 0) {
        syslog(LOG_ERR, "Failed to listen: %s", strerror(errno));
        close(server->server_fd);
        return false;
    }
    
    /* Non-blocking */
    int flags = fcntl(server->server_fd, F_GETFL, 0);
    fcntl(server->server_fd, F_SETFL, flags | O_NONBLOCK);
    
    syslog(LOG_INFO, "Socket server initialized: %s", server->socket_path);
    return true;
}

/* ============================================
 * Libera Recursos
 * ============================================ */
void daemon_socket_cleanup(daemon_socket_server_t *server)
{
    if (!server) {
        return;
    }
    
    /* Fecha clientes */
    for (int i = 0; i < DAEMON_MAX_CONNECTIONS; i++) {
        if (server->client_fds[i] >= 0) {
            close(server->client_fds[i]);
            server->client_fds[i] = -1;
        }
    }
    
    /* Fecha servidor */
    if (server->server_fd >= 0) {
        close(server->server_fd);
        server->server_fd = -1;
    }
    
    /* Remove socket */
    unlink(server->socket_path);
    
    syslog(LOG_INFO, "Socket server cleaned up");
}

/* ============================================
 * Aceita Novas Conexões
 * ============================================ */
bool daemon_socket_accept(daemon_socket_server_t *server)
{
    if (!server || server->server_fd < 0) {
        return false;
    }
    
    if (server->num_clients >= DAEMON_MAX_CONNECTIONS) {
        return false;  /* Limite atingido */
    }
    
    int client_fd = accept(server->server_fd, NULL, NULL);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            syslog(LOG_WARNING, "Accept failed: %s", strerror(errno));
        }
        return false;
    }
    
    /* Non-blocking */
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    
    /* Adiciona à lista */
    for (int i = 0; i < DAEMON_MAX_CONNECTIONS; i++) {
        if (server->client_fds[i] < 0) {
            server->client_fds[i] = client_fd;
            server->num_clients++;
            syslog(LOG_DEBUG, "Client connected (fd: %d)", client_fd);
            return true;
        }
    }
    
    /* Não há espaço */
    close(client_fd);
    return false;
}

/* ============================================
 * Processa Comando de Cliente
 * ============================================ */
static void daemon_socket_process_client_command(int client_fd, sfp_daemon_state_data_t *state, const char *command, time_t daemon_uptime)
{
    if (!command || !state) {
        return;
    }
    
    char *json_response = NULL;
    int status_code = 200;
    const char *status_msg = "OK";
    
    /* Remove newline */
    char cmd[256];
    strncpy(cmd, command, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';
    char *nl = strchr(cmd, '\n');
    if (nl) *nl = '\0';
    
    /* Remove espaços */
    char *p = cmd;
    while (*p == ' ' || *p == '\t') p++;
    
    /* Processa comando */
    if (strcmp(p, "GET CURRENT") == 0) {
        json_response = daemon_socket_serialize_current(state);
        if (!json_response) {
            status_code = 500;
            status_msg = "ERROR";
        }
    } else if (strcmp(p, "GET STATIC") == 0) {
        json_response = daemon_socket_serialize_static(state);
        if (!json_response) {
            status_code = 500;
            status_msg = "ERROR";
        }
    } else if (strcmp(p, "GET DYNAMIC") == 0) {
        json_response = daemon_socket_serialize_dynamic(state);
        if (!json_response) {
            status_code = 500;
            status_msg = "ERROR";
        }
    } else if (strcmp(p, "GET STATE") == 0) {
        json_response = daemon_socket_serialize_state(state);
        if (!json_response) {
            status_code = 500;
            status_msg = "ERROR";
        }
    } else if (strcmp(p, "PING") == 0) {
        json_response = daemon_socket_serialize_ping(daemon_uptime);
        if (!json_response) {
            status_code = 500;
            status_msg = "ERROR";
        }
    } else {
        status_code = 400;
        status_msg = "BAD_REQUEST";
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "status", "error");
        cJSON_AddStringToObject(json, "message", "Invalid command");
        json_response = cJSON_Print(json);
        cJSON_Delete(json);
    }
    
    /* Envia resposta */
    if (json_response) {
        char status_line[256];
        snprintf(status_line, sizeof(status_line), "STATUS %d %s\n", status_code, status_msg);
        send(client_fd, status_line, strlen(status_line), 0);
        send(client_fd, json_response, strlen(json_response), 0);
        send(client_fd, "\n", 1, 0);
        free(json_response);
    }
}

/* ============================================
 * Processa Comandos Pendentes
 * ============================================ */
int daemon_socket_process_commands(daemon_socket_server_t *server, sfp_daemon_state_data_t *state, time_t daemon_uptime)
{
    if (!server || !state) {
        return 0;
    }
    
    int processed = 0;
    
    for (int i = 0; i < DAEMON_MAX_CONNECTIONS; i++) {
        if (server->client_fds[i] < 0) {
            continue;
        }
        
        char buffer[1024];
        ssize_t bytes_read = recv(server->client_fds[i], buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_read < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                /* Erro ou conexão fechada */
                close(server->client_fds[i]);
                server->client_fds[i] = -1;
                server->num_clients--;
            }
            continue;
        }
        
        if (bytes_read == 0) {
            /* Conexão fechada */
            close(server->client_fds[i]);
            server->client_fds[i] = -1;
            server->num_clients--;
            continue;
        }
        
        buffer[bytes_read] = '\0';
        daemon_socket_process_client_command(server->client_fds[i], state, buffer, daemon_uptime);
        processed++;
    }
    
    return processed;
}

/* ============================================
 * Fecha Conexões Inativas
 * ============================================ */
void daemon_socket_close_inactive(daemon_socket_server_t *server)
{
    if (!server) {
        return;
    }
    
    /* Implementação futura: timeout de conexões */
    /* Por enquanto, conexões são fechadas apenas quando cliente desconecta */
}

/* ============================================
 * Serializa Estado Completo
 * ============================================ */
char *daemon_socket_serialize_current(const sfp_daemon_state_data_t *state)
{
    if (!state) {
        return NULL;
    }
    
    cJSON *json = cJSON_CreateObject();
    cJSON *timestamps = cJSON_CreateObject();
    cJSON *a0_obj = NULL;
    cJSON *a2_obj = NULL;
    
    /* Obtém cópia thread-safe */
    sfp_daemon_state_data_t state_copy;
    daemon_state_get_copy((sfp_daemon_state_data_t *)state, &state_copy);
    
    /* Status e estado */
    if (state_copy.state == SFP_STATE_ABSENT) {
        cJSON_AddStringToObject(json, "status", "not_found");
        cJSON_AddStringToObject(json, "message", "SFP not detected on I²C bus");
    } else if (state->state == SFP_STATE_ERROR) {
        cJSON_AddStringToObject(json, "status", "error");
        cJSON_AddStringToObject(json, "message", "I²C error or recovery in progress");
    } else {
        cJSON_AddStringToObject(json, "status", "ok");
    }
    
    cJSON_AddStringToObject(json, "state", daemon_fsm_state_to_string(state_copy.state));
    cJSON_AddNumberToObject(json, "generation_id", (double)state_copy.generation_id);
    
    /* Timestamps */
    cJSON_AddNumberToObject(timestamps, "first_detected", (double)state_copy.first_detected);
    cJSON_AddNumberToObject(timestamps, "last_a0_read", (double)state_copy.last_a0_read);
    cJSON_AddNumberToObject(timestamps, "last_a2_read", (double)state_copy.last_a2_read);
    cJSON_AddItemToObject(json, "timestamps", timestamps);
    
    /* A0h */
    if (state_copy.a0_valid) {
        a0_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a0_obj, "valid", true);
        cJSON_AddNumberToObject(a0_obj, "identifier", state_copy.a0_parsed.identifier);
        cJSON_AddStringToObject(a0_obj, "vendor_name", state_copy.a0_parsed.vendor_name);
        cJSON_AddStringToObject(a0_obj, "vendor_pn", state_copy.a0_parsed.vendor_pn);
        if (state_copy.a0_parsed.variant == SFP_VARIANT_OPTICAL) {
            cJSON_AddNumberToObject(a0_obj, "wavelength_nm", state_copy.a0_parsed.wavelength_nm);
        }
        cJSON_AddItemToObject(json, "a0", a0_obj);
    } else {
        a0_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a0_obj, "valid", false);
        cJSON_AddItemToObject(json, "a0", a0_obj);
    }
    
    /* A2h */
    if (state_copy.a2_valid) {
        a2_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a2_obj, "valid", true);
        cJSON_AddNumberToObject(a2_obj, "temperature_c", state_copy.a2_parsed.temperature_c);
        cJSON_AddNumberToObject(a2_obj, "voltage_v", state_copy.a2_parsed.voltage_v);
        cJSON_AddNumberToObject(a2_obj, "tx_power_dbm", state_copy.a2_parsed.tx_power_dbm);
        cJSON_AddNumberToObject(a2_obj, "rx_power_dbm", state_copy.a2_parsed.rx_power_dbm);
        
        /* Alarms */
        cJSON *alarms = cJSON_CreateObject();
        cJSON_AddBoolToObject(alarms, "temp_alarm_high", state_copy.a2_parsed.alarms.temp_alarm_high);
        cJSON_AddBoolToObject(alarms, "temp_alarm_low", state_copy.a2_parsed.alarms.temp_alarm_low);
        cJSON_AddBoolToObject(alarms, "rx_power_alarm_low", state_copy.a2_parsed.alarms.rx_power_alarm_low);
        cJSON_AddItemToObject(a2_obj, "alarms", alarms);
        
        cJSON_AddItemToObject(json, "a2", a2_obj);
    } else {
        a2_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a2_obj, "valid", false);
        cJSON_AddItemToObject(json, "a2", a2_obj);
    }
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    return json_string;
}

/* ============================================
 * Serializa Apenas A0h
 * ============================================ */
char *daemon_socket_serialize_static(const sfp_daemon_state_data_t *state)
{
    if (!state) {
        return NULL;
    }
    
    cJSON *json = cJSON_CreateObject();
    cJSON *a0_obj = NULL;
    
    /* Obtém cópia thread-safe */
    sfp_daemon_state_data_t state_copy;
    daemon_state_get_copy((sfp_daemon_state_data_t *)state, &state_copy);
    
    cJSON_AddNumberToObject(json, "generation_id", (double)state_copy.generation_id);
    cJSON_AddNumberToObject(json, "last_a0_read", (double)state_copy.last_a0_read);
    
    if (state_copy.a0_valid) {
        a0_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a0_obj, "valid", true);
        cJSON_AddNumberToObject(a0_obj, "identifier", state_copy.a0_parsed.identifier);
        cJSON_AddStringToObject(a0_obj, "vendor_name", state_copy.a0_parsed.vendor_name);
        cJSON_AddStringToObject(a0_obj, "vendor_pn", state_copy.a0_parsed.vendor_pn);
        if (state_copy.a0_parsed.variant == SFP_VARIANT_OPTICAL) {
            cJSON_AddNumberToObject(a0_obj, "wavelength_nm", state_copy.a0_parsed.wavelength_nm);
        }
        cJSON_AddItemToObject(json, "a0", a0_obj);
    } else {
        a0_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a0_obj, "valid", false);
        cJSON_AddItemToObject(json, "a0", a0_obj);
    }
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    return json_string;
}

/* ============================================
 * Serializa Apenas A2h
 * ============================================ */
char *daemon_socket_serialize_dynamic(const sfp_daemon_state_data_t *state)
{
    if (!state) {
        return NULL;
    }
    
    cJSON *json = cJSON_CreateObject();
    cJSON *a2_obj = NULL;
    
    /* Obtém cópia thread-safe */
    sfp_daemon_state_data_t state_copy;
    daemon_state_get_copy((sfp_daemon_state_data_t *)state, &state_copy);
    
    cJSON_AddNumberToObject(json, "last_a2_read", (double)state_copy.last_a2_read);
    
    if (state_copy.a2_valid) {
        a2_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a2_obj, "valid", true);
        cJSON_AddNumberToObject(a2_obj, "temperature_c", state_copy.a2_parsed.temperature_c);
        cJSON_AddNumberToObject(a2_obj, "voltage_v", state_copy.a2_parsed.voltage_v);
        cJSON_AddNumberToObject(a2_obj, "tx_power_dbm", state_copy.a2_parsed.tx_power_dbm);
        cJSON_AddNumberToObject(a2_obj, "rx_power_dbm", state_copy.a2_parsed.rx_power_dbm);
        
        /* Alarms */
        cJSON *alarms = cJSON_CreateObject();
        cJSON_AddBoolToObject(alarms, "temp_alarm_high", state_copy.a2_parsed.alarms.temp_alarm_high);
        cJSON_AddBoolToObject(alarms, "temp_alarm_low", state_copy.a2_parsed.alarms.temp_alarm_low);
        cJSON_AddBoolToObject(alarms, "rx_power_alarm_low", state_copy.a2_parsed.alarms.rx_power_alarm_low);
        cJSON_AddItemToObject(a2_obj, "alarms", alarms);
        
        cJSON_AddItemToObject(json, "a2", a2_obj);
    } else {
        a2_obj = cJSON_CreateObject();
        cJSON_AddBoolToObject(a2_obj, "valid", false);
        cJSON_AddItemToObject(json, "a2", a2_obj);
    }
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    return json_string;
}

/* ============================================
 * Serializa Apenas Estado
 * ============================================ */
char *daemon_socket_serialize_state(const sfp_daemon_state_data_t *state)
{
    if (!state) {
        return NULL;
    }
    
    cJSON *json = cJSON_CreateObject();
    cJSON *timestamps = cJSON_CreateObject();
    
    /* Obtém cópia thread-safe */
    sfp_daemon_state_data_t state_copy;
    daemon_state_get_copy((sfp_daemon_state_data_t *)state, &state_copy);
    
    cJSON_AddStringToObject(json, "state", daemon_fsm_state_to_string(state_copy.state));
    cJSON_AddNumberToObject(json, "generation_id", (double)state_copy.generation_id);
    
    cJSON_AddNumberToObject(timestamps, "first_detected", (double)state_copy.first_detected);
    cJSON_AddNumberToObject(timestamps, "last_a0_read", (double)state_copy.last_a0_read);
    cJSON_AddNumberToObject(timestamps, "last_a2_read", (double)state_copy.last_a2_read);
    cJSON_AddItemToObject(json, "timestamps", timestamps);
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    return json_string;
}

/* ============================================
 * Serializa PING
 * ============================================ */
char *daemon_socket_serialize_ping(time_t uptime_seconds)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "ok");
    cJSON_AddNumberToObject(json, "uptime", (double)uptime_seconds);
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    return json_string;
}

