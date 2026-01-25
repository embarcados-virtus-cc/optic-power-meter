#include "sfp_8472_a0h.h"
#include <stdio.h>
#include <string.h>

/* ============================================
 * Byte 0 — Identifier
 * ============================================ */
void sfp_parse_a0_base_identifier(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    a0->identifier = (sfp_identifier_t)a0_base_data[0];
}

/* ============================================
 * Função Getter
 * ============================================ */
sfp_identifier_t sfp_a0_get_identifier(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return SFP_ID_UNKNOWN;

    return a0->identifier;
}

/* ============================================
 * Byte 1 — Extended Identifier
 * ============================================ */
void sfp_parse_a0_base_ext_identifier(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    a0->ext_identifier = a0_base_data[1];
}

/* ============================================
 * Função Getter
 * ============================================ */
uint8_t sfp_a0_get_ext_identifier(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return 0x00;

    return a0->ext_identifier;
}

bool sfp_validate_ext_identifier(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return false;

    return (a0->ext_identifier == SFP_EXT_IDENTIFIER_EXPECTED);
}

/* ============================================
 * Byte 2 — Connector
 * ============================================ */
void sfp_parse_a0_base_connector(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    uint8_t connector_raw = a0_base_data[2];
    a0->connector = (sfp_connector_type_t)connector_raw;
}

/* ============================================
 * Função Getter
 * ============================================ */
sfp_connector_type_t sfp_a0_get_connector(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return SFP_CONNECTOR_UNKNOWN;

    return a0->connector;
}

/* ============================================
 * Função de Exposição
 * ============================================ */
const char *sfp_connector_to_string(sfp_connector_type_t connector)
{
    switch (connector) {
        case SFP_CONNECTOR_SC:
            return "SC";
        case SFP_CONNECTOR_FC_STYLE_1:
            return "Fibre Channel Style 1";
        case SFP_CONNECTOR_FC_STYLE_2:
            return "Fibre Channel Style 2";
        case SFP_CONNECTOR_BNC_TNC:
            return "BNC/TNC";
        case SFP_CONNECTOR_FC_COAX:
            return "Fibre Channel Coax";
        case SFP_CONNECTOR_FIBER_JACK:
            return "Fiber Jack";
        case SFP_CONNECTOR_LC:
            return "LC";
        case SFP_CONNECTOR_MT_RJ:
            return "MT-RJ";
        case SFP_CONNECTOR_MU:
            return "MU";
        case SFP_CONNECTOR_SG:
            return "SG";
        case SFP_CONNECTOR_OPTICAL_PIGTAIL:
            return "Optical Pigtail";
        case SFP_CONNECTOR_MPO_1X12:
            return "MPO 1x12";
        case SFP_CONNECTOR_MPO_2X16:
            return "MPO 2x16";
        case SFP_CONNECTOR_HSSDC_II:
            return "HSSDC II";
        case SFP_CONNECTOR_COPPER_PIGTAIL:
            return "Copper Pigtail";
        case SFP_CONNECTOR_RJ45:
            return "RJ45";
        case SFP_CONNECTOR_NO_SEPARABLE:
            return "No Separable Connector";
        default:
            return "Unknown Connector";
    }
}

/* ============================================
 * Bytes 3-10 — Compliance Codes
 * ============================================ */
void sfp_read_compliance(const uint8_t *a0_base_data, sfp_compliance_codes_t *cc)
{
    if (!a0_base_data || !cc)
        return;

    cc->byte3  = a0_base_data[3];
    cc->byte4  = a0_base_data[4];
    cc->byte5  = a0_base_data[5];
    cc->byte6  = a0_base_data[6];
    cc->byte7  = a0_base_data[7];
    cc->byte8  = a0_base_data[8];
    cc->byte9  = a0_base_data[9];
    cc->byte10 = a0_base_data[10];
}

/* ============================================
 * Decode do Byte 3
 * ============================================ */
