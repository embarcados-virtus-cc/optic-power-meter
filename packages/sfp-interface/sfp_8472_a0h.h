/**
 * @file sfp_8472_a0h.h
 * @brief Biblioteca de parsing SFF-8472 – Página A0h (Base ID)
 *
 * @author Alexandre Junior
 * @author Carlos Elias
 * @author Melquisedeque Leite
 * @author Miguel Ryan
 * @author Nicholas Gomes
 * @author Pablo Daniel
 * @author Pedro Lucena
 * @author Pedro Wilson
 *
 * @details
 *  Implementa a leitura e interpretação dos campos da EEPROM A0h
 *  de módulos SFP/SFP+, conforme a especificação SFF-8472.
 */

#ifndef SFF_8472_A0H_H
#define SFF_8472_A0H_H

#include <stdint.h>
#include <stdbool.h>

/************************************
 * Basic Type Definitions
 *************************************/

/** @brief Endereço I2C para EEPROM A0h (Base ID e Extended ID) */
#define SFP_I2C_ADDR_A0     0x50

/** @brief Tamanho do bloco base A0h (Bytes 0-63) */
#define SFP_A0_BASE_SIZE    64

/** @brief Byte 1 (Extended Identifier) */
#define SFP_EXT_IDENTIFIER_EXPECTED 0x04

/** @brief  */
#define SFP_NOMINAL_RATE_RAW_UNSPECIFIED 0x00u
/** @brief  */
#define SFP_NOMINAL_RATE_RAW_EXTENDED    0xFFu
/** @brief  */
#define SFP_NOMINAL_RATE_RAW_UNIT_MBD   100u


/*==========================================
 * Byte 0 — Identifier (SFF-8472 / SFF-8024)
 ===========================================*/
typedef enum {
    SFP_ID_UNKNOWN   = 0x00,
    SFP_ID_GBIC      = 0x02,
    SFP_ID_SFP       = 0x03,
    SFP_ID_QSFP      = 0x0C,
    SFP_ID_QSFP_PLUS = 0x11,
    SFP_I_QSFP28     = 0x18
} sfp_identifier_t;

/* ==============================
 * Byte 2 — Connector Types
 * SFF-8024 Table 4-3
 * ============================== */
typedef enum {
    SFP_CONNECTOR_UNKNOWN           = 0x00,
    SFP_CONNECTOR_SC                = 0x01,
    SFP_CONNECTOR_FC_STYLE_1        = 0x02,
    SFP_CONNECTOR_FC_STYLE_2        = 0x03,
    SFP_CONNECTOR_BNC_TNC           = 0x04,
    SFP_CONNECTOR_FC_COAX           = 0x05,
    SFP_CONNECTOR_FIBER_JACK        = 0x06,
    SFP_CONNECTOR_LC                = 0x07,
    SFP_CONNECTOR_MT_RJ             = 0x08,
    SFP_CONNECTOR_MU                = 0x09,
    SFP_CONNECTOR_SG                = 0x0A,
    SFP_CONNECTOR_OPTICAL_PIGTAIL   = 0x0B,
    SFP_CONNECTOR_MPO_1X12          = 0x0C,
    SFP_CONNECTOR_MPO_2X16          = 0x0D,
    SFP_CONNECTOR_HSSDC_II          = 0x20,
    SFP_CONNECTOR_COPPER_PIGTAIL    = 0x21,
    SFP_CONNECTOR_RJ45              = 0x22,
    SFP_CONNECTOR_NO_SEPARABLE      = 0x23
} sfp_connector_type_t;

/* ==============================
 * Byte 11 — Encoding values
 * ============================== */
typedef enum {
    SFP_ENC_UNSPECIFIED     = 0x00,
    SFP_ENC_8B_10B          = 0x01,
    SFP_ENC_4B_5B           = 0x02,
    SFP_ENC_NRZ             = 0x03,
    SFP_ENC_MANCHESTER      = 0x04,
    SFP_ENC_SONET_SCRAMBLED = 0x05,
    SFP_ENC_64B_66B         = 0x06,
    SFP_ENC_256B_257B       = 0x07,
    SFP_ENC_PAM4            = 0x08,
} sfp_encoding_codes_t;

/* ==============================
 * Byte 12 — Signaling Rate, Nominal
 * ============================== */
