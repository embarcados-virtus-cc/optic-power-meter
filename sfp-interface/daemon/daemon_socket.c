/**
 * @file daemon_socket.c
 * @brief Implementação do servidor socket e serialização JSON
 */

#include "daemon_socket.h"
#include "daemon_fsm.h"
#include "daemon_state.h"
#include "../a0h.h"
#include "../a2h.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
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
    size_t path_len = strlen(config->socket_path);
    size_t copy_len = (path_len < sizeof(server->socket_path) - 1) ? path_len : sizeof(server->socket_path) - 1;
    memcpy(server->socket_path, config->socket_path, copy_len);
    server->socket_path[copy_len] = '\0';
    server->server_fd = -1;
    server->num_clients = 0;

    for (int i = 0; i < DAEMON_MAX_CONNECTIONS; i++) {
        server->client_fds[i] = -1;
    }

    /* Cria diretório do socket se não existir */
    char dir_path[256];
    size_t dir_path_len = strlen(server->socket_path);
    size_t dir_path_copy_len = (dir_path_len < sizeof(dir_path) - 1) ? dir_path_len : sizeof(dir_path) - 1;
    memcpy(dir_path, server->socket_path, dir_path_copy_len);
    dir_path[dir_path_copy_len] = '\0';

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
    size_t sun_path_len = strlen(server->socket_path);
    size_t sun_path_copy_len = (sun_path_len < sizeof(addr.sun_path) - 1) ? sun_path_len : sizeof(addr.sun_path) - 1;
    memcpy(addr.sun_path, server->socket_path, sun_path_copy_len);
    addr.sun_path[sun_path_copy_len] = '\0';

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
    size_t cmd_len = strlen(command);
    size_t cmd_copy_len = (cmd_len < sizeof(cmd) - 1) ? cmd_len : sizeof(cmd) - 1;
    memcpy(cmd, command, cmd_copy_len);
    cmd[cmd_copy_len] = '\0';
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
 * Funções Auxiliares de Serialização
 * ============================================ */

