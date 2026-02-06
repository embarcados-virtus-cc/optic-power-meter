#ifndef SFP_8472_DEFS_H
#define SFP_8472_DEFS_H

#include <stdint.h>

/*
 * @file SFP8472_defs.h
 * @brief Definições para acesso à memória de transceptores SFP-8472
 *
 */

/*
 * @brief Endereços I2C (2-wire) conforme Seção 4 [1, 2]
 */
typedef enum {
    SFP8472_ADDR_A0 = 0x50, /* Serial ID e Extended ID */
    SFP8472_ADDR_A2 = 0x51  /* Diagnósticos, Controle e Páginas */
} SFP8472_i2c_addr_t;

/*
 * @brief Offsets de Memória para o Endereço A0h (Serial ID)
 * Conforme Tabela 4-2 [3-8]
 */
typedef enum {
    /* Base ID Fields (0-63) */
    SFP8472_A0_IDENTIFIER           = 0,   /* Tipo de transceptor  */
    SFP8472_A0_EXT_IDENTIFIER       = 1,   /* ID estendido  */
    SFP8472_A0_CONNECTOR            = 2,   /* Código do conector  */
    SFP8472_A0_TRANSCEIVER          = 3,   /* Compatibilidade (Bytes 3-10) */
    SFP8472_A0_ENCODING             = 11,  /* Algoritmo de codificação  */
    SFP8472_A0_BR_NOMINAL           = 12,  /* Taxa de sinalização nominal (100MBd) */
    SFP8472_A0_RATE_IDENTIFIER      = 13,  /* Tipo de Rate Select  */
    SFP8472_A0_LENGTH_SMF_KM        = 14,  /* SMF (km) ou Atenuação de cobre (12.9GHz)  */
    SFP8472_A0_LENGTH_SMF_100M      = 15,  /* SMF (100m) ou Atenuação de cobre (25.78GHz)  */
    SFP8472_A0_LENGTH_OM2_10M       = 16,  /* 50um OM2 (10m)  */
    SFP8472_A0_LENGTH_OM1_10M       = 17,  /* 62.5um OM1 (10m) [ */
    SFP8472_A0_LENGTH_OM4_10M       = 18,  /* OM4 ou comprimento de cabo de cobre (m)  */
    SFP8472_A0_LENGTH_OM3_10M       = 19,  /* OM3 ou multiplicador de cabo de cobre  */
    SFP8472_A0_VENDOR_NAME          = 20,  /* Nome do fornecedor (ASCII, 16 bytes)  */
    SFP8472_A0_EXT_TRANSCEIVER      = 36,  /* Código de compatibilidade estendido */
    SFP8472_A0_VENDOR_OUI           = 37,  /* IEEE Company ID (3 bytes) */
    SFP8472_A0_VENDOR_PN            = 40,  /* Part Number (ASCII, 16 bytes) */
    SFP8472_A0_VENDOR_REV           = 56,  /* Revisão (ASCII, 4 bytes) */
    SFP8472_A0_WAVELENGTH           = 60,  /* Comprimento de onda (2 bytes)  */
    SFP8472_A0_FIBRE_CHANNEL_SPD2   = 62,  /* Velocidades FC (ex: 64GFC)  */
    SFP8472_A0_CC_BASE              = 63,  /* Checksum bytes 0-62  */

    /* Extended ID Fields (64-95) */
    SFP8472_A0_OPTIONS              = 64,  /* Sinais opcionais implementados (2 bytes)  */
    SFP8472_A0_BR_MAX               = 66,  /* Margem superior ou taxa nominal (250MBd)  */
    SFP8472_A0_BR_MIN               = 67,  /* Margem inferior */
    SFP8472_A0_VENDOR_SN            = 68,  /* Serial Number (ASCII, 16 bytes)  */
    SFP8472_A0_DATE_CODE            = 84,  /* Código de data de fabricação (8 bytes) */
    SFP8472_A0_DIAG_MONITORING_TYPE = 92,  /* Tipo de diagnóstico e calibração  */
    SFP8472_A0_ENHANCED_OPTIONS     = 93,  /* Recursos de diagnóstico opcionais  */
    SFP8472_A0_SFP8472_COMPLIANCE   = 94,  /* Revisão da norma compatível */
    SFP8472_A0_CC_EXT               = 95,  /* Checksum bytes 64-94  */

    /* Vendor Specific (96-127) */
    SFP8472_A0_VENDOR_SPECIFIC      = 96   /* Área específica do fornecedor  */
} SFP8472_a0_offset_t;