typedef enum {
    SFP_A0_BYTE_NOMINAL_RATE = 12
} sfp_a0_nominal_rate_offset_t;

typedef enum {
    SFP_NOMINAL_RATE_NOT_SPECIFIED = 0, /* 00h */
    SFP_NOMINAL_RATE_VALID,            /* 01h-0xFE (raw * 100 MBd) */
    SFP_NOMINAL_RATE_EXTENDED          /* 0xFF (rate in extended block, not parsed here) */
} sfp_nominal_rate_status_t;

/* ==============================
* Byte 14 — Status da informação
* de alcance SMF ou atenuação de
* cobre
* ============================== */
typedef enum {
  SFP_SMF_LEN_NOT_SUPPORTED,    /* Byte 14 = 0x00 */
  SFP_SMF_LEN_VALID,            /* Byte 14 = 0x01–0xFE */
  SFP_SMF_LEN_EXTENDED          /* Byte 14 = 0xFF */
} sfp_smf_length_status_t;

/* ==============================
 * Byte 16 — Status da
 * informação de alcance OM2
 * ============================== */
typedef enum {
    SFP_OM2_LEN_NOT_SUPPORTED,   /* Byte 16 = 0x00 */
    SFP_OM2_LEN_VALID,           /* 0x01–0xFE */
    SFP_OM2_LEN_EXTENDED         /* Byte 16 = 0xFF */
} sfp_om2_length_status_t;

/* ==============================
 * Byte 17 — Status da
 * informação de alcance OM1
 * ============================== */
typedef enum {
    SFP_OM1_LEN_NOT_SUPPORTED,   /* Byte 17 = 0x00 */
    SFP_OM1_LEN_VALID,           /* 0x01–0xFE */
    SFP_OM1_LEN_EXTENDED         /* Byte 17 = 0xFF */
} sfp_om1_length_status_t;

/* ==============================
 * Byte 18 — Status da
 * informação de alcance OM4
 * ============================== */
typedef enum {
  SFP_OM4_LEN_NOT_SUPPORTED,    /* Byte 18 = 0x00 */
  SFP_OM4_LEN_VALID,            /* Byte 18 = 0x01–0xFE */
  SFP_OM4_LEN_EXTENDED          /* Byte 18 = 0xFF */
} sfp_om4_length_status_t;

/* ==============================
 * Bytes 20-35 — Vendor Name
 * ============================== */
typedef enum {
    SFP_A0_BYTE_VENDOR_NAME = 20,
    SFP_A0_LEN_VENDOR_NAME  = 16,
    SFP_A0_END_VENDOR_NAME  = 35
} sfp_a0_vendor_name_offset_t;

/**
 * @brief Códigos de Conformidade de Especificação Estendida (Tabela 4-4 do SFF-8024)
 *
 * @details
 *  Códigos que identificam capacidades do módulo não cobertas pelas tabelas básicas.
 *  Usados pelo SFF-8472 (Byte 36) e SFF-8636 (Bytes 116, 192) para reportar
 *  suporte a interfaces ópticas/elétricas (AOC/ACC, variantes Ethernet, FC, etc.).
 *
 *  Nota: Códigos 0x80-0x81 não são para dispositivos single-lane SFF-8472.
 *        0x82-0xFE reservados, 0xFF específico do fabricante.
 *
 *  Referência: SFF-8024 Rev 4.13, Seção 4.5, Páginas 20-22.
 */

