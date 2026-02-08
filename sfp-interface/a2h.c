#include "a2h.h"
#include <math.h>

/* ============================================
 * Byte 00-01 -High Temperature Alarm
 * ============================================ */

void sfp_parse_a2h_temp_high_alarm(const uint8_t *a2_data,sfp_a2h_t *a2){
  if(!a2_data || !a2){
    return;
  }
  uint16_t raw_temp;

  uint8_t msb = a2_data[A2_TEMP_HIGH_ALARM]; 
  uint8_t lsb = a2_data[A2_TEMP_HIGH_ALARM + 1];/*Deslocando um byte para pegar o msb */

  raw_temp = ((msb << 8) | lsb);
  a2->thresholds.temp_high_alarm = TEMP_TO_DEGC(raw_temp);

}
/* ============================================
 * Função Getter
 * ============================================ */

float sfp_a2h_get_temp_high_alarm(const sfp_a2h_t *a2){
  if (!a2) {
    return -1;/*INDICA UM ERRO*/
  }

  return a2->thresholds.temp_high_alarm;
}

/* ============================================
 * Byte 02-03 -Low Temperature Alarm
 * ============================================ */
 
 void sfp_parse_a2h_temp_low_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TEMP_LOW_ALARM];
    uint8_t lsb = a2_data[A2_TEMP_LOW_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.temp_low_alarm = TEMP_TO_DEGC(raw);
}

/* ============================================
 * Função Getter
 * ============================================ */
float sfp_a2h_get_temp_low_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.temp_low_alarm;
}

/* ============================================
 * Byte 04-05 - High Temperature Warning
 * ============================================ */
 void sfp_parse_a2h_temp_high_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TEMP_HIGH_WARNING];
    uint8_t lsb = a2_data[A2_TEMP_HIGH_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.temp_high_warning = TEMP_TO_DEGC(raw);
}
 
 /* ============================================
 * Função Getter
 * ============================================ */
 float sfp_a2h_get_temp_high_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.temp_high_warning;
}

/* ============================================
 * Byte 06-07 - LowTemperature Warning
 * ============================================ */
 void sfp_parse_a2h_temp_low_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TEMP_LOW_WARNING];
    uint8_t lsb = a2_data[A2_TEMP_LOW_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.temp_low_warning = TEMP_TO_DEGC(raw);
}

 /* ============================================
 * Função Getter
 * ============================================ */

float sfp_a2h_get_temp_low_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.temp_low_warning;
}

/* ============================================
 * Byte 08-09 - High Alarm VCC
 * ============================================ */
 void sfp_parse_a2h_vcc_high_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_VCC_HIGH_ALARM];
    uint8_t lsb = a2_data[A2_VCC_HIGH_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.vcc_high_alarm = VCC_TO_VOLTS(raw);
}

/* ============================================
 * Função Getter
 * ============================================ */
float sfp_a2h_get_vcc_high_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.vcc_high_alarm;
}

/* ============================================
 * Byte 10-11 - LOW Alarm VCC
 * ============================================ */
 void sfp_parse_a2h_vcc_low_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_VCC_LOW_ALARM];
    uint8_t lsb = a2_data[A2_VCC_LOW_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.vcc_low_alarm = VCC_TO_VOLTS(raw);
}

/* ============================================
 * Função Getter
 * ============================================ */

float sfp_a2h_get_vcc_low_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.vcc_low_alarm;
}

/* ============================================
 * Byte 12-13 - High Warning VCC
 * ============================================ */

void sfp_parse_a2h_vcc_high_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_VCC_HIGH_WARNING];
    uint8_t lsb = a2_data[A2_VCC_HIGH_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.vcc_high_warning = VCC_TO_VOLTS(raw);
}
/* ============================================
 * Função Getter
 * ============================================ */
float sfp_a2h_get_vcc_high_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.vcc_high_warning;
}
 
/* ============================================
 * Byte 14-15 - Low Warning VCC
 * ============================================ */
void sfp_parse_a2h_vcc_low_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_VCC_LOW_WARNING];
    uint8_t lsb = a2_data[A2_VCC_LOW_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.vcc_low_warning = VCC_TO_VOLTS(raw);
}
/* ============================================
 * Função Getter
 * ============================================ */
float sfp_a2h_get_vcc_low_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.vcc_low_warning;
}


/* ============================================
 * Byte 16-17 - High Alarm BIAS
 * ============================================ */
 
 void sfp_parse_a2h_tx_bias_high_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_BIAS_HIGH_ALARM];
    uint8_t lsb = a2_data[A2_TX_BIAS_HIGH_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_bias_high_alarm = TX_BIAS_TO_MA(raw);
}

/* ============================================
 * Função Getter
 * ============================================ */