static void decode_byte3(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte3;
    out->eth_10g_base_er               = (b & (1 << 7)) != 0;
    out->eth_10g_base_lrm              = (b & (1 << 6)) != 0;
    out->eth_10g_base_lr               = (b & (1 << 5)) != 0;
    out->eth_10g_base_sr               = (b & (1 << 4)) != 0;
    out->infiniband_1x_sx              = (b & (1 << 3)) != 0;
    out->infiniband_1x_lx              = (b & (1 << 2)) != 0;
    out->infiniband_1x_copper_active   = (b & (1 << 1)) != 0;
    out->infiniband_1x_copper_passive  = (b & (1 << 0)) != 0;
}

/* ============================================
 * Decode do Byte 4
 * ============================================ */
static void decode_byte4(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte4;
    out->escon_mmf  = (b & (1 << 7)) != 0;
    out->escon_smf  = (b & (1 << 6)) != 0;
    out->oc_192_sr  = (b & (1 << 5)) != 0;
    out->sonet_rs_1 = (b & (1 << 4)) != 0;
    out->sonet_rs_2 = (b & (1 << 3)) != 0;
    out->oc_48_lr   = (b & (1 << 2)) != 0;
    out->oc_48_ir   = (b & (1 << 1)) != 0;
    out->oc_48_sr   = (b & (1 << 0)) != 0;
}

/* ============================================
 * Decode do Byte 5
 * ============================================ */
static void decode_byte5(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte5;
    out->oc_12_sm_lr = (b & (1 << 6)) != 0;
    out->oc_12_sm_ir = (b & (1 << 5)) != 0;
    out->oc_12_sr    = (b & (1 << 4)) != 0;
    out->oc_3_sm_lr  = (b & (1 << 2)) != 0;
    out->oc_3_sm_ir  = (b & (1 << 1)) != 0;
    out->oc_3_sr     = (b & (1 << 0)) != 0;
}

/* ============================================
 * Decode do Byte 6
 * ============================================ */
static void decode_byte6(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte6;
    out->eth_base_px      = (b & (1 << 7)) != 0;
    out->eth_base_bx_10   = (b & (1 << 6)) != 0;
    out->eth_100_base_fx  = (b & (1 << 5)) != 0;
    out->eth_100_base_lx  = (b & (1 << 4)) != 0;
    out->eth_1000_base_t  = (b & (1 << 3)) != 0;
    out->eth_1000_base_cx = (b & (1 << 2)) != 0;
    out->eth_1000_base_lx = (b & (1 << 1)) != 0;
    out->eth_1000_base_sx = (b & (1 << 0)) != 0;
}

/* ============================================
 * Decode do Byte 7
 * ============================================ */
static void decode_byte7(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte7;
    out->fc_very_long_distance      = (b & (1 << 7)) != 0;
    out->fc_short_distance          = (b & (1 << 6)) != 0;
    out->fc_intermediate_distance   = (b & (1 << 5)) != 0;
    out->fc_long_distance           = (b & (1 << 4)) != 0;
    out->fc_medium_distance         = (b & (1 << 3)) != 0;
    out->shortwave_laser_sa         = (b & (1 << 2)) != 0;
    out->longwave_laser_lc          = (b & (1 << 1)) != 0;
    out->electrical_inter_enclosure = (b & (1 << 0)) != 0;
}

/* ============================================
 * Decode do Byte 8
 * ============================================ */
static void decode_byte8(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte8;
    out->electrical_intra_enclosure = (b & (1 << 7)) != 0;
    out->shortwave_laser_sn         = (b & (1 << 6)) != 0;
    out->shortwave_laser_sl        = (b & (1 << 5)) != 0;
    out->longwave_laser_ll          = (b & (1 << 4)) != 0;
    out->active_cable               = (b & (1 << 3)) != 0;
    out->passive_cable              = (b & (1 << 2)) != 0;
}

/* ============================================
 * Decode do Byte 9
 * ============================================ */
static void decode_byte9(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte9;
    out->twin_axial_pair = (b & (1 << 7)) != 0;
    out->twisted_pair    = (b & (1 << 6)) != 0;
    out->miniature_coax  = (b & (1 << 5)) != 0;
    out->video_coax      = (b & (1 << 4)) != 0;
    out->multimode_m6    = (b & (1 << 3)) != 0;
    out->multimode_m5    = (b & (1 << 2)) != 0;
    out->single_mode     = (b & (1 << 0)) != 0;
}