typedef enum {
    EXT_SPEC_COMPLIANCE_UNSPECIFIED                                                  = 0x00,
    EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_5E_5                          = 0x01,
    EXT_SPEC_COMPLIANCE_100GBASE_SR4_OR_25GBASE_SR                                   = 0x02,
    EXT_SPEC_COMPLIANCE_100GBASE_LR4_OR_25GBASE_LR                                   = 0x03,
    EXT_SPEC_COMPLIANCE_100GBASE_ER4_OR_25GBASE_ER                                   = 0x04,
    EXT_SPEC_COMPLIANCE_100GBASE_SR10                                                = 0x05,
    EXT_SPEC_COMPLIANCE_100G_CWDM4                                                   = 0x06,
    EXT_SPEC_COMPLIANCE_100G_PSM4                                                    = 0x07,
    EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_5E_5                          = 0x08,
    EXT_SPEC_COMPLIANCE_OBSOLETE                                                     = 0x09,
    EXT_SPEC_COMPLIANCE_RESERVED_0A                                                  = 0x0A,
    EXT_SPEC_COMPLIANCE_100GBASE_CR4_OR_25GBASE_CR_CA_25G_L_OR_50GBASE_CR2_RS_FEC    = 0x0B,
    EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_S_OR_50GBASE_CR2_BASE_R_FEC                = 0x0C,
    EXT_SPEC_COMPLIANCE_25GBASE_CR_CA_25G_N_OR_50GBASE_CR2_NO_FEC                    = 0x0D,
    EXT_SPEC_COMPLIANCE_10MB_SINGLE_PAIR_ETHERNET                                    = 0x0E,
    EXT_SPEC_COMPLIANCE_RESERVED_0F                                                  = 0x0F,
    EXT_SPEC_COMPLIANCE_40GBASE_ER4                                                  = 0x10,
    EXT_SPEC_COMPLIANCE_4X_10GBASE_SR                                                = 0x11,
    EXT_SPEC_COMPLIANCE_40G_PSM4                                                     = 0x12,
    EXT_SPEC_COMPLIANCE_G9591_P111_2D1                                               = 0x13,
    EXT_SPEC_COMPLIANCE_G9591_P151_2D2                                               = 0x14,
    EXT_SPEC_COMPLIANCE_G9591_P111_2D2                                               = 0x15,
    EXT_SPEC_COMPLIANCE_10GBASE_T_SFI                                               = 0x16,
    EXT_SPEC_COMPLIANCE_100G_CLR4                                                    = 0x17,
    EXT_SPEC_COMPLIANCE_100G_AOC_OR_25GAUI_C2M_AOC_BER_1E_12                         = 0x18,
    EXT_SPEC_COMPLIANCE_100G_ACC_OR_25GAUI_C2M_ACC_BER_1E_12                         = 0x19,
    EXT_SPEC_COMPLIANCE_100GE_DWDM2                                                  = 0x1A,
    EXT_SPEC_COMPLIANCE_100G_1550NM_WDM_4L                                           = 0x1B,
    EXT_SPEC_COMPLIANCE_10GBASE_T_SHORT_REACH                                        = 0x1C,
    EXT_SPEC_COMPLIANCE_5GBASE_T                                                     = 0x1D,
    EXT_SPEC_COMPLIANCE_2_5GBASE_T                                                   = 0x1E,
    EXT_SPEC_COMPLIANCE_40G_SWDM4                                                    = 0x1F,
    EXT_SPEC_COMPLIANCE_100G_SWDM4                                                   = 0x20,
    EXT_SPEC_COMPLIANCE_100G_PAM4_BIDI                                               = 0x21,
    EXT_SPEC_COMPLIANCE_4WDM_10_MSA                                                  = 0x22,
    EXT_SPEC_COMPLIANCE_4WDM_20_MSA                                                  = 0x23,
    EXT_SPEC_COMPLIANCE_4WDM_40_MSA                                                  = 0x24,
    EXT_SPEC_COMPLIANCE_100GBASE_DR_CAUI4_NO_FEC                                     = 0x25,
    EXT_SPEC_COMPLIANCE_100G_FR_OR_100GBASE_FR1_CAUI4_NO_FEC                         = 0x26,
    EXT_SPEC_COMPLIANCE_100G_LR_OR_100GBASE_LR1_CAUI4_NO_FEC                         = 0x27,
    EXT_SPEC_COMPLIANCE_100GBASE_SR1_CAUI4_NO_FEC                                    = 0x28,
    EXT_SPEC_COMPLIANCE_100GBASE_SR1_OR_200GBASE_SR2_OR_400GBASE_SR4                 = 0x29,
    EXT_SPEC_COMPLIANCE_100GBASE_FR1_OR_400GBASE_DR4_2                               = 0x2A,
    EXT_SPEC_COMPLIANCE_100GBASE_LR1                                                 = 0x2B,
    EXT_SPEC_COMPLIANCE_100G_LR1_20_MSA_CAUI4_NO_FEC                                 = 0x2C,
    EXT_SPEC_COMPLIANCE_100G_FR1_30_MSA_CAUI4_NO_FEC                                 = 0x2D,
    EXT_SPEC_COMPLIANCE_100G_FR1_40_MSA_CAUI4_NO_FEC                                 = 0x2E,
    EXT_SPEC_COMPLIANCE_100G_LR1_20_MSA                                              = 0x2F,
    EXT_SPEC_COMPLIANCE_ACTIVE_CU_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6        = 0x30,
    EXT_SPEC_COMPLIANCE_ACTIVE_OPTICAL_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_1E_6   = 0x31,
    EXT_SPEC_COMPLIANCE_ACTIVE_CU_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_2_6E_4      = 0x32,
    EXT_SPEC_COMPLIANCE_ACTIVE_OPTICAL_CABLE_50GAUI_100GAUI2_200GAUI4_C2M_BER_2_6E_4 = 0x33,
    EXT_SPEC_COMPLIANCE_100G_FR1_30_MSA                                              = 0x34,
    EXT_SPEC_COMPLIANCE_100G_FR1_40_MSA                                              = 0x35,
    EXT_SPEC_COMPLIANCE_100GBASE_VR1_CAUI4_NO_FEC                                    = 0x36,
    EXT_SPEC_COMPLIANCE_10GBASE_BR                                                   = 0x37,
    EXT_SPEC_COMPLIANCE_25GBASE_BR                                                   = 0x38,
    EXT_SPEC_COMPLIANCE_50GBASE_BR                                                   = 0x39,
    EXT_SPEC_COMPLIANCE_100GBASE_VR1_CAUI4_NO_FEC_2                                  = 0x3A,
    EXT_SPEC_COMPLIANCE_RESERVED_3B                                                  = 0x3B,
    EXT_SPEC_COMPLIANCE_RESERVED_3C                                                  = 0x3C,
    EXT_SPEC_COMPLIANCE_RESERVED_3D                                                  = 0x3D,
    EXT_SPEC_COMPLIANCE_RESERVED_3E                                                  = 0x3E,
    EXT_SPEC_COMPLIANCE_100GBASE_CR1_OR_200GBASE_CR2_OR_400GBASE_CR4                 = 0x3F,
    EXT_SPEC_COMPLIANCE_50GBASE_CR_OR_100GBASE_CR2_OR_200GBASE_CR4                   = 0x40,
    EXT_SPEC_COMPLIANCE_50GBASE_R_OR_100GBASE_SR2_OR_200GBASE_SR4                    = 0x41,
    EXT_SPEC_COMPLIANCE_50GBASE_FR_OR_200GBASE_DR4                                   = 0x42,
    EXT_SPEC_COMPLIANCE_200GBASE_FR4                                                 = 0x43,
    EXT_SPEC_COMPLIANCE_200G_1550NM_PSM4                                             = 0x44,
    EXT_SPEC_COMPLIANCE_50GBASE_LR                                                   = 0x45,
    EXT_SPEC_COMPLIANCE_200GBASE_LR4                                                 = 0x46,
    EXT_SPEC_COMPLIANCE_400GBASE_DR4_400GAUI4_C2M                                    = 0x47,
    EXT_SPEC_COMPLIANCE_400GBASE_FR4                                                 = 0x48,
    EXT_SPEC_COMPLIANCE_400GBASE_LR4_6                                               = 0x49,
    EXT_SPEC_COMPLIANCE_RESERVED_4A                                                  = 0x4A,
    EXT_SPEC_COMPLIANCE_400G_LR4_10                                                  = 0x4B,
    EXT_SPEC_COMPLIANCE_400GBASE_ZR_OBSOLETE                                         = 0x4C,
    /* 0x4D–0x7E are reserved */
    EXT_SPEC_COMPLIANCE_256GFC_SW4                                                   = 0x7F,
    EXT_SPEC_COMPLIANCE_64GFC                                                        = 0x80,
    EXT_SPEC_COMPLIANCE_128GFC                                                       = 0x81,
    /* 0x82–0xFE are reserved */
    EXT_SPEC_COMPLIANCE_VENDOR_SPECIFIC                                              = 0xFF
} sfp_extended_spec_compliance_code_t;