float sfp_a2h_get_tx_bias_high_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_bias_high_alarm;
}

/* ============================================
 * Byte 18-19 - Low Alarm BIAS
 * ============================================ */
 void sfp_parse_a2h_tx_bias_low_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_BIAS_LOW_ALARM];
    uint8_t lsb = a2_data[A2_TX_BIAS_LOW_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_bias_low_alarm = TX_BIAS_TO_MA(raw);
}
/* ============================================
 * Função Getter
 * ============================================ */
float sfp_a2h_get_tx_bias_low_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_bias_low_alarm;
}

/* ============================================
 * Byte 20-21 - High Warning BIAS
 * ============================================ */
void sfp_parse_a2h_tx_bias_high_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_BIAS_HIGH_WARNING];
    uint8_t lsb = a2_data[A2_TX_BIAS_HIGH_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_bias_high_warning = TX_BIAS_TO_MA(raw);
}
/* ============================================
 * Função Getter
 * ============================================ */
float sfp_a2h_get_tx_bias_high_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_bias_high_warning;
}


/* ============================================
 * Byte 22-23 - Low Warning BIAS
 * ============================================ */
void sfp_parse_a2h_tx_bias_low_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_BIAS_LOW_WARNING];
    uint8_t lsb = a2_data[A2_TX_BIAS_LOW_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_bias_low_warning = TX_BIAS_TO_MA(raw);
}

float sfp_a2h_get_tx_bias_low_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_bias_low_warning;
}


/* ============================================
 * Byte 24-25 - High Alarm TX
 * ============================================ */
void sfp_parse_a2h_tx_power_high_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_POWER_HIGH_ALARM];
    uint8_t lsb = a2_data[A2_TX_POWER_HIGH_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_power_high_alarm = POWER_TO_UW(raw);
}

float sfp_a2h_get_tx_power_high_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_power_high_alarm;
}



/* ============================================
 * Byte 26-27 - Low Alarm TX
 * ============================================ */
void sfp_parse_a2h_tx_power_low_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_POWER_LOW_ALARM];
    uint8_t lsb = a2_data[A2_TX_POWER_LOW_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_power_low_alarm = POWER_TO_UW(raw);
}

float sfp_a2h_get_tx_power_low_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_power_low_alarm;
}





/* ============================================
 * Byte 28-29 - High Warning TX
 * ============================================ */
void sfp_parse_a2h_tx_power_high_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_POWER_HIGH_WARNING];
    uint8_t lsb = a2_data[A2_TX_POWER_HIGH_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_power_high_warning = POWER_TO_UW(raw);
}

float sfp_a2h_get_tx_power_high_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_power_high_warning;
}


/* ============================================
 * Byte 30-21 - Low Warning TX
 * ============================================ */
void sfp_parse_a2h_tx_power_low_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_TX_POWER_LOW_WARNING];
    uint8_t lsb = a2_data[A2_TX_POWER_LOW_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.tx_power_low_warning = POWER_TO_UW(raw);
}

float sfp_a2h_get_tx_power_low_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.tx_power_low_warning;
}



void sfp_parse_a2h_rx_power_high_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_RX_POWER_HIGH_ALARM];
    uint8_t lsb = a2_data[A2_RX_POWER_HIGH_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.rx_power_high_alarm = POWER_TO_UW(raw);
}

float sfp_a2h_get_rx_power_high_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.rx_power_high_alarm;
}


void sfp_parse_a2h_rx_power_low_alarm(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_RX_POWER_LOW_ALARM];
    uint8_t lsb = a2_data[A2_RX_POWER_LOW_ALARM + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.rx_power_low_alarm = POWER_TO_UW(raw);
}

float sfp_a2h_get_rx_power_low_alarm(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.rx_power_low_alarm;
}


void sfp_parse_a2h_rx_power_high_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_RX_POWER_HIGH_WARNING];
    uint8_t lsb = a2_data[A2_RX_POWER_HIGH_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.rx_power_high_warning = POWER_TO_UW(raw);
}

float sfp_a2h_get_rx_power_high_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.rx_power_high_warning;
}


void sfp_parse_a2h_rx_power_low_warning(const uint8_t *a2_data, sfp_a2h_t *a2){
    if(!a2_data || !a2){
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_RX_POWER_LOW_WARNING];
    uint8_t lsb = a2_data[A2_RX_POWER_LOW_WARNING + 1];

    raw = (msb << 8) | lsb;
    a2->thresholds.rx_power_low_warning = POWER_TO_UW(raw);
}

float sfp_a2h_get_rx_power_low_warning(const sfp_a2h_t *a2){
    if(!a2){
        return -1;
    }
    return a2->thresholds.rx_power_low_warning;
}


