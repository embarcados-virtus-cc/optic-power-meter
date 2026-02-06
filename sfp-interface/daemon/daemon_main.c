/**
 * @file daemon_main.c
 * @brief Entry point do daemon SFP
 */

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "daemon_config.h"
#include "daemon_state.h"
#include "daemon_fsm.h"
#include "daemon_i2c.h"
#include "daemon_socket.h"
#include "../sfp_init.h"

/* ============================================
 * Função Auxiliar: Sleep em Milissegundos
 * ============================================ */
static void msleep(uint32_t ms)
{
    /* Usa usleep que é mais comum e funciona bem no Raspberry Pi */
    usleep(ms * 1000);
}

/* ============================================
 * Variáveis Globais
 * ============================================ */
static volatile bool g_running = true;
static sfp_daemon_state_data_t g_state;
static daemon_config_t g_config;
static int g_i2c_fd = -1;
static daemon_socket_server_t g_socket_server;
static time_t g_start_time;

/* ============================================
 * Handler de Sinal
 * ============================================ */
static void signal_handler(int sig)
{
    (void)sig;
    g_running = false;
    syslog(LOG_INFO, "Received signal, shutting down...");
}

/* ============================================
 * Daemonização
 * ============================================ */
static void daemonize(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Failed to fork: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);  /* Parent exits */
    }

    /* Child continues */
    if (setsid() < 0) {
        fprintf(stderr, "Failed to setsid: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Second fork */
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Failed to fork: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* Redireciona I/O */
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
}

/* ============================================
 * Loop Principal
 * ============================================ */
static void main_loop(void)
{
    time_t last_presence_check = 0;
    time_t last_a2_read = 0;

    while (g_running) {
        uint32_t poll_delay_ms = DAEMON_POLL_ABSENT_MS;

        /* Aceita novas conexões socket */
        daemon_socket_accept(&g_socket_server);

        /* Processa comandos socket */
        time_t daemon_uptime = time(NULL) - g_start_time;
        daemon_socket_process_commands(&g_socket_server, &g_state, daemon_uptime);

        /* Obtém estado atual (thread-safe) */
        pthread_mutex_lock(&g_state.mutex);
        sfp_daemon_state_t current_state = g_state.state;
        pthread_mutex_unlock(&g_state.mutex);

        time_t now = time(NULL);

        /* Verificação de presença (polling) */
        bool presence_detected = false;
        if (g_i2c_fd >= 0) {
            presence_detected = daemon_i2c_detect_presence(g_i2c_fd);
        }

        /* Máquina de Estados */
        switch (current_state) {
            case SFP_STATE_INIT:
                daemon_fsm_init_to_absent(&g_state, presence_detected);
                poll_delay_ms = DAEMON_POLL_ABSENT_MS;
                break;

            case SFP_STATE_ABSENT:
                if (presence_detected) {
                    /* Transição ABSENT → PRESENT */
                    if (daemon_fsm_absent_to_present(&g_state)) {
                        /* Lê A0h completo */
                        uint8_t a0_raw[SFP_A0_SIZE];
                        if (daemon_i2c_read_a0h(g_i2c_fd, a0_raw)) {
                            pthread_mutex_lock(&g_state.mutex);

                            memcpy(g_state.a0_raw, a0_raw, SFP_A0_SIZE);
                            uint32_t new_hash = daemon_state_calculate_a0_hash(a0_raw, SFP_A0_SIZE);

                            /* Parse A0h */
                            sfp_parse_a0_base_identifier(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_ext_identifier(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_connector(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_compliance(g_state.a0_raw, &g_state.a0_parsed.cc);
                            sfp_parse_a0_base_encoding(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_nominal_rate(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_rate_identifier(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_smf_km(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_smf_m(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_om2(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_om1(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_om4_or_copper(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_om3_or_cable(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_vendor_name(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_ext_compliance(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_vendor_oui(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_vendor_pn(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_vendor_rev(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_media(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_fc_speed_2(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_parse_a0_base_cc_base(g_state.a0_raw, &g_state.a0_parsed);
                            sfp_a0_decode_compliance(&g_state.a0_parsed.cc, &g_state.a0_parsed.dc);

                            g_state.a0_valid = true;
                            g_state.a0_hash = new_hash;
                            g_state.last_a0_read = now;

                            pthread_mutex_unlock(&g_state.mutex);

                            syslog(LOG_INFO, "A0h read successfully (generation_id: %lu)",
                                   (unsigned long)g_state.generation_id);
                        } else {
                            /* Erro ao ler A0h - volta para ABSENT */
                            daemon_fsm_present_to_absent(&g_state);
                        }
                    }
                }
                poll_delay_ms = g_config.poll_absent_ms;
                break;

            case SFP_STATE_PRESENT:
                /* Verifica presença periodicamente */
                if (now - last_presence_check >= (DAEMON_PRESENCE_CHECK_INTERVAL_MS / 1000)) {
                    if (!presence_detected) {
                        daemon_fsm_present_to_absent(&g_state);
                        poll_delay_ms = g_config.poll_absent_ms;
                        break;
                    }
                    last_presence_check = now;
                }

                /* Lê A2h periodicamente */
                if (now - last_a2_read >= (g_config.poll_present_ms / 1000)) {
                    uint8_t a2_raw[SFP_A2_SIZE];
                    if (daemon_i2c_read_a2h(g_i2c_fd, a2_raw)) {
                        pthread_mutex_lock(&g_state.mutex);

                        memcpy(g_state.a2_raw, a2_raw, SFP_A2_SIZE);
                        float vcc;
                        if (get_sfp_vcc(g_state.a2_raw, &vcc)) {
                            g_state.a2_parsed.vcc_realtime = vcc;
                        }
                        sfp_parse_a2h_data_ready(g_state.a2_raw, &g_state.a2_parsed);

                        g_state.a2_valid = true;
                        g_state.last_a2_read = now;
                        g_state.i2c_error_count = 0;

                        pthread_mutex_unlock(&g_state.mutex);

                        last_a2_read = now;
                    } else {
                        /* Erro ao ler A2h */
                        pthread_mutex_lock(&g_state.mutex);
                        g_state.i2c_error_count++;

                        if (g_state.i2c_error_count >= g_config.max_i2c_errors) {
                            pthread_mutex_unlock(&g_state.mutex);
                            daemon_fsm_present_to_error(&g_state);
                        } else {
                            pthread_mutex_unlock(&g_state.mutex);
                        }
                    }
                }
                poll_delay_ms = g_config.poll_present_ms;
                break;

            case SFP_STATE_ERROR:
                /* Tenta recuperação */
                pthread_mutex_lock(&g_state.mutex);
                g_state.recovery_attempts++;

                if (g_state.recovery_attempts >= g_config.max_recovery_attempts) {
                    /* Verifica presença após muitas tentativas */
                    pthread_mutex_unlock(&g_state.mutex);
                    if (!presence_detected) {
                        daemon_fsm_error_to_absent(&g_state, false);
                    }
                } else {
                    pthread_mutex_unlock(&g_state.mutex);

                    /* Tenta ler A2h novamente */
                    uint8_t a2_raw[SFP_A2_SIZE];
                    if (daemon_i2c_read_a2h(g_i2c_fd, a2_raw)) {
                        pthread_mutex_lock(&g_state.mutex);

                        memcpy(g_state.a2_raw, a2_raw, SFP_A2_SIZE);
                        float vcc;
                        if (get_sfp_vcc(g_state.a2_raw, &vcc)) {
                            g_state.a2_parsed.vcc_realtime = vcc;
                        }
                        sfp_parse_a2h_data_ready(g_state.a2_raw, &g_state.a2_parsed);

                        g_state.a2_valid = true;
                        g_state.last_a2_read = now;
                        g_state.i2c_error_count = 0;

                        pthread_mutex_unlock(&g_state.mutex);

                        daemon_fsm_error_to_present(&g_state);
                        last_a2_read = now;
                    }
                }
                poll_delay_ms = g_config.poll_error_ms;
                break;
        }

        /* Sleep */
        msleep(poll_delay_ms);
    }
}

/* ============================================
 * Main
 * ============================================ */
int main(int argc, char *argv[])
{
    /* Parse argumentos */
    bool foreground = false;
    const char *config_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--foreground") == 0) {
            foreground = true;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                config_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [-f|--foreground] [-c|--config FILE]\n", argv[0]);
            return EXIT_SUCCESS;
        }
    }

    /* Abre syslog */
    openlog(DAEMON_LOG_IDENT, LOG_PID | LOG_CONS, DAEMON_LOG_FACILITY);
    syslog(LOG_INFO, "Starting SFP daemon...");

    /* Carrega configuração */
    if (!daemon_config_load(&g_config, config_file)) {
        syslog(LOG_ERR, "Failed to load configuration");
        closelog();
        return EXIT_FAILURE;
    }

    /* Daemonização */
    if (g_config.daemonize && !foreground) {
        daemonize();
    }

    g_start_time = time(NULL);

    /* Configura handlers de sinal */
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, SIG_IGN);

    /* Inicializa estado */
    if (!daemon_state_init(&g_state)) {
        syslog(LOG_ERR, "Failed to initialize state");
        closelog();
        return EXIT_FAILURE;
    }

    /* Abre dispositivo I²C */
    g_i2c_fd = sfp_i2c_init(g_config.i2c_device);
    if (g_i2c_fd < 0) {
        syslog(LOG_ERR, "Failed to open I²C device: %s", g_config.i2c_device);
        daemon_state_cleanup(&g_state);
        closelog();
        return EXIT_FAILURE;
    }

    syslog(LOG_INFO, "I²C device opened: %s", g_config.i2c_device);

    /* Inicializa servidor socket */
    if (!daemon_socket_init(&g_socket_server, &g_config)) {
        syslog(LOG_ERR, "Failed to initialize socket server");
        sfp_i2c_close(g_i2c_fd);
        daemon_state_cleanup(&g_state);
        closelog();
        return EXIT_FAILURE;
    }

    syslog(LOG_INFO, "Daemon started successfully");

    /* Loop principal */
    main_loop();

    /* Cleanup */
    syslog(LOG_INFO, "Shutting down daemon...");
    daemon_socket_cleanup(&g_socket_server);
    sfp_i2c_close(g_i2c_fd);
    daemon_state_cleanup(&g_state);
    closelog();

    return EXIT_SUCCESS;
}