/* Serializa compliance codes decodificados */
static void serialize_compliance_codes(cJSON *obj, const sfp_compliance_decoded_t *dc)
{
    if (!obj || !dc) return;

    cJSON *byte3 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte3, "eth_10g_base_sr", dc->eth_10g_base_sr);
    cJSON_AddBoolToObject(byte3, "eth_10g_base_lr", dc->eth_10g_base_lr);
    cJSON_AddBoolToObject(byte3, "eth_10g_base_lrm", dc->eth_10g_base_lrm);
    cJSON_AddBoolToObject(byte3, "eth_10g_base_er", dc->eth_10g_base_er);
    cJSON_AddBoolToObject(byte3, "infiniband_1x_sx", dc->infiniband_1x_sx);
    cJSON_AddBoolToObject(byte3, "infiniband_1x_lx", dc->infiniband_1x_lx);
    cJSON_AddBoolToObject(byte3, "infiniband_1x_copper_active", dc->infiniband_1x_copper_active);
    cJSON_AddBoolToObject(byte3, "infiniband_1x_copper_passive", dc->infiniband_1x_copper_passive);
    cJSON_AddItemToObject(obj, "byte3_ethernet_infiniband", byte3);

    cJSON *byte4 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte4, "escon_mmf", dc->escon_mmf);
    cJSON_AddBoolToObject(byte4, "escon_smf", dc->escon_smf);
    cJSON_AddBoolToObject(byte4, "oc_192_sr", dc->oc_192_sr);
    cJSON_AddBoolToObject(byte4, "sonet_rs_1", dc->sonet_rs_1);
    cJSON_AddBoolToObject(byte4, "sonet_rs_2", dc->sonet_rs_2);
    cJSON_AddBoolToObject(byte4, "oc_48_lr", dc->oc_48_lr);
    cJSON_AddBoolToObject(byte4, "oc_48_ir", dc->oc_48_ir);
    cJSON_AddBoolToObject(byte4, "oc_48_sr", dc->oc_48_sr);
    cJSON_AddItemToObject(obj, "byte4_escon_sonet", byte4);

    cJSON *byte5 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte5, "oc_12_sm_lr", dc->oc_12_sm_lr);
    cJSON_AddBoolToObject(byte5, "oc_12_sm_ir", dc->oc_12_sm_ir);
    cJSON_AddBoolToObject(byte5, "oc_12_sr", dc->oc_12_sr);
    cJSON_AddBoolToObject(byte5, "oc_3_sm_lr", dc->oc_3_sm_lr);
    cJSON_AddBoolToObject(byte5, "oc_3_sm_ir", dc->oc_3_sm_ir);
    cJSON_AddBoolToObject(byte5, "oc_3_sr", dc->oc_3_sr);
    cJSON_AddItemToObject(obj, "byte5_sonet", byte5);

    cJSON *byte6 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte6, "eth_base_px", dc->eth_base_px);
    cJSON_AddBoolToObject(byte6, "eth_base_bx_10", dc->eth_base_bx_10);
    cJSON_AddBoolToObject(byte6, "eth_100_base_fx", dc->eth_100_base_fx);
    cJSON_AddBoolToObject(byte6, "eth_100_base_lx", dc->eth_100_base_lx);
    cJSON_AddBoolToObject(byte6, "eth_1000_base_t", dc->eth_1000_base_t);
    cJSON_AddBoolToObject(byte6, "eth_1000_base_cx", dc->eth_1000_base_cx);
    cJSON_AddBoolToObject(byte6, "eth_1000_base_lx", dc->eth_1000_base_lx);
    cJSON_AddBoolToObject(byte6, "eth_1000_base_sx", dc->eth_1000_base_sx);
    cJSON_AddItemToObject(obj, "byte6_ethernet_1g", byte6);

    cJSON *byte7 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte7, "fc_very_long_distance", dc->fc_very_long_distance);
    cJSON_AddBoolToObject(byte7, "fc_short_distance", dc->fc_short_distance);
    cJSON_AddBoolToObject(byte7, "fc_intermediate_distance", dc->fc_intermediate_distance);
    cJSON_AddBoolToObject(byte7, "fc_long_distance", dc->fc_long_distance);
    cJSON_AddBoolToObject(byte7, "fc_medium_distance", dc->fc_medium_distance);
    cJSON_AddBoolToObject(byte7, "shortwave_laser_sa", dc->shortwave_laser_sa);
    cJSON_AddBoolToObject(byte7, "longwave_laser_lc", dc->longwave_laser_lc);
    cJSON_AddBoolToObject(byte7, "electrical_inter_enclosure", dc->electrical_inter_enclosure);
    cJSON_AddItemToObject(obj, "byte7_fc_link_length", byte7);

    cJSON *byte8 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte8, "electrical_intra_enclosure", dc->electrical_intra_enclosure);
    cJSON_AddBoolToObject(byte8, "shortwave_laser_sn", dc->shortwave_laser_sn);
    cJSON_AddBoolToObject(byte8, "shortwave_laser_sl", dc->shortwave_laser_sl);
    cJSON_AddBoolToObject(byte8, "longwave_laser_ll", dc->longwave_laser_ll);
    cJSON_AddBoolToObject(byte8, "active_cable", dc->active_cable);
    cJSON_AddBoolToObject(byte8, "passive_cable", dc->passive_cable);
    cJSON_AddItemToObject(obj, "byte8_fc_technology", byte8);

    cJSON *byte9 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte9, "twin_axial_pair", dc->twin_axial_pair);
    cJSON_AddBoolToObject(byte9, "twisted_pair", dc->twisted_pair);
    cJSON_AddBoolToObject(byte9, "miniature_coax", dc->miniature_coax);
    cJSON_AddBoolToObject(byte9, "video_coax", dc->video_coax);
    cJSON_AddBoolToObject(byte9, "multimode_m6", dc->multimode_m6);
    cJSON_AddBoolToObject(byte9, "multimode_m5", dc->multimode_m5);
    cJSON_AddBoolToObject(byte9, "single_mode", dc->single_mode);
    cJSON_AddItemToObject(obj, "byte9_fc_transmission_media", byte9);

    cJSON *byte10 = cJSON_CreateObject();
    cJSON_AddBoolToObject(byte10, "cs_1200_mbps", dc->cs_1200_mbps);
    cJSON_AddBoolToObject(byte10, "cs_800_mbps", dc->cs_800_mbps);
    cJSON_AddBoolToObject(byte10, "cs_1600_mbps", dc->cs_1600_mbps);
    cJSON_AddBoolToObject(byte10, "cs_400_mbps", dc->cs_400_mbps);
    cJSON_AddBoolToObject(byte10, "cs_3200_mbps", dc->cs_3200_mbps);
    cJSON_AddBoolToObject(byte10, "cs_200_mbps", dc->cs_200_mbps);
    cJSON_AddBoolToObject(byte10, "see_byte_62", dc->see_byte_62);
    cJSON_AddBoolToObject(byte10, "cs_100_mbps", dc->cs_100_mbps);
    cJSON_AddItemToObject(obj, "byte10_fc_channel_speed", byte10);
}

