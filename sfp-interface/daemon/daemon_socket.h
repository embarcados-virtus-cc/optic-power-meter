/**
 * @file daemon_socket.h
 * @brief Servidor UNIX socket e protocolo de comunicação
 */

#ifndef DAEMON_SOCKET_H
#define DAEMON_SOCKET_H

#include <stdbool.h>
#include "daemon_state.h"
#include "daemon_config.h"

/* ============================================
 * Estrutura do Servidor Socket
 * ============================================ */
typedef struct {
    int server_fd;
    int client_fds[DAEMON_MAX_CONNECTIONS];
    int num_clients;
    char socket_path[256];
} daemon_socket_server_t;

/* ============================================
 * Funções do Servidor Socket
 * ============================================ */

/**
 * @brief Inicializa servidor socket UNIX
 * @param server Ponteiro para estrutura do servidor
 * @param config Ponteiro para configuração
 * @return true se inicializado com sucesso, false caso contrário
 */
bool daemon_socket_init(daemon_socket_server_t *server, const daemon_config_t *config);

/**
 * @brief Libera recursos do servidor socket
 * @param server Ponteiro para estrutura do servidor
 */
void daemon_socket_cleanup(daemon_socket_server_t *server);

/**
 * @brief Aceita novas conexões (non-blocking)
 * @param server Ponteiro para estrutura do servidor
 * @return true se nova conexão aceita, false caso contrário
 */
bool daemon_socket_accept(daemon_socket_server_t *server);

/**
 * @brief Processa comandos pendentes de clientes conectados
 * @param server Ponteiro para estrutura do servidor
 * @param state Ponteiro para estado global
 * @param daemon_uptime Uptime do daemon em segundos
 * @return Número de comandos processados
 */
int daemon_socket_process_commands(daemon_socket_server_t *server, sfp_daemon_state_data_t *state, time_t daemon_uptime);

/**
 * @brief Fecha conexões inativas
 * @param server Ponteiro para estrutura do servidor
 */
void daemon_socket_close_inactive(daemon_socket_server_t *server);

/* ============================================
 * Funções de Serialização JSON
 * ============================================ */

/**
 * @brief Serializa estado completo para JSON
 * @param state Ponteiro para estado
 * @return String JSON (deve ser liberada pelo caller usando free())
 */
char *daemon_socket_serialize_current(const sfp_daemon_state_data_t *state);

/**
 * @brief Serializa apenas dados A0h para JSON
 * @param state Ponteiro para estado
 * @return String JSON (deve ser liberada pelo caller usando free())
 */
char *daemon_socket_serialize_static(const sfp_daemon_state_data_t *state);

/**
 * @brief Serializa apenas dados A2h para JSON
 * @param state Ponteiro para estado
 * @return String JSON (deve ser liberada pelo caller usando free())
 */
char *daemon_socket_serialize_dynamic(const sfp_daemon_state_data_t *state);

/**
 * @brief Serializa apenas estado da FSM para JSON
 * @param state Ponteiro para estado
 * @return String JSON (deve ser liberada pelo caller usando free())
 */
char *daemon_socket_serialize_state(const sfp_daemon_state_data_t *state);

/**
 * @brief Serializa resposta PING para JSON
 * @param uptime_seconds Uptime do daemon em segundos
 * @return String JSON (deve ser liberada pelo caller usando free())
 */
char *daemon_socket_serialize_ping(time_t uptime_seconds);

#endif /* DAEMON_SOCKET_H */