/* Bytes 3-10: Transceiver Compliance Codes */
typedef struct {
    uint8_t byte3;
    uint8_t byte4;
    uint8_t byte5;
    uint8_t byte6;
    uint8_t byte7;
    uint8_t byte8;
    uint8_t byte9;
    uint8_t byte10;
} sfp_compliance_codes_t;

typedef struct {
    /* ==============================
     * Byte 3 — Ethernet / InfiniBand
     * ============================== */
    bool eth_10g_base_sr;
    bool eth_10g_base_lr;
    bool eth_10g_base_lrm;
    bool eth_10g_base_er;
    bool infiniband_1x_sx;
    bool infiniband_1x_lx;
    bool infiniband_1x_copper_active;
    bool infiniband_1x_copper_passive;

    /* ======================
     * Byte 4 — ESCON / SONET
     * ====================== */
    bool escon_mmf;
    bool escon_smf;
    bool oc_192_sr;
    bool sonet_rs_1;
    bool sonet_rs_2;
    bool oc_48_lr;
    bool oc_48_ir;
    bool oc_48_sr;

    /* ======================
     * Byte 5 — SONET
     * ====================== */
    bool oc_12_sm_lr;
    bool oc_12_sm_ir;
    bool oc_12_sr;
    bool oc_3_sm_lr;
    bool oc_3_sm_ir;
    bool oc_3_sr;

    /* ======================
     * Byte 6 — Ethernet 1G
     * ====================== */
    bool eth_base_px;
    bool eth_base_bx_10;
    bool eth_100_base_fx;
    bool eth_100_base_lx;
    bool eth_1000_base_t;
    bool eth_1000_base_cx;
    bool eth_1000_base_lx;
    bool eth_1000_base_sx;

    /* ====================================
     * Byte 7 — FC Link Length & Technology
     * ==================================== */
    bool fc_very_long_distance;
    bool fc_short_distance;
    bool fc_intermediate_distance;
    bool fc_long_distance;
    bool fc_medium_distance;
    bool shortwave_laser_sa;
    bool longwave_laser_lc;
    bool electrical_inter_enclosure;

    /* ==============================================
     * Byte 8 — FC technology & SFP+ Cable Technology
     * ============================================== */
    bool electrical_intra_enclosure;
    bool shortwave_laser_sn;
    bool shortwave_laser_sl;
    bool longwave_laser_ll;
    bool active_cable;
    bool passive_cable;

    /* ==============================
     * Byte 9 — FC Transmission Media
     * ============================== */
    bool twin_axial_pair;
    bool twisted_pair;
    bool miniature_coax;
    bool video_coax;
    bool multimode_m6; // 62.5 um
    bool multimode_m5; // 50 um
    bool single_mode;

    /* ==============================
     * Byte 10 — FC Channel Speed
     * ============================== */
    bool cs_1200_mbps;
    bool cs_800_mbps;
    bool cs_1600_mbps;
    bool cs_400_mbps;
    bool cs_3200_mbps;
    bool cs_200_mbps;
    bool see_byte_62;
    bool cs_100_mbps;
} sfp_compliance_decoded_t;