/* Serializa identifier type como string */
static const char* identifier_type_to_string(sfp_identifier_t id)
{
    switch (id) {
        case SFP_ID_GBIC: return "GBIC";
        case SFP_ID_SFP: return "SFP/SFP+";
        case SFP_ID_QSFP: return "QSFP";
        case SFP_ID_QSFP_PLUS: return "QSFP+";
        case SFP_ID_QSFP28: return "QSFP28";
        default: return "Unknown";
    }
}

/* Serializa extended compliance code como string */
static const char* ext_compliance_to_string(sfp_extended_spec_compliance_code_t code)
{
    switch (code) {
        case EXT_SPEC_COMPLIANCE_UNSPECIFIED:
            return "Não especificado";
        case EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_5E_5:
            return "100G AOC ou 25GAUI C2M AOC (BER 5e-5)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR4_OR_25GBASE_SR:
            return "100GBASE-SR4 ou 25GBASE-SR";
        case EXT_SPEC_COMPLIANCE_100GBASE_LR4_OR_25GBASE_LR:
            return "100GBASE-LR4 ou 25GBASE-LR";
        case EXT_SPEC_COMPLIANCE_100GBASE_ER4_OR_25GBASE_ER:
            return "100GBASE-ER4 ou 25GBASE-ER";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR10:
            return "100GBASE-SR10";
        case EXT_SPEC_COMPLIANCE_100G_CWDM4:
            return "100G CWDM4";
        case EXT_SPEC_COMPLIANCE_100G_PSM4:
            return "100G PSM4";
        case EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_5E_5:
            return "100G ACC ou 25GAUI C2M ACC (BER 5e-5)";
        case EXT_SPEC_COMPLIANCE_100GBASE_CR4_OR_25GBASE_CR_CA_25G_L_OR_50GBASE_CR2_RS_FEC:
            return "100GBASE-CR4, 25GBASE-CR CA-25G-L ou 50GBASE-CR2 RS-FEC";
        case EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_S_OR_50GBASE_CR2_BASE_R_FEC:
            return "25GBASE-CR CA-25G-S ou 50GBASE-CR2 BASE-R FEC";
        case EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_N_OR_50GBASE_CR2_NO_FEC:
            return "25GBASE-CR CA-25G-N ou 50GBASE-CR2 NO-FEC";
        case EXT_SPEC_COMPLIANCE_10MB_SINGLE_PAIR_ETHERNET:
            return "10Mb Single Pair Ethernet";
        case EXT_SPEC_COMPLIANCE_40GBASE_ER4:
            return "40GBASE-ER4";
        case EXT_SPEC_COMPLIANCE_4X_10GBASE_SR:
            return "4x10GBASE-SR";
        case EXT_SPEC_COMPLIANCE_40G_PSM4:
            return "40G PSM4";
        case EXT_SPEC_COMPLIANCE_10GBASE_T_SFI:
            return "10GBASE-T SFI";
        case EXT_SPEC_COMPLIANCE_100G_CLR4:
            return "100G CLR4";
        case EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_1E_12:
            return "100G AOC ou 25GAUI C2M AOC (BER 1e-12)";
        case EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_1E_12:
            return "100G ACC ou 25GAUI C2M ACC (BER 1e-12)";
        case EXT_SPEC_COMPLIANCE_10GBASE_T_SHORT_REACH:
            return "10GBASE-T Short Reach";
        case EXT_SPEC_COMPLIANCE_5GBASE_T:
            return "5GBASE-T";
        case EXT_SPEC_COMPLIANCE_2_5GBASE_T:
            return "2.5GBASE-T";
        case EXT_SPEC_COMPLIANCE_40G_SWDM4:
            return "40G SWDM4";
        case EXT_SPEC_COMPLIANCE_100G_SWDM4:
            return "100G SWDM4";
        case EXT_SPEC_COMPLIANCE_100G_PAM4_BIDI:
            return "100G PAM4 BiDi";
        case EXT_SPEC_COMPLIANCE_100GBASE_DR_CAUI4_NO_FEC:
            return "100GBASE-DR (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100G_FR_OR_100GBASE_FR1_CAUI4_NO_FEC:
            return "100G-FR/100GBASE-FR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100G_LR_OR_100GBASE_LR1_CAUI4_NO_FEC:
            return "100G-LR/100GBASE-LR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR1_CAUI4_NO_FEC:
            return "100GBASE-SR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR1_OR_200GBASE_SR2_OR_400GBASE_SR4:
            return "100GBASE-SR1, 200GBASE-SR2 ou 400GBASE-SR4";
        case EXT_SPEC_COMPLIANCE_100GBASE_FR1_OR_400GBASE_DR4_2:
            return "100GBASE-FR1 ou 400GBASE-DR4";
        case EXT_SPEC_COMPLIANCE_100GBASE_LR1:
            return "100GBASE-LR1";
        case EXT_SPEC_COMPLIANCE_ACTIVE_CU_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6:
            return "Active Copper Cable (50GAUI/100GAUI-2/200GAUI-4 C2M BER 1e-6)";
        case EXT_SPEC_COMPLIANCE_ACTIVE_OPTICAL_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6:
            return "Active Optical Cable (50GAUI/100GAUI-2/200GAUI-4 C2M BER 1e-6)";
        case EXT_SPEC_COMPLIANCE_100GBASE_CR1_OR_200GBASE_CR2_OR_400GBASE_CR4:
            return "100GBASE-CR1, 200GBASE-CR2 ou 400GBASE-CR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_CR_OR_100GBASE_CR2_OR_200GBASE_CR4:
            return "50GBASE-CR, 100GBASE-CR2 ou 200GBASE-CR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_R_OR_100GBASE_SR2_OR_200GBASE_SR4:
            return "50GBASE-R, 100GBASE-SR2 ou 200GBASE-SR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_FR_OR_200GBASE_DR4:
            return "50GBASE-FR ou 200GBASE-DR4";
        case EXT_SPEC_COMPLIANCE_200GBASE_FR4:
            return "200GBASE-FR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_LR:
            return "50GBASE-LR";
        case EXT_SPEC_COMPLIANCE_200GBASE_LR4:
            return "200GBASE-LR4";
        case EXT_SPEC_COMPLIANCE_400GBASE_DR4_400GAUI4_C2M:
            return "400GBASE-DR4 (400GAUI-4 C2M)";
        case EXT_SPEC_COMPLIANCE_400GBASE_FR4:
            return "400GBASE-FR4";
        case EXT_SPEC_COMPLIANCE_400GBASE_LR4_6:
            return "400GBASE-LR4-6";
        case EXT_SPEC_COMPLIANCE_400G_LR4_10:
            return "400G LR4-10";
        case EXT_SPEC_COMPLIANCE_256GFC_SW4:
            return "256GFC SW4";
        case EXT_SPEC_COMPLIANCE_64GFC:
            return "64GFC";
        case EXT_SPEC_COMPLIANCE_128GFC:
            return "128GFC";
        case EXT_SPEC_COMPLIANCE_VENDOR_SPECIFIC:
            return "Específico do fabricante";
        default:
            if (code >= 0x4D && code <= 0x7E) {
                return "Reservado (SFF-8024)";
            } else if (code >= 0x82 && code <= 0xFE) {
                return "Reservado (SFF-8024)";
            }
            return "Código desconhecido";
    }
}