/**
 * Verifica se o transceptor implementa a página de diagnósticos A2h.
 * @param a0_data Buffer contendo os 256 bytes lidos do endereço A0h.
 * @return true se A2h estiver disponível, false caso contrário.
 */

bool check_sfp_a2h_exists(const uint8_t *a0_data) {
    if (!a0_data) return false;

    // Obtém o byte de Tipo de Monitoramento de Diagnóstico (Byte 92 do A0h)
    uint8_t diag_type = a0_data[A0_DIAG_MONITORING_TYPE];

    // Conforme a Tabela 8-5, o Bit 6 indica se o monitoramento digital (A2h)
    // está implementado no transceptor.
    bool dmi_implemented = (diag_type & (1 << SFP_A0_BIT_DMI_IMPL));

    return dmi_implemented;
}

/**
 * Verifica se os dados de monitoramento (DMI) estão prontos para leitura.
 *
 * @param status_byte O valor lido do Byte 110 no endereço A2h.
 * @return true se os dados estiverem prontos (bit 0 == 0), false se não estiverem.
 */
bool sfp_check_data_ready(uint8_t status_byte) {
    // O bit Data_Not_Ready (bit 0) indica se o módulo tem dados válidos.
    // Se o bit for 1, os dados NÃO estão prontos.
    // Se o bit for 0, os dados ESTÃO prontos [3].

    if ((status_byte & (1 << SFP_A2_BIT_DATA_NOT_READY)) == 0) {
        return true;  // Dados válidos e prontos
    }

    return false; // Módulo ainda não processou a primeira leitura A/D
}

/**
 * Lê e interpreta a tensão de alimentação (Vcc).
 * @param a2_data Buffer de 256 bytes contendo a página A2h.
 * @param vcc Ponteiro para armazenar o valor em Volts.
 * @return true se a leitura for válida, false caso contrário.
 */
bool get_sfp_vcc(const uint8_t *a2_data, float *vcc) {
    if (!a2_data || !vcc) return false;


    /* Coerência de Dados
       A especificação exige que campos multi-byte (como o Vcc nos bytes 98-99)
       sejam recuperados usando uma sequência única de leitura de dois bytes
       para garantir a consistência dos dados. */
    uint16_t raw_vcc = (uint16_t)(a2_data[A2_VCC_CURR] << 8) | a2_data[ A2_VCC_CURR + 1];

    /* 3. Interpretação (Calibração Interna)
       O valor bruto (0-65535) representa a faixa de 0 a 6.55 V. */
    *vcc = VCC_TO_VOLTS(raw_vcc);

    return true;
}

/* ============================================
 * Byte 104-105 -RX_POWER
 * ============================================ */
 
/**
 * Faz o parse da potência de recepção (RX Power) em tempo real.
 * @param a2_data Buffer contendo os dados lidos da página A2h.
 * @param a2 Estrutura para armazenar os dados processados.
 */
void sfp_parse_a2h_rx_power(const uint8_t *a2_data, sfp_a2h_t *a2) {
    if (!a2_data || !a2) {
        return;
    }

    uint16_t raw;
    uint8_t msb = a2_data[A2_RX_POWER];
    uint8_t lsb = a2_data[A2_RX_POWER + 1];

    raw = (uint16_t)((msb << 8) | lsb);

    /* Converte o valor bruto para microwatts (uW). 
       O LSB é definido como 0,1 uW. */
    a2->rx_power = POWER_TO_UW(raw);
}
/* ============================================
 * Função Getter
 * ============================================ */

/**
 * Retorna o valor da potência RX processada.
 * @param a2 Ponteiro para a estrutura de dados do SFP.
 * @return Valor em uW ou -1 em caso de erro.
 */
float sfp_a2h_get_rx_power(const sfp_a2h_t *a2) {
    if (!a2) {
        return -1; /* Indica um erro */
    }

    return a2->rx_power;
}

float sfp_a2h_get_rx_power_dbm(const sfp_a2h_t *a2){
  if (!a2) {
    return -1;
  }
  float power_uW = a2->rx_power;
  if (power_uW <=0.0f) {
    return -40.0f;/*Piso condizente com a sensibilidade do Módulo*/
  }else {
    return 10.0f * log10f(power_uW /1000.0f);
  }
}


/* ============================================
 * Byte 110 -Data_Not_Ready
 * ============================================ */

void sfp_parse_a2h_data_ready(const uint8_t *a2_data,sfp_a2h_t *a2){
   if ((a2_data[STATUS_CONTROL]& (1 << SFP_A2_BIT_DATA_NOT_READY)) == 0) {
        a2->data_ready = true;  // Dados prontos para leitura
    }
   a2->data_ready = false;
}

bool sfp_a2h_get_data_ready(const sfp_a2h_t *a2){
  if (!a2){
    return false;
  }
  return a2->data_ready;
}