/* ============================================
 * Decode do Byte 10
 * ============================================ */
static void decode_byte10(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    uint8_t b = cc->byte10;
    out->cs_1200_mbps = (b & (1 << 7)) != 0;
    out->cs_800_mbps  = (b & (1 << 6)) != 0;
    out->cs_1600_mbps = (b & (1 << 5)) != 0;
    out->cs_400_mbps  = (b & (1 << 4)) != 0;
    out->cs_3200_mbps = (b & (1 << 3)) != 0;
    out->cs_200_mbps  = (b & (1 << 2)) != 0;
    out->see_byte_62  = (b & (1 << 1)) != 0;
    out->cs_100_mbps  = (b & (1 << 0)) != 0;
}

/* ============================================
 * Função principal para decodificação de todos os bytes
 * ============================================ */
void sfp_decode_compliance(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out)
{
    if (!cc || !out)
        return;

    memset(out, 0, sizeof(*out));
    decode_byte3(cc, out);
    decode_byte4(cc, out);
    decode_byte5(cc, out);
    decode_byte6(cc, out);
    decode_byte7(cc, out);
    decode_byte8(cc, out);
    decode_byte9(cc, out);
    decode_byte10(cc, out);
}

/* ============================================
 * Função de Exposição
 * ============================================ */
