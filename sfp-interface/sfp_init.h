/**
 * @file sfp_init.h
 * @brief Funções de inicialização e leitura completa de módulos SFP
 */

#ifndef SFP_INIT_H
#define SFP_INIT_H

#include <stdint.h>
#include <stdbool.h>
#include "a0h.h"
#include "a2h.h"

/**
 * @brief Estrutura que contém todos os dados lidos de um módulo SFP
 */
typedef struct {
    /* Dados brutos */
    uint8_t a0_raw[SFP_A0_SIZE];
    uint8_t a2_raw[SFP_A2_SIZE];

    /* Estruturas interpretadas */
    sfp_a0h_base_t a0;
    sfp_a2h_t a2;

    /* Status de leitura */
    bool a0_valid;
    bool a2_valid;

    /* File descriptor I2C */
    int i2c_fd;
} sfp_module_t;

/**
 * @brief Inicializa e lê completamente um módulo SFP
 *
 * @param module Ponteiro para a estrutura que receberá os dados
 * @param i2c_device Caminho do dispositivo I2C (ex: "/dev/i2c-1")
 * @return true se a inicialização foi bem-sucedida, false caso contrário
 */
bool sfp_init(sfp_module_t *module, const char *i2c_device);

/**
 * @brief Libera recursos associados ao módulo SFP
 *
 * @param module Ponteiro para a estrutura do módulo
 */
void sfp_cleanup(sfp_module_t *module);

/**
 * @brief Imprime todas as informações lidas do módulo SFP
 *
 * @param module Ponteiro para a estrutura do módulo
 */
void sfp_info(const sfp_module_t *module);

/**
 * @brief Imprime dump hexadecimal dos dados brutos
 *
 * @param module Ponteiro para a estrutura do módulo
 */
void sfp_dump(const sfp_module_t *module);

#endif /* SFP_INIT_H */
