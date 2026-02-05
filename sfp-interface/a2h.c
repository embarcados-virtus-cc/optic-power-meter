#include "a2h.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ============================================
 * Parse Diagnostic Data from A2h Page
 * ============================================ */
void sfp_parse_a2h_diagnostics(const uint8_t *a2_data, sfp_a2h_diagnostics_t *diag)
{
    if (!a2_data || !diag) {
        return;
    }

    memset(diag, 0, sizeof(*diag));

    /* Bytes 96-97: Temperature */
    diag->temperature_raw = ((int16_t)a2_data[96] << 8) | a2_data[97];
    if (diag->temperature_raw != 0 && diag->temperature_raw != (int16_t)0x8000) {
        diag->temperature_c = (float)diag->temperature_raw / 256.0f;
        diag->temperature_valid = true;
    } else {
        diag->temperature_valid = false;
    }

    /* Bytes 98-99: Voltage */
    diag->voltage_raw = ((uint16_t)a2_data[98] << 8) | a2_data[99];
    if (diag->voltage_raw != 0 && diag->voltage_raw != 0xFFFF) {
        diag->voltage_v = (float)diag->voltage_raw * 0.0001f; /* 100 µV = 0.0001 V */
        diag->voltage_valid = true;
    } else {
        diag->voltage_valid = false;
    }

    /* Bytes 100-101: Bias Current */
    diag->bias_current_raw = ((uint16_t)a2_data[100] << 8) | a2_data[101];
    if (diag->bias_current_raw != 0 && diag->bias_current_raw != 0xFFFF) {
        diag->bias_current_ma = (float)diag->bias_current_raw * 0.002f; /* 2 µA = 0.002 mA */
        diag->bias_current_valid = true;
    } else {
        diag->bias_current_valid = false;
    }

    /* Bytes 102-103: TX Power */
    diag->tx_power_raw = ((uint16_t)a2_data[102] << 8) | a2_data[103];
    if (diag->tx_power_raw != 0 && diag->tx_power_raw != 0xFFFF) {
        /* 0.1 µW = 0.1 * 10^-6 W = 0.1 * 10^-3 mW = 0.0001 mW */
        diag->tx_power_mw = (float)diag->tx_power_raw * 0.0001f;
        if (diag->tx_power_mw > 0.0f) {
            diag->tx_power_dbm = 10.0f * log10f(diag->tx_power_mw);
        } else {
            diag->tx_power_dbm = -40.0f;
        }
        diag->tx_power_valid = true;
    } else {
        diag->tx_power_valid = false;
    }

    /* Bytes 104-105: RX Power */
    diag->rx_power_raw = ((uint16_t)a2_data[104] << 8) | a2_data[105];
    if (diag->rx_power_raw != 0 && diag->rx_power_raw != 0xFFFF) {
        /* 0.1 µW = 0.1 * 10^-6 W = 0.1 * 10^-3 mW = 0.0001 mW */
        diag->rx_power_mw = (float)diag->rx_power_raw * 0.0001f;
        if (diag->rx_power_mw > 0.0f) {
            diag->rx_power_dbm = 10.0f * log10f(diag->rx_power_mw);
        } else {
            diag->rx_power_dbm = -40.0f; /* Minimum readable value */
        }
        diag->rx_power_valid = true;
    } else {
        diag->rx_power_valid = false;
    }

    /* Bytes 112-113: Alarm Flags (corrigido conforme SFF-8472) */
    uint8_t alarm_byte_high = a2_data[112];  /* Byte 112 */
    uint8_t alarm_byte_low = a2_data[113];   /* Byte 113 */

    /* Byte 112 - TX/RX Alarm Flags */
    diag->alarms.tx_power_alarm_high  = (alarm_byte_high & (1 << 7)) != 0;
    diag->alarms.tx_power_alarm_low   = (alarm_byte_high & (1 << 6)) != 0;
    diag->alarms.rx_power_alarm_high  = (alarm_byte_high & (1 << 5)) != 0;
    diag->alarms.rx_power_alarm_low   = (alarm_byte_high & (1 << 4)) != 0;
    diag->alarms.tx_power_warning_high = (alarm_byte_high & (1 << 3)) != 0;
    diag->alarms.tx_power_warning_low  = (alarm_byte_high & (1 << 2)) != 0;
    diag->alarms.rx_power_warning_high = (alarm_byte_high & (1 << 1)) != 0;
    diag->alarms.rx_power_warning_low  = (alarm_byte_high & (1 << 0)) != 0;

    /* Byte 113 - Temp/Voltage/Bias Alarm Flags */
    diag->alarms.temp_alarm_high      = (alarm_byte_low & (1 << 7)) != 0;
    diag->alarms.temp_alarm_low       = (alarm_byte_low & (1 << 6)) != 0;
    diag->alarms.voltage_alarm_high   = (alarm_byte_low & (1 << 5)) != 0;
    diag->alarms.voltage_alarm_low    = (alarm_byte_low & (1 << 4)) != 0;
    diag->alarms.temp_warning_high    = (alarm_byte_low & (1 << 3)) != 0;
    diag->alarms.temp_warning_low     = (alarm_byte_low & (1 << 2)) != 0;
    diag->alarms.voltage_warning_high = (alarm_byte_low & (1 << 1)) != 0;
    diag->alarms.voltage_warning_low  = (alarm_byte_low & (1 << 0)) != 0;

    /* Bytes 116-117: Bias Current Alarm/Warning Flags */
    uint8_t bias_alarm_byte = a2_data[116];

    diag->alarms.bias_alarm_high      = (bias_alarm_byte & (1 << 7)) != 0;
    diag->alarms.bias_alarm_low       = (bias_alarm_byte & (1 << 6)) != 0;
    /* Bits 5-4 são reservados */
    diag->alarms.bias_warning_high    = (bias_alarm_byte & (1 << 3)) != 0;
    diag->alarms.bias_warning_low     = (bias_alarm_byte & (1 << 2)) != 0;
    /* Bits 1-0 são reservados */
}

