/**
 * @file daemon_fsm.c
 * @brief Implementação da máquina de estados
 */

#include "daemon_fsm.h"
#include <string.h>
#include <time.h>
#include <syslog.h>

/* ============================================
 * INIT → ABSENT
 * ============================================ */
bool daemon_fsm_init_to_absent(sfp_daemon_state_data_t *state, bool presence_detected)
{
    if (!state) {
        return false;
    }
    
    pthread_mutex_lock(&state->mutex);
    
    if (state->state != SFP_STATE_INIT) {
        pthread_mutex_unlock(&state->mutex);
        return false;
    }
    
    if (!presence_detected) {
        state->state = SFP_STATE_ABSENT;
        syslog(LOG_INFO, "State transition: INIT -> ABSENT");
        pthread_mutex_unlock(&state->mutex);
        return true;
    }
    
    pthread_mutex_unlock(&state->mutex);
    return false;
}

/* ============================================
 * ABSENT → PRESENT
 * ============================================ */
bool daemon_fsm_absent_to_present(sfp_daemon_state_data_t *state)
{
    if (!state) {
        return false;
    }
    
    pthread_mutex_lock(&state->mutex);
    
    if (state->state != SFP_STATE_ABSENT) {
        pthread_mutex_unlock(&state->mutex);
        return false;
    }
    
    state->state = SFP_STATE_PRESENT;
    state->generation_id++;
    state->first_detected = time(NULL);
    state->i2c_error_count = 0;
    state->recovery_attempts = 0;
    
    syslog(LOG_INFO, "State transition: ABSENT -> PRESENT (generation_id: %lu)", 
           (unsigned long)state->generation_id);
    
    pthread_mutex_unlock(&state->mutex);
    return true;
}

/* ============================================
 * PRESENT → ABSENT
 * ============================================ */
bool daemon_fsm_present_to_absent(sfp_daemon_state_data_t *state)
{
    if (!state) {
        return false;
    }
    
    pthread_mutex_lock(&state->mutex);
    
    if (state->state != SFP_STATE_PRESENT) {
        pthread_mutex_unlock(&state->mutex);
        return false;
    }
    
    state->state = SFP_STATE_ABSENT;
    state->a0_valid = false;
    state->a2_valid = false;
    state->a0_hash = 0;
    memset(state->a0_raw, 0, sizeof(state->a0_raw));
    memset(state->a2_raw, 0, sizeof(state->a2_raw));
    state->i2c_error_count = 0;
    state->recovery_attempts = 0;
    
    syslog(LOG_INFO, "State transition: PRESENT -> ABSENT");
    
    pthread_mutex_unlock(&state->mutex);
    return true;
}

/* ============================================
 * PRESENT → ERROR
 * ============================================ */
bool daemon_fsm_present_to_error(sfp_daemon_state_data_t *state)
{
    if (!state) {
        return false;
    }
    
    pthread_mutex_lock(&state->mutex);
    
    if (state->state != SFP_STATE_PRESENT) {
        pthread_mutex_unlock(&state->mutex);
        return false;
    }
    
    state->state = SFP_STATE_ERROR;
    state->recovery_attempts = 0;
    
    syslog(LOG_WARNING, "State transition: PRESENT -> ERROR (i2c_error_count: %u)", 
           state->i2c_error_count);
    
    pthread_mutex_unlock(&state->mutex);
    return true;
}

/* ============================================
 * ERROR → PRESENT
 * ============================================ */
bool daemon_fsm_error_to_present(sfp_daemon_state_data_t *state)
{
    if (!state) {
        return false;
    }
    
    pthread_mutex_lock(&state->mutex);
    
    if (state->state != SFP_STATE_ERROR) {
        pthread_mutex_unlock(&state->mutex);
        return false;
    }
    
    state->state = SFP_STATE_PRESENT;
    state->i2c_error_count = 0;
    state->recovery_attempts = 0;
    
    syslog(LOG_INFO, "State transition: ERROR -> PRESENT (recovered)");
    
    pthread_mutex_unlock(&state->mutex);
    return true;
}

/* ============================================
 * ERROR → ABSENT
 * ============================================ */
bool daemon_fsm_error_to_absent(sfp_daemon_state_data_t *state, bool presence_detected)
{
    if (!state) {
        return false;
    }
    
    pthread_mutex_lock(&state->mutex);
    
    if (state->state != SFP_STATE_ERROR) {
        pthread_mutex_unlock(&state->mutex);
        return false;
    }
    
    if (!presence_detected) {
        state->state = SFP_STATE_ABSENT;
        state->a0_valid = false;
        state->a2_valid = false;
        state->a0_hash = 0;
        memset(state->a0_raw, 0, sizeof(state->a0_raw));
        memset(state->a2_raw, 0, sizeof(state->a2_raw));
        state->i2c_error_count = 0;
        state->recovery_attempts = 0;
        
        syslog(LOG_INFO, "State transition: ERROR -> ABSENT (SFP removed)");
        pthread_mutex_unlock(&state->mutex);
        return true;
    }
    
    pthread_mutex_unlock(&state->mutex);
    return false;
}

/* ============================================
 * Estado para String
 * ============================================ */
const char *daemon_fsm_state_to_string(sfp_daemon_state_t state)
{
    switch (state) {
        case SFP_STATE_INIT:
            return "INIT";
        case SFP_STATE_ABSENT:
            return "ABSENT";
        case SFP_STATE_PRESENT:
            return "PRESENT";
        case SFP_STATE_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