/**********************************************
 * A0h Memory Map (128 bytes) - Base ID Fields
 **********************************************/
typedef struct {
    /* Byte 0: Identifier */
    sfp_identifier_t identifier;

    /* Byte 1: Extended Identifier */
    uint8_t ext_identifier;

    /* Byte 2: Connector */
    uint8_t connector;

    /*Byte 3 - 10: Compliance Codes */
    sfp_compliance_codes_t cc;
    sfp_compliance_decoded_t dc;

    /* Byte 11: Encoding */
    sfp_encoding_codes_t encoding;

    /* Byte 12: Signaling Rate, Nominal */
    uint8_t nominal_rate;

    /* Byte 13: Rate Identifier */
    uint8_t rate_identifier;

    /* Byte 14: SMF Length or Copper Attenuation */
    uint16_t smf_length_m;
    sfp_smf_length_status_t smf_status;

    /* Byte 16: 50 um OM2 */
    uint16_t om2_length_m;
    sfp_om2_length_status_t om2_status;

    /* Byte 17: 62.5 um OM1 */
    uint16_t om1_length_m;
    sfp_om1_length_status_t om1_status;

    /* Byte 18: OM4 or Copper */
    uint16_t om4_or_copper_length_m;
    sfp_om4_length_status_t om4_or_copper_status;

    /* Bytes 20-35: Vendor Name (ASCII) */
    char vendor_name[16];
    bool is_valid_vendor_name;

    /* Byte 36: Extended Compliance Codes */
    sfp_extended_spec_compliance_code_t ext_compliance;

    /* Bytes 37-39: Vendor OUI */
    uint8_t vendor_oui[3];

    /* Bytes 40-55: Vendor Part Number (ASCII) */
    char vendor_pn[17];

    /* Bytes 56-59: Vendor Revision (ASCII) */
    char vendor_rev[5];

    /* Bytes 60-61: Wavelength or Cable Compliance */
    union {
        uint16_t wavelength;
        struct {
            uint8_t passive_bits;
            uint8_t active_bits;
        } cable_compliance;
    } media_info;

    /* Byte 62: Fibre Channel Speed 2 */
    uint8_t fc_speed2;

    /* Byte 63: CC_BASE (Checksum) */
    uint8_t cc_base;

    /* CC_BASE Validation */
    bool cc_base_is_valid;
} sfp_a0h_base_t;