/* Serializa A0h completo */
static void serialize_a0h_complete(cJSON *a0_obj, const sfp_a0h_base_t *a0)
{
    if (!a0_obj || !a0) return;

    /* Byte 0 - Identifier */
    uint8_t identifier = sfp_a0_get_identifier(a0);
    cJSON_AddNumberToObject(a0_obj, "identifier", identifier);
    cJSON_AddStringToObject(a0_obj, "identifier_type", identifier_type_to_string((sfp_identifier_t)identifier));

    /* Byte 1 - Extended Identifier */
    uint8_t ext_identifier = sfp_a0_get_ext_identifier(a0);
    bool ext_id_valid = sfp_validate_ext_identifier(a0);
    cJSON_AddNumberToObject(a0_obj, "ext_identifier", ext_identifier);
    cJSON_AddBoolToObject(a0_obj, "ext_identifier_valid", ext_id_valid);

    /* Byte 2 - Connector */
    sfp_connector_type_t connector = sfp_a0_get_connector(a0);
    const char *connector_str = sfp_connector_to_string(connector);
    cJSON_AddNumberToObject(a0_obj, "connector", connector);
    cJSON_AddStringToObject(a0_obj, "connector_type", connector_str);

    /* Bytes 3-10 - Compliance Codes */
    serialize_compliance_codes(a0_obj, &a0->dc);

    /* Byte 11 - Encoding */
    sfp_encoding_codes_t encoding = sfp_a0_get_encoding(a0);
    cJSON_AddNumberToObject(a0_obj, "encoding", encoding);

    /* Byte 12 - Nominal Rate */
    sfp_nominal_rate_status_t nominal_rate_status;
    uint8_t nominal_rate = sfp_a0_get_nominal_rate_mbd(a0, &nominal_rate_status);
    cJSON_AddNumberToObject(a0_obj, "nominal_rate_mbd", nominal_rate);
    cJSON_AddNumberToObject(a0_obj, "nominal_rate_status", nominal_rate_status);

    /* Byte 13 - Rate Identifier */
    sfp_rate_select rate_id = sfp_a0_get_rate_identifier(a0);
    cJSON_AddNumberToObject(a0_obj, "rate_identifier", rate_id);

    /* Byte 14 - SMF Length or Copper Attenuation */
    sfp_smf_length_status_t smf_status_km;
    uint16_t smf_len_km = sfp_a0_get_smf_length_km(a0, &smf_status_km);
    cJSON_AddNumberToObject(a0_obj, "smf_length_km", smf_len_km);
    cJSON_AddNumberToObject(a0_obj, "smf_length_status_km", smf_status_km);

    /* Byte 15 SMF Length or Copper Attenuation (units 100m) */
    sfp_smf_length_status_t smf_status_m;
    uint16_t smf_len_m = sfp_a0_get_smf_length_m(a0, &smf_status_m);
    cJSON_AddNumberToObject(a0_obj, "smf_length_m", smf_len_m);
    cJSON_AddNumberToObject(a0_obj, "smf_length_status_m", smf_status_m);

    /* Byte 16 - OM2 Length */
    sfp_om2_length_status_t om2_status;
    uint16_t om2_len = sfp_a0_get_om2_length_m(a0, &om2_status);
    cJSON_AddNumberToObject(a0_obj, "om2_length_m", om2_len);
    cJSON_AddNumberToObject(a0_obj, "om2_length_status", om2_status);

    /* Byte 17 - OM1 Length */
    sfp_om1_length_status_t om1_status;
    uint16_t om1_len = sfp_a0_get_om1_length_m(a0, &om1_status);
    cJSON_AddNumberToObject(a0_obj, "om1_length_m", om1_len);
    cJSON_AddNumberToObject(a0_obj, "om1_length_status", om1_status);

    /* Byte 18 - OM4 or Copper Length */
    sfp_om4_length_status_t om4_status;
    uint16_t om4_copper_len = sfp_a0_get_om4_copper_or_length_m(a0, &om4_status);
    cJSON_AddNumberToObject(a0_obj, "om4_or_copper_length_m", om4_copper_len);
    cJSON_AddNumberToObject(a0_obj, "om4_or_copper_length_status", om4_status);

    /* Byte 19 - OM3 or OM3 or Optical/Cable Physical Interconnect Length */
    sfp_om3_length_status_t om3_status;
    uint32_t om3_len = sfp_a0_get_om3_cable_length_m(a0, &om3_status);
    cJSON_AddNumberToObject(a0_obj, "om3_length_m", om3_len);
    cJSON_AddNumberToObject(a0_obj, "om3_length_status", om3_status);

    /* Bytes 20-35 - Vendor Name */
    char vendor_name[SFP_A0_LEN_VENDOR_NAME + 1] = {0};
    bool vendor_name_valid = sfp_a0_get_vendor_name(a0, vendor_name);
    cJSON_AddStringToObject(a0_obj, "vendor_name", vendor_name_valid && vendor_name[0] ? vendor_name : "");
    cJSON_AddBoolToObject(a0_obj, "vendor_name_valid", vendor_name_valid);

    /* Byte 36 - Extended Compliance */
    sfp_extended_spec_compliance_code_t ext_compliance = sfp_a0_get_ext_compliance(a0);
    cJSON_AddNumberToObject(a0_obj, "ext_compliance_code", ext_compliance);
    cJSON_AddStringToObject(a0_obj, "ext_compliance_desc", ext_compliance_to_string(ext_compliance));

    /* Bytes 37-39 - Vendor OUI */
    uint8_t vendor_oui_raw[3] = {0};
    bool vendor_oui_valid = sfp_a0_get_vendor_oui(a0, vendor_oui_raw);
    uint32_t vendor_oui_u32 = sfp_vendor_oui_to_u32(a0);
    cJSON_AddBoolToObject(a0_obj, "vendor_oui_valid", vendor_oui_valid);
    if (vendor_oui_valid) {
        cJSON *oui_array = cJSON_CreateArray();
        cJSON_AddItemToArray(oui_array, cJSON_CreateNumber(vendor_oui_raw[0]));
        cJSON_AddItemToArray(oui_array, cJSON_CreateNumber(vendor_oui_raw[1]));
        cJSON_AddItemToArray(oui_array, cJSON_CreateNumber(vendor_oui_raw[2]));
        cJSON_AddItemToObject(a0_obj, "vendor_oui", oui_array);
        cJSON_AddNumberToObject(a0_obj, "vendor_oui_u32", vendor_oui_u32);
    }

    /* Bytes 40-55 - Vendor Part Number */
    const char *vendor_pn = NULL;
    bool vendor_pn_valid = sfp_a0_get_vendor_pn(a0, &vendor_pn);
    cJSON_AddStringToObject(a0_obj, "vendor_pn", vendor_pn_valid && vendor_pn ? vendor_pn : "");
    cJSON_AddBoolToObject(a0_obj, "vendor_pn_valid", vendor_pn_valid);

    /* Bytes 56-59 - Vendor Revision */
    char vendor_rev[5] = {0};
    bool vendor_rev_valid = sfp_a0_get_vendor_rev(a0, vendor_rev);
    cJSON_AddStringToObject(a0_obj, "vendor_rev", vendor_rev_valid ? vendor_rev : "");
    cJSON_AddStringToObject(a0_obj, "vendor_rev_valid", vendor_rev_valid ? "valido": "invalido");

    /* Bytes 60-61 - Wavelength or Cable Compliance */
    sfp_variant_t variant = sfp_a0_get_variant(a0);
    cJSON_AddNumberToObject(a0_obj, "variant", variant);
    if (variant == SFP_VARIANT_OPTICAL) {
        uint16_t wavelength = 0;
        if (sfp_a0_get_wavelength_nm(a0, &wavelength)) {
            cJSON_AddNumberToObject(a0_obj, "wavelength_nm", wavelength);
        }
    } else if (variant == SFP_VARIANT_PASSIVE_CABLE || variant == SFP_VARIANT_ACTIVE_CABLE) {
        uint8_t cable_compliance = 0;
        if (sfp_a0_get_cable_compliance(a0, &cable_compliance)) {
            cJSON_AddNumberToObject(a0_obj, "cable_compliance", cable_compliance);
        }
    }

    /* Byte 62 - Fibre Channel Speed 2 */
    bool fc_speed_2_valid = sfp_get_a0_fc_speed_2(a0, &a0->dc);
    cJSON_AddBoolToObject(a0_obj, "fc_speed_2_valid", fc_speed_2_valid);
    if (fc_speed_2_valid) {
        cJSON_AddNumberToObject(a0_obj, "fc_speed_2", a0->fc_speed2);
    }

    /* Byte 63 - CC_BASE (Checksum) */
    bool cc_base_valid = sfp_a0_get_cc_base_is_valid(a0);
    cJSON_AddBoolToObject(a0_obj, "cc_base_valid", cc_base_valid);
    cJSON_AddNumberToObject(a0_obj, "cc_base", a0->cc_base);
}

/* Serializa A2h completo */
static void serialize_a2h_complete(cJSON *a2_obj, const sfp_a2h_t *a2)
{
    if (!a2_obj || !a2) return;

    /* Voltage */
    cJSON_AddBoolToObject(a2_obj, "voltage_valid", true);
    cJSON_AddNumberToObject(a2_obj, "voltage_v", a2->vcc_realtime);

    /* Data Ready */
    cJSON_AddBoolToObject(a2_obj, "data_ready", a2->data_ready);

    /* Omit unrequested fields for now as per user request */
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
    } else if (state_copy.state == SFP_STATE_ERROR) {
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
        serialize_a0h_complete(a0_obj, &state_copy.a0_parsed);
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
        serialize_a2h_complete(a2_obj, &state_copy.a2_parsed);
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
        serialize_a0h_complete(a0_obj, &state_copy.a0_parsed);
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
        serialize_a2h_complete(a2_obj, &state_copy.a2_parsed);
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
