/**
 * @file daemon_i2c.h
 * @brief Wrapper I²C para daemon (detecção de presença e polling)
 */

#ifndef DAEMON_I2C_H
#define DAEMON_I2C_H

#include <stdbool.h>
#include <stdint.h>
#include "../i2c.h"
#include "../a0h.h"
#include "../a2h.h"

/* ============================================
 * Funções de Detecção de Presença
 * ============================================ */

/**
 * @brief Detecta se SFP está presente no barramento I²C
 * @param i2c_fd File descriptor do dispositivo I²C
 * @return true se SFP detectado, false caso contrário
 */
bool daemon_i2c_detect_presence(int i2c_fd);

/**
 * @brief Detecta se endereço específico está presente
 * @param i2c_fd File descriptor do dispositivo I²C
 * @param addr Endereço I²C a verificar (0x50 ou 0x51)
 * @return true se endereço detectado, false caso contrário
 */
bool daemon_i2c_detect_address(int i2c_fd, uint8_t addr);

/* ============================================
 * Funções de Leitura
 * ============================================ */

/**
 * @brief Lê dados A0h completos
 * @param i2c_fd File descriptor do dispositivo I²C
 * @param a0_raw Buffer para dados brutos (deve ter pelo menos SFP_A0_SIZE bytes)
 * @return true se leitura bem-sucedida, false caso contrário
 */
bool daemon_i2c_read_a0h(int i2c_fd, uint8_t *a0_raw);

/**
 * @brief Lê dados A2h (diagnósticos)
 * @param i2c_fd File descriptor do dispositivo I²C
 * @param a2_raw Buffer para dados brutos (deve ter pelo menos SFP_A2_SIZE bytes)
 * @return true se leitura bem-sucedida, false caso contrário
 */
bool daemon_i2c_read_a2h(int i2c_fd, uint8_t *a2_raw);

#endif /* DAEMON_I2C_H */