/* ============================================
 * Get Individual Diagnostic Values
 * ============================================ */
float sfp_a2h_get_temperature_c(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag || !diag->temperature_valid)
        return 0.0f;
    return diag->temperature_c;
}

float sfp_a2h_get_voltage_v(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag || !diag->voltage_valid)
        return 0.0f;
    return diag->voltage_v;
}

float sfp_a2h_get_bias_current_ma(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag || !diag->bias_current_valid)
        return 0.0f;
    return diag->bias_current_ma;
}

float sfp_a2h_get_tx_power_dbm(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag || !diag->tx_power_valid)
        return -40.0f;
    return diag->tx_power_dbm;
}

float sfp_a2h_get_rx_power_dbm(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag || !diag->rx_power_valid)
        return -40.0f;
    return diag->rx_power_dbm;
}

/* ============================================
 * Get Alarm Flags
 * ============================================ */
const sfp_a2h_alarm_flags_t *sfp_a2h_get_alarms(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag)
        return NULL;
    return &diag->alarms;
}

/* ============================================
 * Check if Diagnostics are Valid
 * ============================================ */
bool sfp_a2h_is_valid(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag)
        return false;
    return diag->temperature_valid || diag->voltage_valid ||
           diag->bias_current_valid || diag->tx_power_valid ||
           diag->rx_power_valid;
}

/* ============================================
 * Função de Exposição
 * ============================================ */
void sfp_print_a2h_diagnostics(const sfp_a2h_diagnostics_t *diag)
{
    if (!diag) {
        printf("Diagnostics: NULL\n");
        return;
    }

    printf("\n=== SFP A2h Diagnostics ===\n");

    if (diag->temperature_valid) {
        printf("Temperature: %.2f °C (raw: %d)\n", diag->temperature_c, diag->temperature_raw);
    } else {
        printf("Temperature: N/A\n");
    }

    if (diag->voltage_valid) {
        printf("Voltage: %.3f V (raw: 0x%04X)\n", diag->voltage_v, diag->voltage_raw);
    } else {
        printf("Voltage: N/A\n");
    }

    if (diag->bias_current_valid) {
        printf("Bias Current: %.2f mA (raw: 0x%04X)\n", diag->bias_current_ma, diag->bias_current_raw);
    } else {
        printf("Bias Current: N/A\n");
    }

    if (diag->tx_power_valid) {
        printf("TX Power: %.2f dBm (%.4f mW, raw: 0x%04X)\n",
               diag->tx_power_dbm, diag->tx_power_mw, diag->tx_power_raw);
    } else {
        printf("TX Power: N/A\n");
    }

    if (diag->rx_power_valid) {
        printf("RX Power: %.2f dBm (%.4f mW, raw: 0x%04X)\n",
               diag->rx_power_dbm, diag->rx_power_mw, diag->rx_power_raw);
    } else {
        printf("RX Power: N/A\n");
    }

    printf("\n=== Alarm Flags ===\n");
    printf("Temperature:\n");
    printf("  Alarm  - High: %s, Low: %s\n",
           diag->alarms.temp_alarm_high ? "ON" : "OFF",
           diag->alarms.temp_alarm_low ? "ON" : "OFF");
    printf("  Warning - High: %s, Low: %s\n",
           diag->alarms.temp_warning_high ? "ON" : "OFF",
           diag->alarms.temp_warning_low ? "ON" : "OFF");

    printf("\nVoltage:\n");
    printf("  Alarm  - High: %s, Low: %s\n",
           diag->alarms.voltage_alarm_high ? "ON" : "OFF",
           diag->alarms.voltage_alarm_low ? "ON" : "OFF");
    printf("  Warning - High: %s, Low: %s\n",
           diag->alarms.voltage_warning_high ? "ON" : "OFF",
           diag->alarms.voltage_warning_low ? "ON" : "OFF");

    printf("\nBias Current:\n");
    printf("  Alarm  - High: %s, Low: %s\n",
           diag->alarms.bias_alarm_high ? "ON" : "OFF",
           diag->alarms.bias_alarm_low ? "ON" : "OFF");
    printf("  Warning - High: %s, Low: %s\n",
           diag->alarms.bias_warning_high ? "ON" : "OFF",
           diag->alarms.bias_warning_low ? "ON" : "OFF");

    printf("\nTX Power:\n");
    printf("  Alarm  - High: %s, Low: %s\n",
           diag->alarms.tx_power_alarm_high ? "ON" : "OFF",
           diag->alarms.tx_power_alarm_low ? "ON" : "OFF");
    printf("  Warning - High: %s, Low: %s\n",
           diag->alarms.tx_power_warning_high ? "ON" : "OFF",
           diag->alarms.tx_power_warning_low ? "ON" : "OFF");

    printf("\nRX Power:\n");
    printf("  Alarm  - High: %s, Low: %s\n",
           diag->alarms.rx_power_alarm_high ? "ON" : "OFF",
           diag->alarms.rx_power_alarm_low ? "ON" : "OFF");
    printf("  Warning - High: %s, Low: %s\n",
           diag->alarms.rx_power_warning_high ? "ON" : "OFF",
           diag->alarms.rx_power_warning_low ? "ON" : "OFF");
}