/**********************************************
 * Function Prototypes
 **********************************************/

/* Byte 0 — Identifier */
void sfp_parse_a0_base_identifier(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
sfp_identifier_t sfp_a0_get_identifier(const sfp_a0h_base_t *a0);

/* Byte 1 — Extended Identifier */
void sfp_parse_a0_base_ext_identifier(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
uint8_t sfp_a0_get_ext_identifier(const sfp_a0h_base_t *a0);
bool sfp_validate_ext_identifier(const sfp_a0h_base_t *a0);

/* Byte 2 — Connector */
void sfp_parse_a0_base_connector(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
sfp_connector_type_t sfp_a0_get_connector(const sfp_a0h_base_t *a0);
const char *sfp_connector_to_string(sfp_connector_type_t connector);

/* Byte 3-10 Compliance Codes */
void sfp_read_compliance(const uint8_t *a0_base_data, sfp_compliance_codes_t *cc);
void sfp_decode_compliance(const sfp_compliance_codes_t *cc, sfp_compliance_decoded_t *out);
void sfp_print_compliance(const sfp_compliance_decoded_t *c);

/* Byte 11 — Encoding */
void sfp_parse_a0_base_encoding(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
sfp_encoding_codes_t sfp_a0_get_encoding(const sfp_a0h_base_t *a0);
void sfp_print_encoding(sfp_encoding_codes_t encoding);

/* Byte 14 SMF Length or Copper Attenuation */
void sfp_parse_a0_base_smf(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
uint16_t sfp_a0_get_smf_length_m(const sfp_a0h_base_t *a0, sfp_smf_length_status_t *status);

/* Byte 16 — OM2 Length Status (50 µm) */
void sfp_parse_a0_base_om2(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
uint16_t sfp_a0_get_om2_length_m(const sfp_a0h_base_t *a0, sfp_om2_length_status_t *status);

/* Byte 17 — OM1 (62.5 µm) */
void sfp_parse_a0_base_om1(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
uint16_t sfp_a0_get_om1_length_m(const sfp_a0h_base_t *a0, sfp_om1_length_status_t *status);

/* Byte 18 — OM4 or Copper Cable */
void sfp_parse_a0_base_om4_or_copper(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
uint16_t sfp_a0_get_om4_copper_or_length_m(const sfp_a0h_base_t *a0, sfp_om4_length_status_t *status);

/* Bytes 20-35 Vendor Name */
void sfp_parse_a0_base_vendor_name(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
bool sfp_a0_get_vendor_name(const sfp_a0h_base_t *a0, const char *vendor_name)

/* Byte 36 — Extended Compliance Codes */
void sfp_parse_a0_base_ext_compliance(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
sfp_extended_spec_compliance_code_t sfp_a0_get_ext_compliance(const sfp_a0h_base_t *a0);

/* Byte 37–39 Vendor OUI (IEEE Company Identifier) */
void sfp_parse_a0_base_vendor_oui(const uint8_t *a0_base_data,sfp_a0h_base_t *a0);
bool sfp_a0_get_vendor_oui(const sfp_a0h_base_t *a0, uint8_t oui_buffer[3]);
uint32_t sfp_vendor_oui_to_u32(const sfp_a0h_base_t *a0);

/* Byte 62 — Fibre Channel Speed 2 */
void sfp_parse_a0_fc_speed_2(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
bool sfp_get_a0_fc_speed_2(const sfp_a0h_base_t *a0, const sfp_compliance_decoded_t *comp);

/* Byte 63 — CC_BASE (Checksum) */
void sfp_parse_a0_base_cc_base(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
bool sfp_a0_get_cc_base_is_valid(const sfp_a0h_base_t *a0);

#endif /* SFF_8472_A0H_H */
