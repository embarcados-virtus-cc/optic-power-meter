#ifndef SFP_A2H_H
#define SFP_A2H_H

#include "defs.h"
#include <stdint.h>
#include <stdbool.h>

#define SFP_I2C_ADDR_A2 0x51
/*SIZE do Bloco do A2H*/
#define SFP_A2_SIZE 128

// Estrutura para os Limiares de Alarme e Aviso (Bytes 0-55)
typedef struct {
    uint16_t temp_high_alarm;    // Bytes 00-01
    uint16_t temp_low_alarm;     // Bytes 02-03
    uint16_t temp_high_warning;  // Bytes 04-05
    uint16_t temp_low_warning;   // Bytes 06-07
    uint16_t voltage_high_alarm; // Bytes 08-09
    uint16_t voltage_low_alarm;  // Bytes 10-11
    uint16_t voltage_high_warning;// Bytes 12-13
    uint16_t voltage_low_warning; // Bytes 14-15
    uint16_t bias_high_alarm;    // Bytes 16-17
    uint16_t bias_low_alarm;     // Bytes 18-19
    uint16_t bias_high_warning;  // Bytes 20-21
    uint16_t bias_low_warning;   // Bytes 22-23
    uint16_t tx_power_high_alarm; // Bytes 24-25
    uint16_t tx_power_low_alarm;  // Bytes 26-27
    uint16_t tx_power_high_warning;// Bytes 28-29
    uint16_t tx_power_low_warning; // Bytes 30-31
    uint16_t rx_power_high_alarm; // Bytes 32-33
    uint16_t rx_power_low_alarm;  // Bytes 34-35
    uint16_t rx_power_high_warning;// Bytes 36-37
    uint16_t rx_power_low_warning; // Bytes 38-39
    
    // Limiares Opcionais (DWDM/Laser)
    uint16_t laser_temp_high_alarm; // Bytes 40-41
    uint16_t laser_temp_low_alarm;  // Bytes 42-43
    uint16_t laser_temp_high_warning;// Bytes 44-45
    uint16_t laser_temp_low_warning; // Bytes 46-47
    uint16_t tec_current_high_alarm; // Bytes 48-49
    uint16_t tec_current_low_alarm;  // Bytes 50-51
    uint16_t tec_current_high_warning;// Bytes 52-53
    uint16_t tec_current_low_warning; // Bytes 54-55
} sfp_a2h_thresholds_t;



// Estrutura principal da Página A2h
typedef struct {
    // 0-55: Limiares
    sfp_a2h_thresholds_t thresholds;

    // 56-91: Área Dinâmica (Calibração Externa ou Recursos Avançados)
    // Depende do Byte 92, bit 4 do endereço A0h
    union {
        uint8_t external_cal_constants[8]; // Se Calibração Externa = 1
        uint8_t enhanced_features[8];      // Se Calibração Externa = 0
    } calibration_enhanced;

    //uint8_t reserved_92_94[9];      Bytes 92-94 [10]
    uint8_t cc_dmi;                 // Byte 95: Checksum dos diagnósticos

    // 96-109: Dados de Diagnóstico em Tempo Real [10, 12]
    double temp_realtime;         // Bytes 96-97
    double vcc_realtime;          // Bytes 98-99
    double tx_bias_realtime;      // Bytes 100-101
    double tx_power_realtime;     // Bytes 102-103
    double rx_power_realtime;     // Bytes 104-105
    double laser_temp_wave_opt;   // Bytes 106-107 (Opcional)
    double tec_current_opt;       // Bytes 108-109 (Opcional)

   /* uint8_t status_control;         // Byte 110 */
    bool data_ready;
    //uint8_t reserved_111;          Byte 111
    uint8_t alarm_flags[15];         // Bytes 112-113 
    uint8_t tx_input_eq_ctrl;       // Byte 114 
    uint8_t rx_output_emph_ctrl;    // Byte 115
    uint8_t warning_flags[15];       // Bytes 116-117 
    uint8_t ext_status_control[15];  // Bytes 118-119
    uint8_t vendor_specific[20];     // Bytes 120-126
    uint8_t table_select;           // Byte 127: SELETOR DE PÁGINA

    // 128-255: Memória Paginada (Depende do Byte 127)
    union {
        struct {
            uint8_t user_eeprom[22];   // Bytes 128-247 (Páginas 00h/01h)
            uint8_t vendor_control[24];  // Bytes 248-255 (Páginas 00h/01h)
        } page_00_01;
        
        uint8_t page_02_tunability[26]; // Página 02h: Tunability e RPM
        uint8_t page_03_high_accuracy[26]; // Página 03h: Timing Calibrations
        uint8_t raw_upper_memory[26];
    } upper_memory;

} sfp_a2h_t;


bool check_sfp_a2h_exists(const uint8_t *a2_data);
bool get_sfp_vcc(const uint8_t *a2_data, float *vcc);


void sfp_parse_a2h_data_ready(const uint8_t *a2_data, sfp_a2h_t *a2);
bool sfp_a2h_get_data_ready(const sfp_a2h_t *a2);


#endif