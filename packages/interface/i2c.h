#ifndef I2C_H
#define I2C_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
/* ============================================
 * I2C Initialization and Control (Linux)
 * ============================================ */

/**
 * @brief Inicializa o barramento I2C
 *
 * @param device Caminho do dispositivo I2C (ex: "/dev/i2c-1")
 * @return File descriptor (>= 0) em caso de sucesso, -1 em caso de erro
 */
int sfp_i2c_init(const char *device);

/**
 * @brief Fecha o descritor do barramento I2C
 *
 * @param fd File descriptor retornado por sfp_i2c_init()
 */
void sfp_i2c_close(int fd);

/* ============================================
 * Memory Access
 * ============================================ */

/**
 * @brief Lê um bloco de bytes da EEPROM do SFP
 *
 * @param fd File descriptor do barramento I2C
 * @param dev_addr Endereço I2C do dispositivo (SFP_I2C_ADDR_A0 ou SFP_I2C_ADDR_A2)
 * @param start_offset Offset inicial na EEPROM
 * @param buffer Buffer para armazenar os dados lidos
 * @param length Número de bytes a ler
 * @return true em caso de sucesso, false em caso de erro
 */
bool sfp_read_block(int fd, uint8_t dev_addr, uint8_t start_offset, uint8_t *buffer, uint8_t length);

#endif
