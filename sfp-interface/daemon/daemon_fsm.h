/**
 * @file daemon_fsm.h
 * @brief Máquina de estados do daemon SFP
 */

#ifndef DAEMON_FSM_H
#define DAEMON_FSM_H

#include "daemon_state.h"

/* ============================================
 * Funções de Transição de Estado
 * ============================================ */

/**
 * @brief Processa transição INIT → ABSENT
 * @param state Ponteiro para estrutura de estado
 * @param presence_detected true se SFP foi detectado, false caso contrário
 * @return true se transição ocorreu, false caso contrário
 */
bool daemon_fsm_init_to_absent(sfp_daemon_state_data_t *state, bool presence_detected);

/**
 * @brief Processa transição ABSENT → PRESENT
 * @param state Ponteiro para estrutura de estado
 * @return true se transição ocorreu, false caso contrário
 */
bool daemon_fsm_absent_to_present(sfp_daemon_state_data_t *state);

/**
 * @brief Processa transição PRESENT → ABSENT
 * @param state Ponteiro para estrutura de estado
 * @return true se transição ocorreu, false caso contrário
 */
bool daemon_fsm_present_to_absent(sfp_daemon_state_data_t *state);

/**
 * @brief Processa transição PRESENT → ERROR
 * @param state Ponteiro para estrutura de estado
 * @return true se transição ocorreu, false caso contrário
 */
bool daemon_fsm_present_to_error(sfp_daemon_state_data_t *state);

/**
 * @brief Processa transição ERROR → PRESENT
 * @param state Ponteiro para estrutura de estado
 * @return true se transição ocorreu, false caso contrário
 */
bool daemon_fsm_error_to_present(sfp_daemon_state_data_t *state);

/**
 * @brief Processa transição ERROR → ABSENT
 * @param state Ponteiro para estrutura de estado
 * @param presence_detected true se SFP ainda está presente, false caso contrário
 * @return true se transição ocorreu, false caso contrário
 */
bool daemon_fsm_error_to_absent(sfp_daemon_state_data_t *state, bool presence_detected);

/**
 * @brief Obtém string do estado atual
 * @param state Estado da máquina
 * @return String do estado
 */
const char *daemon_fsm_state_to_string(sfp_daemon_state_t state);

#endif /* DAEMON_FSM_H */

