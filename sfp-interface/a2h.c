#include "a2h.h"
#include <stdio.h>
#include <string.h>


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
 * Byte 110 -Data_Not_Ready
 * ============================================ */

void sfp_parse_a2h_data_ready(const uint8_t *a2_data, sfp_a2h_t *a2) {
    if (!a2_data || !a2) return;

    // O bit Data_Not_Ready (bit 0) indica se o módulo tem dados válidos.
    // 0 = Pronto, 1 = Não pronto
    if ((a2_data[STATUS_CONTROL] & (1 << SFP_A2_BIT_DATA_NOT_READY)) == 0) {
        a2->data_ready = true;
    } else {
        a2->data_ready = false;
    }
}

bool sfp_a2h_get_data_ready(const sfp_a2h_t *a2){
  if (!a2){
    return false;
  }
  return a2->data_ready;
}
