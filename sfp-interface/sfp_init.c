/**
 * @file sfp_init.c
 * @brief Implementação das funções de inicialização e leitura de módulos SFP
 */

#include "sfp_init.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

bool sfp_init(sfp_module_t *module, const char *i2c_device)
{
    if (!module || !i2c_device) {
        return false;
    }

    /* Limpa a estrutura */
    memset(module, 0, sizeof(sfp_module_t));
    module->i2c_fd = -1;

    /* Inicializa I2C */
    module->i2c_fd = sfp_i2c_init(i2c_device);
    if (module->i2c_fd < 0) {
        fprintf(stderr, "ERRO: Não foi possível abrir o dispositivo I2C %s\n", i2c_device);
        fprintf(stderr, "Verifique se:\n");
        fprintf(stderr, "  1. O I2C está habilitado (sudo raspi-config)\n");
        fprintf(stderr, "  2. Você tem permissão (execute como root ou adicione usuário ao grupo i2c)\n");
        fprintf(stderr, "  3. O módulo SFP está conectado corretamente\n");
        return false;
    }

    /* Lê EEPROM A0h */
    printf("Lendo EEPROM A0h...\n");
    module->a0_valid = sfp_read_block(
        module->i2c_fd,
        SFP_I2C_ADDR_A0,
        0x00,
        module->a0_raw,
        SFP_A0_SIZE
    );

    if (!module->a0_valid) {
        fprintf(stderr, "ERRO: Falha na leitura do A0h\n");
        sfp_cleanup(module);
        return false;
    }
    printf("Leitura A0h OK\n");

    /* Parsing do bloco A0h */
    sfp_parse_a0_base_identifier(module->a0_raw, &module->a0);
    sfp_parse_a0_base_ext_identifier(module->a0_raw, &module->a0);
    sfp_parse_a0_base_connector(module->a0_raw, &module->a0);
    sfp_parse_a0_base_compliance(module->a0_raw, &module->a0.cc);
    sfp_parse_a0_base_encoding(module->a0_raw, &module->a0);
    sfp_parse_a0_base_smf_km(module->a0_raw, &module->a0);
    sfp_parse_a0_base_smf_m(module->a0_raw, &module->a0);
    sfp_parse_a0_base_om2(module->a0_raw, &module->a0);
    sfp_parse_a0_base_om1(module->a0_raw, &module->a0);
    sfp_parse_a0_base_om4_or_copper(module->a0_raw, &module->a0);
    sfp_parse_a0_base_om3_or_cable(module->a0_raw, &module->a0);
    sfp_parse_a0_base_vendor_name(module->a0_raw, &module->a0);
    sfp_parse_a0_base_ext_compliance(module->a0_raw, &module->a0);
    sfp_parse_a0_base_vendor_oui(module->a0_raw, &module->a0);
    sfp_parse_a0_base_vendor_rev(module->a0_raw, &module->a0);
    sfp_parse_a0_fc_speed_2(module->a0_raw, &module->a0);
    sfp_parse_a0_base_cc_base(module->a0_raw, &module->a0);

    /* Decodifica compliance codes */
    sfp_a0_decode_compliance(&module->a0.cc, &module->a0.dc);

    /* Lê EEPROM A2h (diagnósticos) */
    printf("Lendo EEPROM A2h...\n");
    module->a2_valid = sfp_read_block(
        module->i2c_fd,
        SFP_I2C_ADDR_A2,
        0x00,
        module->a2_raw,
        SFP_A2_SIZE
    );

    if (!module->a2_valid) {
        fprintf(stderr, "AVISO: Falha na leitura do A2h (diagnósticos podem não estar disponíveis)\n");
    } else {
        printf("Leitura A2h OK\n");
        float vcc;
        if (get_sfp_vcc(module->a2_raw, &vcc)) {
            module->a2.vcc_realtime = vcc;
        }
        sfp_parse_a2h_data_ready(module->a2_raw, &module->a2);
    }

    return true;
}

void sfp_cleanup(sfp_module_t *module)
{
    if (!module) {
        return;
    }

    if (module->i2c_fd >= 0) {
        sfp_i2c_close(module->i2c_fd);
        module->i2c_fd = -1;
    }
}