/*
 * @brief Offsets de Memória para o Endereço A2h (Diagnósticos)
 * Conforme Tabela 4-3 e Tabela 9-16 
 */
typedef enum {
    /* Limiares de Alarme e Aviso (0-55) [43-45] */
    SFP8472_A2_THRESHOLDS_BASE      = 0,   /* Início dos thresholds  */
    SFP8472_A2_OPT_THRESHOLDS_BASE  = 40,  /* Thresholds Laser/TEC  */

    /* Constantes de Calibração / Recursos Avançados (56-91) */
    SFP8472_A2_CAL_CONST_OR_ENHANCED = 56, /* Constantes ou Recursos Melhorados */
    SFP8472_A2_MAX_PWR_CONSUMPTION   = 66, /* Consumo máximo (LSB=0.1W) se bit A0.64.6=1 */

    SFP8472_A2_CC_DMI                = 95, /* Checksum bytes 0-94  */

    /* Dados em Tempo Real (96-109) [38, 39] */
    SFP8472_A2_TEMP_CURR             = 96,  /* Temperatura interna (q8.8) */
    SFP8472_A2_VCC_CURR              = 98,  /* Tensão de alimentação (LSB=100uV) */
    SFP8472_A2_TX_BIAS_CURR          = 100, /* Corrente de Bias do Laser  */
    SFP8472_A2_TX_POWER_CURR         = 102, /* Potência de transmissão  */
    SFP8472_A2_RX_POWER_CURR         = 104, /* Potência recebida */
    SFP8472_A2_OPT_LASER_TEMP_WAVE   = 106, /* Temperatura/Wavelength do Laser  */
    SFP8472_A2_OPT_TEC_CURR          = 108, /* Corrente TEC */

    /* Status e Controle [39-42] */
    STATUS_CONTROL        	     = 110, /* Bits de status e controle soft  */
    SFP8472_A2_ALARM_FLAGS           = 112, /* Flags de Alarme */
    SFP8472_A2_TX_INPUT_EQ_CTRL      = 114, /* Controle de Equalização de Entrada */
    SFP8472_A2_RX_OUT_EMPH_CTRL      = 115, /* Controle de Ênfase de Saída  */
    SFP8472_A2_WARNING_FLAGS         = 116, /* Flags de Aviso */
    SFP8472_A2_EXT_STATUS_CONTROL    = 118, /* Status/Controle estendido (PAM4/PL4)  */

    SFP8472_A2_PAGE_SELECT           = 127  /* Seletor de Página */
} SFP8472_a2_offset_t;

/*
 * @brief Offsets de Memória para A2h Página 02h (Tunability, RDT, RPM)
 * Conforme Tabelas 12-1 e 12-5 [64-68]
 */
typedef enum {
    SFP8472_A2_P02_FEAT_TUNABILITY   = 128, /* SFP-8690 Advertisement */
    SFP8472_A2_P02_FEAT_ADV          = 129, /* Advertisement RPM/RDT */
    SFP8472_A2_P02_RDT_CTRL          = 130, /* Modo RDT */
    SFP8472_A2_P02_RDT_VALUE         = 131, /* Valor RDT */
    SFP8472_A2_P02_CH_TUNING_START   = 144, /* Canais de sintonia (SFP-8690) */
    SFP8472_A2_P02_RPM_COR_LATCH     = 174, /* Alarme latched do Remote PM */
    SFP8472_A2_P02_RPM_STATUS        = 192, /* Status RPM (Escrita A5h reseta erro)  */
    SFP8472_A2_P02_RPM_ERR_COUNTERS  = 198, /* Contadores de erro RPM */
    SFP8472_A2_P02_RPM_TX_MOD_INDEX  = 211, /* Índice de Modulação/Enable TX RPM */
    SFP8472_A2_P02_RPM_USER_TX_DATA  = 240, /* Envio de dados de usuário RPM */
    SFP8472_A2_P02_RPM_USER_RX_DATA  = 248  /* Recebimento de dados de usuário RPM  */
} SFP8472_a2_p02_offset_t;

