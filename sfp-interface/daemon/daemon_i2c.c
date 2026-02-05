/**
 * @file daemon_i2c.c
 * @brief Implementação das funções I²C do daemon
 */

#include "daemon_i2c.h"
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <syslog.h>

/* ============================================
 * Detecta Presença de Endereço
 * ============================================ */
bool daemon_i2c_detect_address(int i2c_fd, uint8_t addr)
{
    if (i2c_fd < 0) {
        return false;
    }
    
    /* Tenta configurar endereço do dispositivo I²C */
    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
        return false;  /* Dispositivo não responde */
    }
    
    /* Tenta ler 1 byte (qualquer offset) */
    uint8_t dummy;
    uint8_t offset = 0;
    
    /* Escreve offset */
    if (write(i2c_fd, &offset, 1) != 1) {
        return false;
    }
    
    /* Tenta ler */
    ssize_t bytes_read = read(i2c_fd, &dummy, 1);
    return (bytes_read == 1);
}

/* ============================================
 * Detecta Presença de SFP
 * ============================================ */
bool daemon_i2c_detect_presence(int i2c_fd)
{
    if (i2c_fd < 0) {
        return false;
    }
    
    /* Verifica ambos os endereços (0x50 e 0x51) */
    bool a0_present = daemon_i2c_detect_address(i2c_fd, SFP_I2C_ADDR_A0);
    bool a2_present = daemon_i2c_detect_address(i2c_fd, SFP_I2C_ADDR_A2);
    
    /* SFP está presente se ambos os endereços respondem */
    return a0_present && a2_present;
}

/* ============================================
 * Lê Dados A0h
 * ============================================ */
bool daemon_i2c_read_a0h(int i2c_fd, uint8_t *a0_raw)
{
    if (i2c_fd < 0 || !a0_raw) {
        return false;
    }
    
    /* Usa função existente da biblioteca i2c */
    bool success = sfp_read_block(
        i2c_fd,
        SFP_I2C_ADDR_A0,
        0x00,
        a0_raw,
        SFP_A0_BASE_SIZE
    );
    
    if (!success) {
        syslog(LOG_DEBUG, "Failed to read A0h");
    }
    
    return success;
}

/* ============================================
 * Lê Dados A2h
 * ============================================ */
bool daemon_i2c_read_a2h(int i2c_fd, uint8_t *a2_raw)
{
    if (i2c_fd < 0 || !a2_raw) {
        return false;
    }
    
    /* Usa função existente da biblioteca i2c */
    bool success = sfp_read_block(
        i2c_fd,
        SFP_I2C_ADDR_A2,
        SFP_A2_DIAG_OFFSET,
        a2_raw,
        SFP_A2_DIAG_SIZE
    );
    
    if (!success) {
        syslog(LOG_DEBUG, "Failed to read A2h");
    }
    
    return success;
}