void sfp_info(const sfp_module_t *module)
{
    if (!module || !module->a0_valid) {
        printf("Dados do módulo inválidos\n");
        return;
    }

    const sfp_a0h_base_t *a0 = &module->a0;

    /* Byte 0 — Identifier */
    uint8_t identifier = sfp_a0_get_identifier(a0);
    printf("\n=== Informações do Módulo SFP ===\n");
    printf("\nByte 0 — Identifier: 0x%02X\n", identifier);
    switch (identifier) {
        case SFP_ID_GBIC:
            printf("Tipo: GBIC\n");
            break;
        case SFP_ID_SFP:
            printf("Tipo: SFP/SFP+\n");
            break;
        case SFP_ID_QSFP:
            printf("Tipo: QSFP\n");
            break;
        case SFP_ID_QSFP_PLUS:
            printf("Tipo: QSFP+\n");
            break;
        case SFP_ID_QSFP28:
            printf("Tipo: QSFP28\n");
            break;
        default:
            printf("Tipo: Desconhecido ou não suportado\n");
            break;
    }

    /* Byte 1 — Extended Identifier */
    uint8_t ext_identifier = sfp_a0_get_ext_identifier(a0);
    bool ext_id_valid = sfp_validate_ext_identifier(a0);
    printf("\nByte 1 — Extended Identifier\n");
    if (identifier == SFP_ID_SFP) {
        printf("Status: %s (0x%02X)\n", ext_id_valid ? "Conforme" : "Não conforme", ext_identifier);
    }

    /* Byte 2 — Connector */
    uint8_t connector_raw = a0->connector;
    const char *connector_str = sfp_connector_to_string(a0->connector);
    printf("\nByte 2 — Connector\n");
    printf("Tipo: %s (0x%02X)\n", connector_str, connector_raw);

    /* Bytes 3–10 — Compliance Codes */
    printf("\nBytes 3-10 — Códigos de Conformidade\n");
    sfp_a0_print_compliance(&a0->dc);

    /* Byte 11 — Encoding */
    uint8_t encoding = sfp_a0_get_encoding(a0);
    printf("\nByte 11 — Encoding\n");
    sfp_print_encoding(encoding);

    /* Byte 14 — Length SMF */
    sfp_smf_length_status_t smf_status_km;
    uint16_t smf_len_km = sfp_a0_get_smf_length_km(a0, &smf_status_km);
    float smf_attenuation_km = smf_len_km * 0.5f;
    printf("\nByte 14 — Length SMF or Copper Attenuation\n");
    switch (smf_status_km) {
        case SFP_SMF_LEN_VALID:
            printf("Alcance SMF: %u km (atenuação: %.1f dB/100m)\n", smf_len_km, smf_attenuation_km);
            break;
        case SFP_SMF_LEN_EXTENDED:
            printf("Alcance SMF: >%u km (atenuação > %.1f dB/100m)\n", smf_len_km, smf_attenuation_km);
            break;
        default:
            printf("Não especificado\n");
            break;
    }

    /* Byte 15 — Length SMF (units 100m) */
    sfp_smf_length_status_t smf_status_m;
    uint16_t smf_len_m = sfp_a0_get_smf_length_m(a0, &smf_status_m);
    float smf_attenuation_m = smf_len_m * 0.5f;
    printf("\nByte 15 — Length SMF or Copper Attenuation (Units 100m)\n");
    switch (smf_status_m) {
        case SFP_SMF_LEN_VALID:
            printf("Alcance SMF: %u km (atenuação: %.1f dB/100m)\n", smf_len_m, smf_attenuation_m);
            break;
        case SFP_SMF_LEN_EXTENDED:
            printf("Alcance SMF: >%u km (atenuação > %.1f dB/100m)\n", smf_len_m, smf_attenuation_m);
            break;
        default:
            printf("Não especificado\n");
            break;
    }

    /* Byte 16 — OM2 */
    sfp_om2_length_status_t om2_status;
    uint16_t om2_len = sfp_a0_get_om2_length_m(a0, &om2_status);
    printf("\nByte 16 — Length OM2 (50 µm)\n");
    switch (om2_status) {
        case SFP_OM2_LEN_VALID:
            printf("Alcance: %u metros\n", om2_len);
            break;
        case SFP_OM2_LEN_EXTENDED:
            printf("Alcance: >%u metros\n", om2_len);
            break;
        default:
            printf("Não especificado\n");
            break;
    }

    /* Byte 17 — OM1 */
    sfp_om1_length_status_t om1_status;
    uint16_t om1_len = sfp_a0_get_om1_length_m(a0, &om1_status);
    printf("\nByte 17 — Length OM1 (62.5 µm)\n");
    switch (om1_status) {
        case SFP_OM1_LEN_VALID:
            printf("Alcance: %u metros\n", om1_len);
            break;
        case SFP_OM1_LEN_EXTENDED:
            printf("Alcance: >%u metros\n", om1_len);
            break;
        default:
            printf("Não especificado\n");
            break;
    }

    /* Byte 18 — OM4 / Copper */
    sfp_om4_length_status_t om4_status;
    uint16_t om4_copper_len = sfp_a0_get_om4_copper_or_length_m(a0, &om4_status);
    printf("\nByte 18 — Length OM4 or Copper Cable\n");
    switch (om4_status) {
        case SFP_OM4_LEN_VALID:
            printf("Comprimento: %u metros\n", om4_copper_len);
            break;
        case SFP_OM4_LEN_EXTENDED:
            printf("Comprimento: >%u metros\n", om4_copper_len);
            break;
        default:
            printf("Não especificado\n");
            break;
    }

    /* Byte 19 — OM3 / Cable */
    sfp_om3_length_status_t om3_status;
    uint32_t om3_cable_len = sfp_a0_get_om3_cable_length_m(a0, &om3_status);
    printf("\nByte 19 — Length OM3 or Physical Interconnect Length\n");
    switch (om3_status) {
        case SFP_OM3_LEN_VALID:
            printf("Comprimento: %u metros\n", om3_cable_len);
            break;
        case SFP_OM3_LEN_EXTENDED:
            printf("Comprimento: >%u metros\n", om3_cable_len);
            break;
        default:
            printf("Não especificado\n");
            break;
    }

    /* Bytes 20–35 — Vendor Name */
    char vendor_name[SFP_A0_LEN_VENDOR_NAME + 1] = {0};
    bool vendor_name_valid = sfp_a0_get_vendor_name(a0, vendor_name);
    printf("\nBytes 20–35 — Vendor Name\n");
    if (vendor_name_valid && vendor_name[0] != '\0') {
        printf("Fabricante: \"%s\"\n", vendor_name);
    } else {
        printf("Não especificado\n");
    }

    /* Byte 36 — Extended Compliance */
    uint8_t ext_compliance = sfp_a0_get_ext_compliance(a0);
    const char *ext_compliance_desc = ext_compliance_to_string(ext_compliance);
    printf("\nByte 36 — Extended Specification Compliance\n");
    printf("Código: 0x%02X\n", ext_compliance);
    printf("Descrição: %s\n", ext_compliance_desc);

    /* Bytes 37–39 — Vendor OUI */
    uint8_t vendor_oui_raw[3] = {0};
    bool vendor_oui_valid = sfp_a0_get_vendor_oui(a0, vendor_oui_raw);
    uint32_t vendor_oui_u32 = sfp_vendor_oui_to_u32(a0);
    printf("\nBytes 37–39 — Vendor OUI\n");
    if (vendor_oui_valid) {
        printf("OUI: %02X-%02X-%02X (0x%06X)\n", vendor_oui_raw[0], vendor_oui_raw[1], vendor_oui_raw[2], vendor_oui_u32);
    } else {
        printf("Não especificado\n");
    }

    /* Bytes 56–59 — Vendor REV */
    char vendor_rev_raw[5];
    bool vendor_rev_valid = sfp_a0_get_vendor_rev(a0, vendor_rev_raw);

    printf("\nBytes 56-59 — Vendor REV\n");
    if (vendor_rev_valid) {
        printf("REV: %s\n", vendor_rev_raw);
    } else {
        printf("Não especificado\n");
    }
    /* Byte 62 — Fibre Channel Speed 2 */
    bool fc_speed_2_valid = sfp_get_a0_fc_speed_2(a0, &a0->dc);
    printf("\nByte 62 — Fibre Channel Speed 2\n");
    if (fc_speed_2_valid) {
        printf("FC Speed 2: Presente\n");
    } else {
        printf("Não especificado ou não aplicável\n");
    }

    /* Byte 63 — CC_BASE */
    bool cc_base_valid = sfp_a0_get_cc_base_is_valid(a0);
    printf("\nByte 63 — CC_BASE (Checksum)\n");
    printf("Status: %s\n", cc_base_valid ? "VÁLIDO" : "INVÁLIDO");

    /* Diagnósticos A2h */
    if (module->a2_valid) {
        printf("\n=== Diagnósticos (A2h) ===\n");
        printf("Data Ready: %s\n", module->a2.data_ready ? "SIM" : "NÃO");
        printf("VCC:         %.3f V\n", module->a2.vcc_realtime);
    }
}

void sfp_dump(const sfp_module_t *module)
{
    if (!module) {
        return;
    }

    if (module->a0_valid) {
        printf("\n=== Dump EEPROM A0h ===");
        for (int i = 0; i < SFP_A0_SIZE; i++) {
            if (i % 16 == 0)
                printf("\n%02X: ", i);
            printf("%02X ", module->a0_raw[i]);
        }
        printf("\n");
    }

    if (module->a2_valid) {
        printf("\n=== Dump EEPROM A2h ===");
        for (int i = 0; i < SFP_A2_SIZE; i++) {
            if (i % 16 == 0)
                printf("\n%02X: ", i);
            printf("%02X ", module->a2_raw[i]);
        }
        printf("\n");
    }
}
