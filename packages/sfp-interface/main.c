/**
 * @file main.c
 * @brief Aplicação de leitura de módulos SFP via I2C para Raspberry Pi (Linux)
 *
 * Demonstra a leitura da EEPROM A0h de módulos SFP/SFP+ conforme SFF-8472.
 */

#include <stdio.h>
#include <stdlib.h>

#include "sfp_init.h"

/* Barramento I2C padrão no Raspberry Pi */
#define I2C_DEVICE "/dev/i2c-1"

int main(int argc, char *argv[])
{
    const char *i2c_device = I2C_DEVICE;

    /* Permite especificar o dispositivo I2C via argumento */
    if (argc > 1) {
        i2c_device = argv[1];
    }

    printf("=== Leitor de Módulos SFP ===\n");
    printf("Dispositivo I2C: %s\n\n", i2c_device);

    /* Estrutura do módulo SFP */
    sfp_module_t module = {0};

    /* Inicializa e lê o módulo */
    if (!sfp_init(&module, i2c_device)) {
        fprintf(stderr, "\nFalha ao inicializar o módulo SFP\n");
        return EXIT_FAILURE;
    }

    sfp_info(&module);

    /* Imprime dump dos dados brutos (opcional) */
    sfp_dump(&module);

    printf("\n=== Leitura concluída com sucesso ===\n");

    /* Libera recursos */
    sfp_cleanup(&module);

    return EXIT_SUCCESS;
}
