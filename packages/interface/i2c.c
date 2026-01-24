#include "i2c.h"
#include <stdint.h>
#include <fcntl.h>

/* ============================================
 * I2C Initialization and Control
 * ============================================ */

/**
 * @brief Inicializa o barramento I2C
 */
int sfp_i2c_init(const char *device)
{
    if (!device) {
        fprintf(stderr, "Erro: dispositivo I2C não especificado\n");
        return -1;
    }

    int fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("Erro ao abrir barramento I2C");
        return -1;
    }
    return fd;
}

/**
 * @brief Fecha o descritor do barramento I2C
 */
void sfp_i2c_close(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @brief Lê um bloco de bytes da EEPROM do SFP
 */
bool sfp_read_block(int fd, uint8_t dev_addr, uint8_t start_offset, uint8_t *buffer, uint8_t length)
{
    if (fd < 0 || !buffer || length == 0) {
        return false;
    }

    /* Configura o endereço do dispositivo I2C slave */
    if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
        perror("Erro ao configurar endereço I2C");
        return false;
    }

    /* Escreve o offset inicial */
    if (write(fd, &start_offset, 1) != 1) {
        perror("Erro ao escrever offset I2C");
        return false;
    }

    /* Lê os dados */
    ssize_t bytes_read = read(fd, buffer, length);
    if (bytes_read != length) {
        if (bytes_read < 0) {
            perror("Erro ao ler dados I2C");
        } else {
            fprintf(stderr, "Erro: lidos %zd bytes, esperados %d\n", bytes_read, length);
        }
        return false;
    }

    return true;
}
