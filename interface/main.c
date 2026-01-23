/**
 * @file main.c
 * @brief Aplicação de leitura de módulos SFP via I2C para Raspberry Pi (Linux)
 *
 * Demonstra a leitura da EEPROM A0h de módulos SFP/SFP+ conforme SFF-8472.
 * Adaptado para Linux embarcado usando a interface I2C do kernel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "sfp_8472_a0h.h"
#include "sfp_8472_a2h.h"
#include "i2c.h"

/* Barramento I2C no Raspberry Pi */
#define I2C_DEVICE "/dev/i2c-1"

/* Função auxiliar para converter código de conformidade estendida em string */
static const char* ext_compliance_to_string(sfp_extended_spec_compliance_code_t code) {
    switch (code) {
        case EXT_SPEC_COMPLIANCE_UNSPECIFIED:
            return "Não especificado";
        case EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_5E_5:
            return "100G AOC ou 25GAUI C2M AOC (BER 5e-5)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR4_OR_25GBASE_SR:
            return "100GBASE-SR4 ou 25GBASE-SR";
        case EXT_SPEC_COMPLIANCE_100GBASE_LR4_OR_25GBASE_LR:
            return "100GBASE-LR4 ou 25GBASE-LR";
        case EXT_SPEC_COMPLIANCE_100GBASE_ER4_OR_25GBASE_ER:
            return "100GBASE-ER4 ou 25GBASE-ER";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR10:
            return "100GBASE-SR10";
        case EXT_SPEC_COMPLIANCE_100G_CWDM4:
            return "100G CWDM4";
        case EXT_SPEC_COMPLIANCE_100G_PSM4:
            return "100G PSM4";
        case EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_5E_5:
            return "100G ACC ou 25GAUI C2M ACC (BER 5e-5)";
        case EXT_SPEC_COMPLIANCE_100GBASE_CR4_OR_25GBASE_CR_CA_25G_L_OR_50GBASE_CR2_RS_FEC:
            return "100GBASE-CR4, 25GBASE-CR CA-25G-L ou 50GBASE-CR2 RS-FEC";
        case EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_S_OR_50GBASE_CR2_BASE_R_FEC:
            return "25GBASE-CR CA-25G-S ou 50GBASE-CR2 BASE-R FEC";
        case EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_N_OR_50GBASE_CR2_NO_FEC:
            return "25GBASE-CR CA-25G-N ou 50GBASE-CR2 NO-FEC";
        case EXT_SPEC_COMPLIANCE_10MB_SINGLE_PAIR_ETHERNET:
            return "10Mb Single Pair Ethernet";
        case EXT_SPEC_COMPLIANCE_40GBASE_ER4:
            return "40GBASE-ER4";
        case EXT_SPEC_COMPLIANCE_4X_10GBASE_SR:
            return "4x10GBASE-SR";
        case EXT_SPEC_COMPLIANCE_40G_PSM4:
            return "40G PSM4";
        case EXT_SPEC_COMPLIANCE_10GBASE_T_SFI:
            return "10GBASE-T SFI";
        case EXT_SPEC_COMPLIANCE_100G_CLR4:
            return "100G CLR4";
        case EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_1E_12:
            return "100G AOC ou 25GAUI C2M AOC (BER 1e-12)";
        case EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_1E_12:
            return "100G ACC ou 25GAUI C2M ACC (BER 1e-12)";
        case EXT_SPEC_COMPLIANCE_10GBASE_T_SHORT_REACH:
            return "10GBASE-T Short Reach";
        case EXT_SPEC_COMPLIANCE_5GBASE_T:
            return "5GBASE-T";
        case EXT_SPEC_COMPLIANCE_2_5GBASE_T:
            return "2.5GBASE-T";
        case EXT_SPEC_COMPLIANCE_40G_SWDM4:
            return "40G SWDM4";
        case EXT_SPEC_COMPLIANCE_100G_SWDM4:
            return "100G SWDM4";
        case EXT_SPEC_COMPLIANCE_100G_PAM4_BIDI:
            return "100G PAM4 BiDi";
        case EXT_SPEC_COMPLIANCE_100GBASE_DR_CAUI4_NO_FEC:
            return "100GBASE-DR (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100G_FR_OR_100GBASE_FR1_CAUI4_NO_FEC:
            return "100G-FR/100GBASE-FR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100G_LR_OR_100GBASE_LR1_CAUI4_NO_FEC:
            return "100G-LR/100GBASE-LR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR1_CAUI4_NO_FEC:
            return "100GBASE-SR1 (CAUI-4 NO FEC)";
        case EXT_SPEC_COMPLIANCE_100GBASE_SR1_OR_200GBASE_SR2_OR_400GBASE_SR4:
            return "100GBASE-SR1, 200GBASE-SR2 ou 400GBASE-SR4";
        case EXT_SPEC_COMPLIANCE_100GBASE_FR1_OR_400GBASE_DR4_2:
            return "100GBASE-FR1 ou 400GBASE-DR4";
        case EXT_SPEC_COMPLIANCE_100GBASE_LR1:
            return "100GBASE-LR1";
        case EXT_SPEC_COMPLIANCE_ACTIVE_CU_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6:
            return "Active Copper Cable (50GAUI/100GAUI-2/200GAUI-4 C2M BER 1e-6)";
        case EXT_SPEC_COMPLIANCE_ACTIVE_OPTICAL_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6:
            return "Active Optical Cable (50GAUI/100GAUI-2/200GAUI-4 C2M BER 1e-6)";
        case EXT_SPEC_COMPLIANCE_100GBASE_CR1_OR_200GBASE_CR2_OR_400GBASE_CR4:
            return "100GBASE-CR1, 200GBASE-CR2 ou 400GBASE-CR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_CR_OR_100GBASE_CR2_OR_200GBASE_CR4:
            return "50GBASE-CR, 100GBASE-CR2 ou 200GBASE-CR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_R_OR_100GBASE_SR2_OR_200GBASE_SR4:
            return "50GBASE-R, 100GBASE-SR2 ou 200GBASE-SR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_FR_OR_200GBASE_DR4:
            return "50GBASE-FR ou 200GBASE-DR4";
        case EXT_SPEC_COMPLIANCE_200GBASE_FR4:
            return "200GBASE-FR4";
        case EXT_SPEC_COMPLIANCE_50GBASE_LR:
            return "50GBASE-LR";
        case EXT_SPEC_COMPLIANCE_200GBASE_LR4:
            return "200GBASE-LR4";
        case EXT_SPEC_COMPLIANCE_400GBASE_DR4_400GAUI4_C2M:
            return "400GBASE-DR4 (400GAUI-4 C2M)";
        case EXT_SPEC_COMPLIANCE_400GBASE_FR4:
            return "400GBASE-FR4";
        case EXT_SPEC_COMPLIANCE_400GBASE_LR4_6:
            return "400GBASE-LR4-6";
        case EXT_SPEC_COMPLIANCE_400G_LR4_10:
            return "400G LR4-10";
        case EXT_SPEC_COMPLIANCE_256GFC_SW4:
            return "256GFC SW4";
        case EXT_SPEC_COMPLIANCE_64GFC:
            return "64GFC";
        case EXT_SPEC_COMPLIANCE_128GFC:
            return "128GFC";
        case EXT_SPEC_COMPLIANCE_VENDOR_SPECIFIC:
            return "Específico do fabricante";
        default:
            if (code >= 0x4D && code <= 0x7E) {
                return "Reservado (SFF-8024)";
            } else if (code >= 0x82 && code <= 0xFE) {
                return "Reservado (SFF-8024)";
            }
            return "Código desconhecido";
    }
}