/*
 * @brief Offsets de Memória para A2h Página 03h (High Accuracy Timing)
 * Conforme Tabelas 13-1 e 13-3 
 */
typedef enum {
    SFP8472_A2_P03_FORMAT_ID         = 128, /* CA1Bh=CALB ou 100Bh=LOOB  */
    SFP8472_A2_P03_VERSION           = 130, /* Versão (01h) */
    SFP8472_A2_P03_CALIB_DATE        = 131, /* Data de calibração  */
    SFP8472_A2_P03_CALIB_ID          = 134, /* ID único de calibração (CUI)  */
    SFP8472_A2_P03_STRATUM           = 140, /* Geração do calibrador */
    SFP8472_A2_P03_NB_LANES          = 150, /* Número de lanes (SFP-8472=1) */
    SFP8472_A2_P03_OP_MODE_ID        = 151, /* ID do modo operacional  */
    SFP8472_A2_P03_AVG_RX_DELAY      = 179, /* Atraso médio RX (ns, q16.16) */
    SFP8472_A2_P03_AVG_TX_DELAY      = 183, /* Atraso médio TX (ns, q16.16) */
    SFP8472_A2_P03_CC_CALIB          = 255  /* Checksum da página 03h  */
} SFP8472_a2_p03_offset_t;

/*
 * @brief Tipos de página suportados no seletor de página (A2h offset 127)
 */
typedef enum {
    SFP8472_PAGE_00 = 0x00, /* Página principal (Diagnósticos) */
    SFP8472_PAGE_01 = 0x01, /* Reservado */
    SFP8472_PAGE_02 = 0x02, /* Tunabilidade, RDT, RPM */
    SFP8472_PAGE_03 = 0x03, /* High Accuracy Timing */
    SFP8472_PAGE_04 = 0x04  /* Reservado para uso futuro */
} SFP8472_page_t;


/* Byte 92 (A0h) - Diagnostic Monitoring Type [Table 8-5] */
#define SFP_A0_BIT_DMI_IMPL           6  /** 1 = Monitoramento Digital implementado */
#define SFP_A0_BIT_INTERNAL_CAL       5  /** 1 = Calibração Interna */
#define SFP_A0_BIT_EXTERNAL_CAL       4  /** 1 = Calibração Externa */
#define SFP_A0_BIT_RX_PWR_AVG         3  /** 0 = OMA, 1 = Potência Média */
#define SFP_A0_BIT_ADDR_CHANGE_REQ    2  /** 1 = Requer sequência de mudança de endereço  */
#define SFP_A0_BIT_RPM_IMPL           1  /** 1 = Registros de Remote Performance Monitoring presentes */


/* Byte 110 (A2h) - Status/Control Bits [Table 9-16] */
#define SFP_A2_BIT_TX_DISABLE_STATE   7  /** Estado digital do pino TX Disable */
#define SFP_A2_BIT_SOFT_TX_DISABLE    6  /** Escrita '1' desabilita o laser por software */
#define SFP_A2_BIT_RS1_STATE          5  /** Estado do pino RS(1) ou AS(1) */
#define SFP_A2_BIT_RS0_STATE          4  /** Estado do pino RS(0) ou AS(0) */
#define SFP_A2_BIT_SOFT_RS0_SELECT    3  /** Escrita '1' seleciona full bandwidth */
#define SFP_A2_BIT_TX_FAULT_STATE     2  /** Estado digital do pino TX Fault  */
#define SFP_A2_BIT_RX_LOS_STATE       1  /** Estado digital do pino RX_LOS  */
#define SFP_A2_BIT_DATA_NOT_READY     0  /** 1 = Módulo ainda não tem dados de monitoramento válidos */


/*
 * @brief Macros  para cálculos dos valores de Diagnosticos
 */
#define SFP8472_TEMP_TO_DEGC(raw)    (((int16_t)(raw)) / 256.0f)   /* q8.8 para °C */
#define SFP8472_VCC_TO_VOLTS(raw)    ((raw) * 0.0001)             /* 100uV/LSB para Volts */
#define SFP8472_BIAS_TO_MA(raw)      ((raw) * 2.0)                /* 2µA/LSB para mA */
#define SFP8472_POWER_TO_UW(raw)     ((raw) * 0.1)                /* 0.1µW/LSB para µW */


#endif /* SFP_8472_DEFS_H */