void sfp_print_compliance(const sfp_compliance_decoded_t *c)
{
    if (!c) return;

    printf("\n=== Transceiver Compliance Codes (Bytes 3–10) ===\n");

    /* ==============================
     * Byte 3 — Ethernet / InfiniBand
     * ============================== */
    printf("\n[Byte 3] Ethernet / InfiniBand:\n");
    if (c->eth_10g_base_er)  printf("  - 10GBASE-ER\n");
    if (c->eth_10g_base_lrm) printf("  - 10GBASE-LRM\n");
    if (c->eth_10g_base_lr)  printf("  - 10GBASE-LR\n");
    if (c->eth_10g_base_sr)  printf("  - 10GBASE-SR\n");

    if (c->infiniband_1x_sx)             printf("  - InfiniBand 1X SX\n");
    if (c->infiniband_1x_lx)             printf("  - InfiniBand 1X LX\n");
    if (c->infiniband_1x_copper_active)  printf("  - InfiniBand 1X Copper Active\n");
    if (c->infiniband_1x_copper_passive) printf("  - InfiniBand 1X Copper Passive\n");

    /* ======================
     * Byte 4 — ESCON / SONET
     * ====================== */
    printf("\n[Byte 4] ESCON / SONET:\n");
    if (c->escon_mmf)   printf("  - ESCON MMF\n");
    if (c->escon_smf)   printf("  - ESCON SMF\n");
    if (c->oc_192_sr)   printf("  - OC-192 SR\n");
    if (c->sonet_rs_1)  printf("  - SONET RS-1\n");
    if (c->sonet_rs_2)  printf("  - SONET RS-2\n");
    if (c->oc_48_lr)    printf("  - OC-48 LR\n");
    if (c->oc_48_ir)    printf("  - OC-48 IR\n");
    if (c->oc_48_sr)    printf("  - OC-48 SR\n");

    /* ======================
     * Byte 5 — SONET
     * ====================== */
    printf("\n[Byte 5] SONET:\n");
    if (c->oc_12_sm_lr) printf("  - OC-12 SM LR\n");
    if (c->oc_12_sm_ir) printf("  - OC-12 SM IR\n");
    if (c->oc_12_sr)    printf("  - OC-12 SR\n");
    if (c->oc_3_sm_lr)  printf("  - OC-3 SM LR\n");
    if (c->oc_3_sm_ir)  printf("  - OC-3 SM IR\n");
    if (c->oc_3_sr)     printf("  - OC-3 SR\n");

    /* ======================
     * Byte 6 — Ethernet
     * ====================== */
    printf("\n[Byte 6] Ethernet:\n");
    if (c->eth_base_px)      printf("  - BASE-PX\n");
    if (c->eth_base_bx_10)   printf("  - BASE-BX10\n");
    if (c->eth_100_base_fx)  printf("  - 100BASE-FX\n");
    if (c->eth_100_base_lx)  printf("  - 100BASE-LX\n");
    if (c->eth_1000_base_t)  printf("  - 1000BASE-T\n");
    if (c->eth_1000_base_cx) printf("  - 1000BASE-CX\n");
    if (c->eth_1000_base_lx) printf("  - 1000BASE-LX\n");
    if (c->eth_1000_base_sx) printf("  - 1000BASE-SX\n");

    /* ====================================
     * Byte 7 — FC Link Length & Technology
     * ==================================== */
    printf("\n[Byte 7] Fibre Channel — Link Length / Technology:\n");
    if (c->fc_very_long_distance)      printf("  - Very Long Distance\n");
    if (c->fc_short_distance)          printf("  - Short Distance\n");
    if (c->fc_intermediate_distance)   printf("  - Intermediate Distance\n");
    if (c->fc_long_distance)           printf("  - Long Distance\n");
    if (c->fc_medium_distance)         printf("  - Medium Distance\n");
    if (c->shortwave_laser_sa)          printf("  - Shortwave Laser (SA)\n");
    if (c->longwave_laser_lc)           printf("  - Longwave Laser (LC)\n");
    if (c->electrical_inter_enclosure)  printf("  - Electrical Inter-Enclosure\n");

    /* ==============================================
     * Byte 8 — FC & SFP+ Cable Technology
     * ============================================== */
    printf("\n[Byte 8] Fibre Channel / Cable Technology:\n");
    if (c->electrical_intra_enclosure) printf("  - Electrical Intra-Enclosure\n");
    if (c->shortwave_laser_sn)         printf("  - Shortwave Laser (SN)\n");
    if (c->shortwave_laser_sl)         printf("  - Shortwave Laser (SL)\n");
    if (c->longwave_laser_ll)          printf("  - Longwave Laser (LL)\n");
    if (c->active_cable)               printf("  - Active SFP+ Cable\n");
    if (c->passive_cable)              printf("  - Passive SFP+ Cable\n");

    /* ==============================
     * Byte 9 — FC Transmission Media
     * ============================== */
    printf("\n[Byte 9] Fibre Channel — Transmission Media:\n");
    if (c->twin_axial_pair) printf("  - Twin Axial Pair\n");
    if (c->twisted_pair)    printf("  - Twisted Pair\n");
    if (c->miniature_coax)  printf("  - Miniature Coax\n");
    if (c->video_coax)      printf("  - Video Coax\n");
    if (c->multimode_m6)    printf("  - Multimode 62.5 µm\n");
    if (c->multimode_m5)    printf("  - Multimode 50 µm\n");
    if (c->single_mode)     printf("  - Single Mode\n");

    /* ==============================
     * Byte 10 — FC Channel Speed
     * ============================== */
    printf("\n[Byte 10] Fibre Channel — Speed:\n");
    if (c->cs_1200_mbps) printf("  - 1200 Mbps\n");
    if (c->cs_800_mbps)  printf("  - 800 Mbps\n");
    if (c->cs_1600_mbps) printf("  - 1600 Mbps\n");
    if (c->cs_400_mbps)  printf("  - 400 Mbps\n");
    if (c->cs_3200_mbps) printf("  - 3200 Mbps\n");
    if (c->cs_200_mbps)  printf("  - 200 Mbps\n");
    if (c->cs_100_mbps)  printf("  - 100 Mbps\n");
}

/* =========================================================
 * Byte 8 — Natureza física do meio
 * =========================================================*/
static bool sfp_is_copper(uint8_t byte8)
{
    return ((byte8 & (1 << 2)) != 0) || ((byte8 & (1 << 3)) != 0);
}

/* ============================================
 * Byte 11 — Encoding
 * ============================================ */
void sfp_parse_a0_base_encoding(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;
    a0->encoding = (sfp_encoding_codes_t)a0_base_data[11];
}

/* ============================================
 * Função Getter
 * ============================================ */
sfp_encoding_codes_t sfp_a0_get_encoding(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return SFP_ENC_UNSPECIFIED;
    return a0->encoding;
}

/* ============================================
 * Função de Exposição
 * ============================================ */
