/**
 * @file daemon_state.c
 * @brief Implementação das funções de gerenciamento de estado
 */

#include "daemon_state.h"
#include <string.h>
#include <syslog.h>

/* ============================================
 * Inicializa Estado
 * ============================================ */
bool daemon_state_init(sfp_daemon_state_data_t *state)
{
    if (!state) {
        return false;
    }

    memset(state, 0, sizeof(sfp_daemon_state_data_t));
    
    state->state = SFP_STATE_INIT;
    state->generation_id = 0;
    state->a0_hash = 0;
    state->a0_valid = false;
    state->a2_valid = false;
    state->i2c_error_count = 0;
    state->recovery_attempts = 0;
    
    /* Inicializa mutex */
    if (pthread_mutex_init(&state->mutex, NULL) != 0) {
        syslog(LOG_ERR, "Failed to initialize mutex");
        return false;
    }
    
    return true;
}

/* ============================================
 * Libera Recursos
 * ============================================ */
void daemon_state_cleanup(sfp_daemon_state_data_t *state)
{
    if (!state) {
        return;
    }
    
    pthread_mutex_destroy(&state->mutex);
    memset(state, 0, sizeof(sfp_daemon_state_data_t));
}

/* ============================================
 * Obtém Cópia Thread-Safe
 * ============================================ */
void daemon_state_get_copy(sfp_daemon_state_data_t *state, sfp_daemon_state_data_t *out)
{
    if (!state || !out) {
        return;
    }
    
    pthread_mutex_lock(&state->mutex);
    memcpy(out, state, sizeof(sfp_daemon_state_data_t));
    pthread_mutex_unlock(&state->mutex);
}

/* ============================================
 * Calcula Hash do A0h
 * ============================================ */
uint32_t daemon_state_calculate_a0_hash(const uint8_t *a0_raw, size_t size)
{
    if (!a0_raw || size == 0) {
        return 0;
    }
    
    /* Hash simples: djb2 algorithm */
    uint32_t hash = 5381;
    for (size_t i = 0; i < size && i < 32; i++) { /* Usa primeiros 32 bytes para hash */
        hash = ((hash << 5) + hash) + a0_raw[i];
    }
    
    return hash;
}

/* ============================================
 * Verifica se SFP Mudou
 * ============================================ */
bool daemon_state_sfp_changed(sfp_daemon_state_data_t *state, uint32_t new_hash)
{
    if (!state) {
        return false;
    }
    
    pthread_mutex_lock(&state->mutex);
    bool changed = (state->a0_hash != new_hash && state->a0_hash != 0);
    pthread_mutex_unlock(&state->mutex);
    
    return changed;
}

