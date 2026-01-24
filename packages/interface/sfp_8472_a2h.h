/**
 * @file sfp_8472_a2h.h
 * @brief Biblioteca de parsing SFF-8472 – Página A2h (Diagnósticos)
 *
 * @author Alexandre Junior
 * @author Miguel Ryan
 * @author Carlos Elias
 * @author Pablo Daniel
 * @author Pedro Lucena
 * @author Nicholas Gomes
 * @author Pedro Wilson
 * @author Melquisedeque Leite
 * @date 2026-01-23
 *
 * @details
 *  Implementa a leitura e interpretação dos campos da EEPROM A2h
 *  de módulos SFP/SFP+, conforme a especificação SFF-8472.
 *  A página A2h contém os diagnósticos em tempo real (DDM - Digital Diagnostic Monitoring).
 */

#ifndef SFF_8472_A2H_H
#define SFF_8472_A2H_H

#include <stdint.h>
#include <stdbool.h>

/************************************
 * Basic Type Definitions
 *************************************/

/** @brief Endereço I2C para EEPROM A2h (Diagnósticos) */
#define SFP_I2C_ADDR_A2     0x51

/** @brief Tamanho total da EEPROM A2h */
#define SFP_A2_SIZE         256

/** @brief Offset dos diagnósticos na página A2h */
#define SFP_A2_DIAG_OFFSET  96

/** @brief Tamanho do bloco de diagnósticos (bytes 96-111) */
#define SFP_A2_DIAG_SIZE    16

/* ==============================
 * Alarm/Status Flags (Bytes 110-111)
 * ============================== */
typedef struct {
    /* Byte 110 - Alarm Flags */
    bool temp_alarm_high;
    bool temp_alarm_low;
    bool voltage_alarm_high;
    bool voltage_alarm_low;
    bool bias_alarm_high;
    bool bias_alarm_low;
    bool tx_power_alarm_high;
    bool tx_power_alarm_low;
    bool rx_power_alarm_high;
    bool rx_power_alarm_low;
    
    /* Byte 111 - Warning Flags */
    bool temp_warning_high;
    bool temp_warning_low;
    bool voltage_warning_high;
    bool voltage_warning_low;
    bool bias_warning_high;
    bool bias_warning_low;
    bool tx_power_warning_high;
    bool tx_power_warning_low;
    bool rx_power_warning_high;
    bool rx_power_warning_low;
} sfp_a2h_alarm_flags_t;

/**********************************************
 * A2h Memory Map - Diagnostic Fields
 **********************************************/
typedef struct {
    /* Bytes 96-97: Temperature (signed 16-bit, units of 1/256°C) */
    int16_t temperature_raw;
    float temperature_c;  /* Converted to Celsius */
    
    /* Bytes 98-99: Voltage (unsigned 16-bit, units of 100 µV = 0.1 mV) */
    uint16_t voltage_raw;
    float voltage_v;  /* Converted to Volts */
    
    /* Bytes 100-101: Bias Current (unsigned 16-bit, units of 2 µA) */
    uint16_t bias_current_raw;
    float bias_current_ma;  /* Converted to milliamps */
    
    /* Bytes 102-103: TX Power (unsigned 16-bit, units of 0.1 µW) */
    uint16_t tx_power_raw;
    float tx_power_mw;  /* Converted to milliwatts */
    float tx_power_dbm; /* Converted to dBm */
    
    /* Bytes 104-105: RX Power (unsigned 16-bit, units of 0.1 µW) */
    uint16_t rx_power_raw;
    float rx_power_mw;  /* Converted to milliwatts */
    float rx_power_dbm; /* Converted to dBm */
    
    /* Bytes 110-111: Alarm/Status Flags */
    sfp_a2h_alarm_flags_t alarms;
    
    /* Validity flags */
    bool temperature_valid;
    bool voltage_valid;
    bool bias_current_valid;
    bool tx_power_valid;
    bool rx_power_valid;
} sfp_a2h_diagnostics_t;

/**********************************************
 * Function Prototypes
 **********************************************/

/* Parse diagnostic data from A2h page */
void sfp_parse_a2h_diagnostics(const uint8_t *a2_data, sfp_a2h_diagnostics_t *diag);

/* Get individual diagnostic values */
float sfp_a2h_get_temperature_c(const sfp_a2h_diagnostics_t *diag);
float sfp_a2h_get_voltage_v(const sfp_a2h_diagnostics_t *diag);
float sfp_a2h_get_bias_current_ma(const sfp_a2h_diagnostics_t *diag);
float sfp_a2h_get_tx_power_dbm(const sfp_a2h_diagnostics_t *diag);
float sfp_a2h_get_rx_power_dbm(const sfp_a2h_diagnostics_t *diag);

/* Get alarm flags */
const sfp_a2h_alarm_flags_t *sfp_a2h_get_alarms(const sfp_a2h_diagnostics_t *diag);

/* Check if diagnostics are valid */
bool sfp_a2h_is_valid(const sfp_a2h_diagnostics_t *diag);

/* Print diagnostics (for debugging) */
void sfp_print_a2h_diagnostics(const sfp_a2h_diagnostics_t *diag);

#endif /* SFF_8472_A2H_H */