void sfp_print_encoding(sfp_encoding_codes_t encoding)
{
    printf("\n[Byte 11] Encoding:\n");

    switch ((uint8_t)encoding) {
        case SFP_ENC_UNSPECIFIED:
            printf("  - Unspecified\n");
            break;
        case SFP_ENC_8B_10B:
            printf("  - 8B/10B\n");
            break;
        case SFP_ENC_4B_5B:
            printf("  - 4B/5B\n");
            break;
        case SFP_ENC_NRZ:
            printf("  - NRZ\n");
            break;
        case SFP_ENC_MANCHESTER:
            printf("  - Manchester\n");
            break;
        case SFP_ENC_SONET_SCRAMBLED:
            printf("  - SONET Scrambled\n");
            break;
        case SFP_ENC_64B_66B:
            printf("  - 64B/66B\n");
            break;
        case SFP_ENC_256B_257B:
            printf("  - 256B/257B\n");
            break;
        case SFP_ENC_PAM4:
            printf("  - PAM4\n");
            break;
        default:
            printf("  - Reserved / Unknown Code (0x%02X)\n", (uint8_t)encoding);
            break;
    }
}

/* =========================================================
 * Byte 14 — Length (SMF) or Attenuation (Copper)
 *
 * @details
 *  - Para fibra SMF: Representa o alcance em quilômetros (km)
 *    Unidade: 1 km (valor 0x01 = 1 km, 0x64 = 100 km)
 *
 *  - Para cabo de cobre: Representa a atenuação em dB/100m
 *    Unidade: 0.5 dB/100m (valor 0x01 = 0.5 dB/100m)
 *
 *  - Valores especiais:
 *    0x00: Não suportado ou informação não disponível
 *    0xFF: Valor superior ao máximo representável (> 254 km ou > 127 dB/100m)
 * =========================================================*/

void sfp_parse_a0_base_smf(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    uint8_t raw = a0_base_data[14];

    uint8_t byte8 = a0_base_data[8];

    bool is_copper = sfp_is_copper(byte8);

    /*
     * Fluxo Principal + Secundários
     */
    if (raw == 0x00) {
        /*
         * Fluxo Secundário 2 (caso 00h):
         * Não há informação explícita de alcance SMF ou atenuação de cobre.
         */
        a0->smf_status   = SFP_SMF_LEN_NOT_SUPPORTED;
        a0->smf_length_m = 0;
    }
    else if (raw == 0xFF) {
        /*
         * Fluxo Secundário (caso FFh):
         * Comprimento/Atenuação superior ao máximo nominal representável.
         *
         * - Fibra SMF: alcance > 254 km
         * - Cabo de cobre: atenuação > 127 dB/100m
         *
         * O valor armazenado representa o limite inferior/superior conhecido.
         */
        a0->smf_status   = SFP_SMF_LEN_EXTENDED;

        if (is_copper)
            a0->smf_length_m = 254; /* 254 * 0.5 = 127 dB/100m */
        else
            a0->smf_length_m = 254; /* 254 km */
    }
    else {
        /*
         * Fluxo Principal:
         * Valor válido (01h–FEh)
         *
         * - Fibra SMF: unidade de 1 km (valor direto)
         * - Cabo de cobre: unidade de 0.5 dB/100m (valor * 0.5)
         */
        a0->smf_status   = SFP_SMF_LEN_VALID;
        a0->smf_length_m = (uint16_t)raw;
    }
}

/* ============================================
 * Função Getter
 * ============================================ */
uint16_t sfp_a0_get_smf_length_m(const sfp_a0h_base_t *a0, sfp_smf_length_status_t *status)
{
    if (!a0) {
        if (status)
            *status = SFP_SMF_LEN_NOT_SUPPORTED;
        return 0;
    }
    if (status)
        *status = a0->smf_status;
    return a0->smf_length_m;
}

/* ============================================
 * Byte 16 — OM2 Length (50 µm)
 * ============================================ */
