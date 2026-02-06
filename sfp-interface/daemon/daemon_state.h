/**
 * @file daemon_state.h
 * @brief Estrutura de estado global do daemon SFP (thread-safe)
 */

#ifndef DAEMON_STATE_H
#define DAEMON_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "../defs.h"
#include "../a0h.h"
#include "../a2h.h"

/* ============================================
 * Estados da Máquina de Estados
 * ============================================ */
typedef enum {
    SFP_STATE_INIT,      /* Estado inicial (ainda não verificou) */
    SFP_STATE_ABSENT,    /* SFP não detectado no barramento */
    SFP_STATE_PRESENT,   /* SFP detectado e dados válidos */
    SFP_STATE_ERROR      /* Erro temporário (tentando recuperar) */
} sfp_daemon_state_t;

/* ============================================
 * Estrutura de Estado Global
 * ============================================ */
typedef struct {
    /* Estado da máquina de estados */
    sfp_daemon_state_t state;

    /* Generation ID: incrementado sempre que um novo SFP é detectado */
    uint64_t generation_id;

    /* Hash do A0h para detecção de mudança de SFP */
    uint32_t a0_hash;

    /* Timestamps */
    time_t last_a0_read;      /* Última leitura bem-sucedida de A0h */
    time_t last_a2_read;      /* Última leitura bem-sucedida de A2h */
    time_t first_detected;    /* Quando o SFP atual foi detectado pela primeira vez */

    /* Dados A0h (estáticos - só mudam quando novo SFP é inserido) */
    bool a0_valid;
    uint8_t a0_raw[SFP_A0_SIZE];
    sfp_a0h_base_t a0_parsed;

    /* Dados A2h (dinâmicos - atualizados periodicamente) */
    bool a2_valid;
    uint8_t a2_raw[SFP_A2_SIZE];
    sfp_a2h_t a2_parsed;

    /* Contadores de erro */
    uint32_t i2c_error_count;      /* Contador de erros I²C consecutivos */
    uint32_t recovery_attempts;     /* Tentativas de recuperação */

    /* Mutex para thread-safety */
    pthread_mutex_t mutex;

} sfp_daemon_state_data_t;

/* ============================================
 * Funções de Gerenciamento de Estado
 * ============================================ */

/**
 * @brief Inicializa a estrutura de estado global
 * @param state Ponteiro para estrutura de estado
 * @return true se inicializado com sucesso, false caso contrário
 */
bool daemon_state_init(sfp_daemon_state_data_t *state);

/**
 * @brief Libera recursos da estrutura de estado
 * @param state Ponteiro para estrutura de estado
 */
void daemon_state_cleanup(sfp_daemon_state_data_t *state);

/**
 * @brief Obtém uma cópia thread-safe do estado atual
 * @param state Ponteiro para estrutura de estado
 * @param out Ponteiro para estrutura de saída (recebe cópia)
 */
void daemon_state_get_copy(sfp_daemon_state_data_t *state, sfp_daemon_state_data_t *out);

/**
 * @brief Calcula hash simples do A0h para detecção de mudança
 * @param a0_raw Dados brutos do A0h
 * @param size Tamanho dos dados
 * @return Hash calculado
 */
uint32_t daemon_state_calculate_a0_hash(const uint8_t *a0_raw, size_t size);

/**
 * @brief Verifica se o SFP mudou comparando hash
 * @param state Ponteiro para estrutura de estado
 * @param new_hash Novo hash calculado
 * @return true se SFP mudou, false caso contrário
 */
bool daemon_state_sfp_changed(sfp_daemon_state_data_t *state, uint32_t new_hash);

#endif /* DAEMON_STATE_H */
