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

sfp_connector_type_t sfp_a0_get_connector(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return SFP_CONNECTOR_UNKNOWN;

    return a0->connector;
}

const char *sfp_connector_to_string(sfp_connector_type_t connector)
{
    switch (connector) {
        case SFP_CONNECTOR_SC:              return "SC";
        case SFP_CONNECTOR_FC_STYLE_1:      return "Fibre Channel Style 1";
        case SFP_CONNECTOR_FC_STYLE_2:      return "Fibre Channel Style 2";
        case SFP_CONNECTOR_BNC_TNC:         return "BNC/TNC";
        case SFP_CONNECTOR_FC_COAX:         return "Fibre Channel Coax";
        case SFP_CONNECTOR_FIBER_JACK:      return "Fiber Jack";
        case SFP_CONNECTOR_LC:              return "LC";
        case SFP_CONNECTOR_MT_RJ:           return "MT-RJ";
        case SFP_CONNECTOR_MU:              return "MU";
        case SFP_CONNECTOR_SG:              return "SG";
        case SFP_CONNECTOR_OPTICAL_PIGTAIL: return "Optical Pigtail";
        case SFP_CONNECTOR_MPO_1X12:        return "MPO 1x12";
        case SFP_CONNECTOR_MPO_2X16:        return "MPO 2x16";
        case SFP_CONNECTOR_HSSDC_II:        return "HSSDC II";
        case SFP_CONNECTOR_COPPER_PIGTAIL:  return "Copper Pigtail";
        case SFP_CONNECTOR_RJ45:            return "RJ45";
        case SFP_CONNECTOR_NO_SEPARABLE:    return "No Separable Connector";
        default:                            return "Unknown Connector";
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

/* Funções internas de decode (static) */
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

void sfp_print_compliance(const sfp_compliance_decoded_t *c)
{
    if (!c) return;
    printf("\n=== Transceiver Compliance Codes (Bytes 3–10) ===\n");
    /* ... (implementação completa de print_compliance) ... */
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

sfp_encoding_codes_t sfp_a0_get_encoding(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return SFP_ENC_UNSPECIFIED;
    return a0->encoding;
}

void sfp_print_encoding(sfp_encoding_codes_t encoding)
{
    printf("\n[Byte 11] Encoding:\n");
    switch ((uint8_t)encoding) {
        case SFP_ENC_UNSPECIFIED:    printf("  - Unspecified\n"); break;
        case SFP_ENC_8B_10B:          printf("  - 8B/10B\n"); break;
        case SFP_ENC_4B_5B:           printf("  - 4B/5B\n"); break;
        case SFP_ENC_NRZ:             printf("  - NRZ\n"); break;
        case SFP_ENC_MANCHESTER:      printf("  - Manchester\n"); break;
        case SFP_ENC_SONET_SCRAMBLED: printf("  - SONET Scrambled\n"); break;
        case SFP_ENC_64B_66B:         printf("  - 64B/66B\n"); break;
        case SFP_ENC_256B_257B:       printf("  - 256B/257B\n"); break;
        case SFP_ENC_PAM4:            printf("  - PAM4\n"); break;
        default:                      printf("  - Reserved / Unknown Code (0x%02X)\n", (uint8_t)encoding); break;
    }
}

/* =========================================================
 * Byte 14 — Length (SMF) or Attenuation (Copper)
 * =========================================================*/
void sfp_parse_a0_base_smf(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    uint8_t raw = a0_base_data[14];
    (void)sfp_is_copper; /* May be used in future for copper detection */

    if (raw == 0x00) {
        a0->smf_status   = SFP_SMF_LEN_NOT_SUPPORTED;
        a0->smf_length_m = 0;
    } else if (raw == 0xFF) {
        a0->smf_status   = SFP_SMF_LEN_EXTENDED;
        a0->smf_length_m = 254;
    } else {
        a0->smf_status   = SFP_SMF_LEN_VALID;
        a0->smf_length_m = (uint16_t)raw;
    }
}

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
    if (raw == 0x00) {
        a0->om2_status   = SFP_OM2_LEN_NOT_SUPPORTED;
        a0->om2_length_m = 0;
    } else if (raw == 0xFF) {
        a0->om2_status   = SFP_OM2_LEN_EXTENDED;
        a0->om2_length_m = 2540;
    } else {
        a0->om2_status   = SFP_OM2_LEN_VALID;
        a0->om2_length_m = (uint16_t)raw * 10;
    }
}

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
    if (raw == 0x00) {
        a0->om1_status   = SFP_OM1_LEN_NOT_SUPPORTED;
        a0->om1_length_m = 0;
    } else if (raw == 0xFF) {
        a0->om1_status   = SFP_OM1_LEN_EXTENDED;
        a0->om1_length_m = 2540;
    } else {
        a0->om1_status   = SFP_OM1_LEN_VALID;
        a0->om1_length_m = (uint16_t)raw * 10;
    }
}

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

    if (raw_length == 0x00) {
        a0->om4_or_copper_status = SFP_OM4_LEN_NOT_SUPPORTED;
        a0->om4_or_copper_length_m = 0;
    } else if (raw_length == 0xFF) {
        a0->om4_or_copper_status = SFP_OM4_LEN_EXTENDED;
        if (is_copper)
            a0->om4_or_copper_length_m = 254;
        else
            a0->om4_or_copper_length_m = 2540;
    } else {
        a0->om4_or_copper_status = SFP_OM4_LEN_VALID;
        if (is_copper)
            a0->om4_or_copper_length_m = raw_length;
        else
            a0->om4_or_copper_length_m = (uint16_t)raw_length * 10;
    }
}

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
 * Byte 36 — Extended Compliance Codes
 * ============================================ */
void sfp_parse_a0_base_ext_compliance(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;
    a0->ext_compliance = (sfp_extended_spec_compliance_code_t)a0_base_data[36];
}

sfp_extended_spec_compliance_code_t sfp_a0_get_ext_compliance(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return EXT_SPEC_COMPLIANCE_UNSPECIFIED;
    return a0->ext_compliance;
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
    uint8_t calc_checksum = (uint8_t)(sum & 0xFF);
    uint8_t stored_checksum = a0_base_data[63];
    a0->cc_base_is_valid = (calc_checksum == stored_checksum);
    a0->cc_base = stored_checksum;
}

bool sfp_a0_get_cc_base_is_valid(const sfp_a0h_base_t *a0)
{
    if (!a0)
        return false;
    return a0->cc_base_is_valid;
}

/* ============================================
 * Vendor Information (Bytes 20-35, 40-55, 56-59)
 * ============================================ */
void sfp_parse_a0_base_vendor_info(const uint8_t *a0_base_data, sfp_a0h_base_t *a0)
{
    if (!a0_base_data || !a0)
        return;

    memcpy(a0->vendor_name, &a0_base_data[20], 16);
    a0->vendor_name[16] = '\0';

    a0->vendor_oui[0] = a0_base_data[37];
    a0->vendor_oui[1] = a0_base_data[38];
    a0->vendor_oui[2] = a0_base_data[39];

    memcpy(a0->vendor_pn, &a0_base_data[40], 16);
    a0->vendor_pn[16] = '\0';

    memcpy(a0->vendor_rev, &a0_base_data[56], 4);
    a0->vendor_rev[4] = '\0';

    a0->media_info.wavelength = ((uint16_t)a0_base_data[60] << 8) | a0_base_data[61];
}