void sfp_parse_a0_base_om2(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    uint8_t raw = a0_base_data[16];

    /*
     * Fluxo Principal + Secundários
     */
    if (raw == 0x00) {
        /*
         * Fluxo Secundário 2 (caso 00h):
         * Não suportado ou deve ser determinado por outros meios.
         */
        a0->om2_status   = SFP_OM2_LEN_NOT_SUPPORTED;
        a0->om2_length_m = 0;
    }
    else if (raw == 0xFF) {
        /*
         * Fluxo Secundário 2 (caso FFh):
         * Alcance > 2.54 km.
         */
        a0->om2_status   = SFP_OM2_LEN_EXTENDED;
        a0->om2_length_m = 2540; /* Limite inferior conhecido (254 * 10) */
    }
    else {
        /*
         * Fluxo Principal:
         * Valor válido (01h–FEh)
         * Unidade: 10 metros
         */
        a0->om2_status   = SFP_OM2_LEN_VALID;
        a0->om2_length_m = (uint16_t)raw * 10;
    }
}

/* ============================================
 * Função Getter
 * ============================================ */
uint16_t sfp_a0_get_om2_length_m(const sfp_a0h_base_t *a0, sfp_om2_length_status_t *status)
{
    if (!a0) {
        if (status)
            *status = SFP_OM2_LEN_NOT_SUPPORTED;
        return 0;
    }
    if (status)
        *status = a0->om2_status;
    return a0->om2_length_m;
}

/* ============================================
 * Byte 17 — OM1 Length (62.5 µm)
 * ============================================ */
void sfp_parse_a0_base_om1(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    uint8_t raw = a0_base_data[17];

    /*
     * Fluxo Principal + Secundários
     */
    if (raw == 0x00) {
        /*
         * Fluxo Secundário 2 (caso 00h):
         * Não há informação explícita de alcance OM1.
         * Inferência futura via bytes 3–10.
         */
        a0->om1_status   = SFP_OM1_LEN_NOT_SUPPORTED;
        a0->om1_length_m = 0;
    }
    else if (raw == 0xFF) {
        /*
         * Fluxo Secundário 2 (caso FFh):
         * Alcance superior ao máximo nominal do campo.
         * Representa > 2,54 km.
         */
        a0->om1_status   = SFP_OM1_LEN_EXTENDED;
        a0->om1_length_m = 2540; /* Limite inferior conhecido */
    }
    else {
        /*
         * Fluxo Principal / Secundário 1:
         * Valor válido (01h–FEh)
         * Unidade: 10 metros
         */
        a0->om1_status   = SFP_OM1_LEN_VALID;
        a0->om1_length_m = (uint16_t)raw * 10;
    }
}

/* ============================================
 * Função Getter
 * ============================================ */
uint16_t sfp_a0_get_om1_length_m(const sfp_a0h_base_t *a0, sfp_om1_length_status_t *status)
{
    if (!a0) {
        if (status)
            *status = SFP_OM1_LEN_NOT_SUPPORTED;
        return 0;
    }
    if (status)
        *status = a0->om1_status;
    return a0->om1_length_m;
}

/* ============================================
 * Byte 18 — OM4 or Copper Cable Length
 * ============================================ */
void sfp_parse_a0_base_om4_or_copper(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    uint8_t raw_length = a0_base_data[18];

    uint8_t byte8 = a0_base_data[8];

    bool is_copper = sfp_is_copper(byte8);

    /*
     * Fluxo Principal + Secundários
     */
    if (raw_length == 0x00) {
        /*
         * Fluxo Secundário 2 (caso 00h):
         * Não há informação explícita de comprimento de OM4 ou o cobre sem informação válida.
         */
        a0->om4_or_copper_status = SFP_OM4_LEN_NOT_SUPPORTED;
        a0->om4_or_copper_length_m = 0;
    }
    else if (raw_length == 0xFF) {
        /*
         * Fluxo Secundário (caso FFh):
         * Comprimento superior ao máximo nominal representável
         * pelo campo.
         *
         * - Cabo de cobre: comprimento > 254 m
         * - OM4: comprimento > 2,54 km
         *
         * O valor armazenado representa o limite inferior conhecido.
         */
        a0->om4_or_copper_status = SFP_OM4_LEN_EXTENDED;

        if (is_copper)
            a0->om4_or_copper_length_m = 254;
        else
            a0->om4_or_copper_length_m = 2540;
    }
    else {
        /*
         * Fluxo Principal:
         * Valor válido (01h–FEh)
         *
         * - OM4: unidades de 10 metros
         * - Cabo de cobre: unidades de 1 metro
         */
        a0->om4_or_copper_status = SFP_OM4_LEN_VALID;

        if (is_copper)
            a0->om4_or_copper_length_m = raw_length;
        else
            a0->om4_or_copper_length_m = (uint16_t)raw_length * 10;
    }
}