int main(int argc, char *argv[])
{
    const char *i2c_device = I2C_DEVICE;

    /* Permite especificar o dispositivo I2C via argumento */
    if (argc > 1) {
        i2c_device = argv[1];
    }

    printf("Dispositivo I2C: %s\n\n", i2c_device);

    /* Inicializa I2C */
    int i2c_fd = sfp_i2c_init(i2c_device);
    if (i2c_fd < 0) {
        fprintf(stderr, "ERRO: Não foi possível abrir o dispositivo I2C %s\n", i2c_device);
        fprintf(stderr, "Verifique se:\n");
        fprintf(stderr, "  1. O I2C está habilitado (sudo raspi-config)\n");
        fprintf(stderr, "  2. Você tem permissão (execute como root ou adicione usuário ao grupo i2c)\n");
        fprintf(stderr, "  3. O módulo SFP está conectado corretamente\n");
        return EXIT_FAILURE;
    }

    /* Buffer cru(raw) da EEPROM A0h */
    uint8_t a0_base_data[SFP_A0_BASE_SIZE] = {0};

    printf("Lendo EEPROM A0h...\n");

    bool ok = sfp_read_block(
        i2c_fd,
        SFP_I2C_ADDR_A0,
        0x00,
        a0_base_data,
        SFP_A0_BASE_SIZE
    );

    if (!ok) {
        fprintf(stderr, "ERRO: Falha na leitura do A0h\n");
        close(i2c_fd);
        return EXIT_FAILURE;
    }

    printf("Leitura A0h OK\n");

    /* Estrutura interpretada */
    sfp_a0h_base_t a0 = {0};

    /* Parsing do bloco */
    sfp_parse_a0_base_identifier(a0_base_data, &a0);
    sfp_parse_a0_base_ext_identifier(a0_base_data, &a0); // <-- RF-02
    sfp_parse_a0_base_connector(a0_base_data, &a0);
    sfp_parse_a0_base_om1(a0_base_data, &a0);
    sfp_parse_a0_base_om2(a0_base_data, &a0);
    sfp_parse_a0_base_smf(a0_base_data, &a0);
    sfp_parse_a0_base_om4_or_copper(a0_base_data, &a0);
    sfp_parse_a0_base_ext_compliance(a0_base_data, &a0);
    sfp_parse_a0_base_encoding(a0_base_data, &a0); /* Byte 11 */
    sfp_parse_a0_base_cc_base(a0_base_data, &a0);  /* Byte 63 */

    /* =====================================================
     * Teste do Byte 0 — Identifier
     * ===================================================== */
    sfp_identifier_t id = sfp_a0_get_identifier(&a0);

    printf("\nByte 0 — Identifier: 0x%02X\n", id);

    switch (id) {
        case SFP_ID_GBIC:
            printf("Módulo GBIC identificado\n");
            break;
        case SFP_ID_SFP:
            printf("Módulo SFP/SFP+ identificado corretamente\n");
            break;
        case SFP_ID_QSFP:
            printf("Módulo QSFP identificado\n");
            break;
        case SFP_ID_QSFP_PLUS:
            printf("Módulo QSFP+ identificado\n");
            break;
        case SFP_I_QSFP28:
            printf("Módulo QSFP28 identificado\n");
            break;
        case SFP_ID_UNKNOWN:
        default:
            printf("Módulo não suportado ou identificador desconhecido\n");
            break;
    }

    /* =============================================================
     * Teste do Byte 1 — Extended Identifier
     * ============================================================= */
    printf("\nByte 1 — Extended Identifier\n");
    if (id == SFP_ID_SFP) {
        uint8_t ext = sfp_a0_get_ext_identifier(&a0);

        if (!sfp_validate_ext_identifier(&a0)) {
            printf("Modulo nao conforme (esperado 0x%02X)\n", SFP_EXT_IDENTIFIER_EXPECTED);
        } else {
            printf("Modulo conforme (0x%02X)\n", SFP_EXT_IDENTIFIER_EXPECTED);
        }
    } else {
        printf("\nByte 0 precisa ser 0x03 / SFP).\n");
    }

    /* ===================================
        Teste do Byte 2 - Leitura do Conector
     * =================================== */
    printf("\nByte 2 - Leitura do Conector\n");
    printf("Connector: %s (0x%02X)\n",sfp_connector_to_string(a0.connector),a0_base_data[2]);

    /* =============================================================
     * Teste dos Bytes 3-10 — Códigos de Conformidade do Transceptor
     * ============================================================= */
    sfp_compliance_codes_t cc;
    sfp_compliance_decoded_t comp;

    sfp_read_compliance(a0_base_data, &cc);
    sfp_decode_compliance(&cc, &comp);

    sfp_print_compliance(&comp);

    /* =====================================================
     * Teste do Byte 11 — Encoding
     * ===================================================== */
    sfp_encoding_codes_t encoding_code = sfp_a0_get_encoding(&a0);
    sfp_print_encoding(encoding_code);

    /* =====================================================
     * Teste do Byte 14 — Length SMF or Copper Attenuation
     * ===================================================== */
    sfp_smf_length_status_t smf_status;
    uint16_t smf_length_m = sfp_a0_get_smf_length_m(&a0, &smf_status);

    printf("\nByte 14 — Length SMF or Copper Attenuation\n");

    switch (smf_status) {
    case SFP_SMF_LEN_VALID:
        printf("Alcance SMF válido: %u km (ou atenuação: %u * 0.5 dB/100m)\n", smf_length_m, smf_length_m);
        break;
    case SFP_SMF_LEN_EXTENDED:
        printf("Alcance SMF superior a %u km (ou atenuação > 127 dB/100m)\n", smf_length_m);
        break;
    case SFP_SMF_LEN_NOT_SUPPORTED:
    default:
        printf("Alcance SMF ou atenuação de cobre não especificado\n");
        break;
    }

    /* =====================================================
     * Teste do Byte 16 — Length OM2 (50 µm)
     * ===================================================== */
    sfp_om2_length_status_t om2_status;
    uint16_t om2_length_m = sfp_a0_get_om2_length_m(&a0, &om2_status);

    printf("\nByte 16 — Length OM2 (50 µm)\n");

    switch (om2_status) {
    case SFP_OM2_LEN_VALID:
        printf("Alcance OM2 válido: %u metros\n", om2_length_m);
        break;
    case SFP_OM2_LEN_EXTENDED:
        printf("Alcance OM2 superior a %u metros (>2.54 km)\n", om2_length_m);
        break;
    case SFP_OM2_LEN_NOT_SUPPORTED:
    default:
        printf("Alcance OM2 não especificado ou não suportado\n");
        break;
    }

    /* =====================================================
     * Teste do Byte 17 — Length OM1 (62.5 µm)
     * ===================================================== */
    sfp_om1_length_status_t om1_status;
    uint16_t om1_length_m = sfp_a0_get_om1_length_m(&a0, &om1_status);

    printf("\nByte 17 — Length OM1 (62.5 µm)\n");

    switch (om1_status) {
    case SFP_OM1_LEN_VALID:
        printf("Alcance OM1 válido: %u metros\n", om1_length_m);
        break;
    case SFP_OM1_LEN_EXTENDED:
        printf("Alcance OM1 superior a %u metros (>2.54 km)\n", om1_length_m);
        break;
    case SFP_OM1_LEN_NOT_SUPPORTED:
    default:
        printf("Alcance OM1 não especificado ou não suportado\n");
        break;
    }

    /* =====================================================
     * Teste do Byte 18 — Length OM4 or copper cable
     * ===================================================== */
    sfp_om4_length_status_t om4_status;
    uint16_t om4_length_m = sfp_a0_get_om4_copper_or_length_m(&a0, &om4_status);

    printf("\nByte 18 — Length OM4 or Copper Cable\n");

    switch (om4_status) {
    case SFP_OM4_LEN_VALID:
        printf("Comprimento válido: %u metros\n", om4_length_m);
        break;
    case SFP_OM4_LEN_EXTENDED:
        printf("Comprimento superior a %u metros\n", om4_length_m);
        break;
    case SFP_OM4_LEN_NOT_SUPPORTED:
    default:
        printf("Comprimento não especificado\n");
        break;
    }

    /* =====================================================
     * Teste do Byte 36 — Extended Specification Compliance Codes
     * ===================================================== */
    sfp_extended_spec_compliance_code_t ext_comp = sfp_a0_get_ext_compliance(&a0);

    printf("\nByte 36 — Extended Specification Compliance Code: 0x%02X\n", ext_comp);
    printf("Descrição: %s\n", ext_compliance_to_string(ext_comp));

    /* Mostrar o valor bruto do byte 36*/
    printf("Valor bruto (Byte 36): 0x%02X\n", a0_base_data[36]);

    /* =====================================================
     * Teste do Byte 63 — CC_BASE (Checksum)
     * ===================================================== */
    bool cc_base_valid = sfp_a0_get_cc_base_is_valid(&a0);

    printf("\nByte 63 — CC_BASE (Checksum):\n");
    printf("Valor: 0x%02X\n", a0_base_data[63]);

    if (cc_base_valid) {
        printf("Status: ✓ Checksum VÁLIDO\n");
    } else {
        printf("Status: ✗ Checksum INVÁLIDO\n");
    }

#ifdef DEBUG
#endif

    /* =====================================================
     * Leitura da página A2h (Diagnósticos)
     * ===================================================== */
    printf("\n=== Leitura da página A2h (Diagnósticos) ===\n");
    
    uint8_t a2_diag_data[SFP_A2_DIAG_SIZE] = {0};
    
    bool ok_a2 = sfp_read_block(
        i2c_fd,
        SFP_I2C_ADDR_A2,
        SFP_A2_DIAG_OFFSET,
        a2_diag_data,
        SFP_A2_DIAG_SIZE
    );

    if (!ok_a2) {
        fprintf(stderr, "AVISO: Falha na leitura do A2h (diagnósticos podem não estar disponíveis)\n");
    } else {
        printf("Leitura A2h OK\n");
        
        sfp_a2h_diagnostics_t diag = {0};
        sfp_parse_a2h_diagnostics(a2_diag_data, &diag);
        
        sfp_print_a2h_diagnostics(&diag);
    }

  /* Dump (opcional) para inspeção manual */
    printf("\nDump EEPROM A0h:");
    for (int i = 0; i < SFP_A0_BASE_SIZE; i++) {
        if (i % 16 == 0)
            printf("\n%02X: ", i);
        printf("%02X ", a0_base_data[i]);
    }
    printf("\n");
  
    printf("===================================================");

    printf("\nDump EEPROM A2h:");
    for (int i = 0; i < SFP_A2_DIAG_SIZE; i++) {
        if (i % 16 == 0)
            printf("\n%02X: ", i);
        printf("%02X ", a2_diag_data[i]);
    }
    printf("\n");

    printf("\nTeste concluído.\n");

    /* Fecha o descritor I2C */
    sfp_i2c_close(i2c_fd);

    return EXIT_SUCCESS;
}