/* ============================================
 * Função Getter
 * ============================================ */
uint16_t sfp_a0_get_om4_copper_or_length_m(const sfp_a0h_base_t *a0, sfp_om4_length_status_t *status)
{
    if (!a0) {
        if (status)
            *status = SFP_OM4_LEN_NOT_SUPPORTED;
        return 0;
    }
    if (status)
        *status = a0->om4_or_copper_status;
    return a0->om4_or_copper_length_m;
}

/* ============================================
 * Byte 36 — Extended Compliance Codes (SFF-8024 Table 4-4)
 * ============================================ */
void sfp_parse_a0_base_ext_compliance(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;
    a0->ext_compliance = (sfp_extended_spec_compliance_code_t)a0_base_data[36];
}

/* ============================================
 * Função Getter
 * ============================================ */
sfp_extended_spec_compliance_code_t sfp_a0_get_ext_compliance(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return EXT_SPEC_COMPLIANCE_UNSPECIFIED;
    return a0->ext_compliance;
}

/* ============================================
 * Byte 37-39 — Vendor OUI
 * ============================================ */
void sfp_parse_a0_base_vendor_oui(const uint8_t *a0_base_data,
                                  sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    /* Bytes 37–39: Vendor OUI (IEEE Company Identifier) */
    a0->vendor_oui[0] = a0_base_data[37];
    a0->vendor_oui[1] = a0_base_data[38];
    a0->vendor_oui[2] = a0_base_data[39];
}

/* ============================================
 * Função Getter
 * ============================================ */
bool sfp_a0_get_vendor_oui(const sfp_a0h_base_t *a0,
                           uint8_t oui_buffer[3])
{
    if (!a0 || !oui_buffer)
        return false;

    oui_buffer[0] = a0->vendor_oui[0];
    oui_buffer[1] = a0->vendor_oui[1];
    oui_buffer[2] = a0->vendor_oui[2];

    return true;
}

/* ============================================
*    Converte o Vendor OUI para um identificador de 24 bits.
*    @param a0 Ponteiro para a estrutura sfp_a0h_base_t contendo o Vendor OUI já parseado.
*    @return Valor inteiro de 24 bits representando o Vendor OUI. Caso a estrutura seja inválida, retorna 0.
* ============================================ */
uint32_t sfp_vendor_oui_to_u32(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return 0;

    return ((uint32_t)a0->vendor_oui[0] << 16) |
           ((uint32_t)a0->vendor_oui[1] << 8)  |
           ((uint32_t)a0->vendor_oui[2]);
}

/* ============================================
 * Byte 62 — Fibre Channel Speed 2
 * ============================================ */
void sfp_parse_a0_fc_speed_2(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    a0->fc_speed2 = a0_base_data[62];
}

/* ============================================
 * Função Getter
 * ============================================ */
bool sfp_get_a0_fc_speed_2(const sfp_a0h_base_t *a0, const sfp_compliance_decoded_t *comp)
{
    uint8_t b = a0->fc_speed2;

    if (!a0 || !comp)
        return false;

    if (!comp->see_byte_62) {
        return false;
    }

    uint8_t speed = b & (1 << 0);

    return speed;
}

/* ============================================
 * Byte 63 — CC_BASE Checksum
 * ============================================ */
void sfp_parse_a0_base_cc_base(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    uint16_t sum = 0;
    for (int i = 0; i < 63; i++) {
        sum += a0_base_data[i];
    }

    uint8_t sum_mod256 = (uint8_t)(sum & 0xFF);

    uint8_t checksum_byte = a0_base_data[63];

    a0->cc_base_is_valid = (sum_mod256 == checksum_byte);

    a0->cc_base = checksum_byte;
}

/* ============================================
 * Função Getter
 * ============================================ */
bool sfp_a0_get_cc_base_is_valid(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return false;
    return a0->cc_base_is_valid;
}
