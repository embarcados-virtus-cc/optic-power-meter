#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

/*
 * @file defs.h
 * @brief Definições para acesso à memória de transceptores SFP-8472
 *
 */

/*
 * @brief Endereços I2C (2-wire) conforme Seção 4 [1, 2]
 */
typedef enum {
    ADDR_A0 = 0x50, /* Serial ID e Extended ID */
    ADDR_A2 = 0x51  /* Diagnósticos, Controle e Páginas */
} i2c_addr_t;

/*
 * @brief Offsets de Memória para o Endereço A0h (Serial ID)
 * Conforme Tabela 4-2 [3-8]
 */
typedef enum {
    /* Base ID Fields (0-63) */
    A0_IDENTIFIER           = 0,   /* Tipo de transceptor  */
    A0_EXT_IDENTIFIER       = 1,   /* ID estendido  */
    A0_CONNECTOR            = 2,   /* Código do conector  */
    A0_TRANSCEIVER          = 3,   /* Compatibilidade (Bytes 3-10) */
    A0_ENCODING             = 11,  /* Algoritmo de codificação  */
    A0_BR_NOMINAL           = 12,  /* Taxa de sinalização nominal (100MBd) */
    A0_RATE_IDENTIFIER      = 13,  /* Tipo de Rate Select  */
    A0_LENGTH_SMF_KM        = 14,  /* SMF (km) ou Atenuação de cobre (12.9GHz)  */
    A0_LENGTH_SMF_100M      = 15,  /* SMF (100m) ou Atenuação de cobre (25.78GHz)  */
    A0_LENGTH_OM2_10M       = 16,  /* 50um OM2 (10m)  */
    A0_LENGTH_OM1_10M       = 17,  /* 62.5um OM1 (10m) [ */
    A0_LENGTH_OM4_10M       = 18,  /* OM4 ou comprimento de cabo de cobre (m)  */
    A0_LENGTH_OM3_10M       = 19,  /* OM3 ou multiplicador de cabo de cobre  */
    A0_VENDOR_NAME          = 20,  /* Nome do fornecedor (ASCII, 16 bytes)  */
    A0_EXT_TRANSCEIVER      = 36,  /* Código de compatibilidade estendido */
    A0_VENDOR_OUI           = 37,  /* IEEE Company ID (3 bytes) */
    A0_VENDOR_PN            = 40,  /* Part Number (ASCII, 16 bytes) */
    A0_VENDOR_REV           = 56,  /* Revisão (ASCII, 4 bytes) */
    A0_WAVELENGTH           = 60,  /* Comprimento de onda (2 bytes)  */
    A0_FIBRE_CHANNEL_SPD2   = 62,  /* Velocidades FC (ex: 64GFC)  */
    A0_CC_BASE              = 63,  /* Checksum bytes 0-62  */

    /* Extended ID Fields (64-95) */
    A0_OPTIONS              = 64,  /* Sinais opcionais implementados (2 bytes)  */
    A0_BR_MAX               = 66,  /* Margem superior ou taxa nominal (250MBd)  */
    A0_BR_MIN               = 67,  /* Margem inferior */
    A0_VENDOR_SN            = 68,  /* Serial Number (ASCII, 16 bytes)  */
    A0_DATE_CODE            = 84,  /* Código de data de fabricação (8 bytes) */
    A0_DIAG_MONITORING_TYPE = 92,  /* Tipo de diagnóstico e calibração  */
    A0_ENHANCED_OPTIONS     = 93,  /* Recursos de diagnóstico opcionais  */
    A0_COMPLIANCE   = 94,  /* Revisão da norma compatível */
    A0_CC_EXT               = 95,  /* Checksum bytes 64-94  */

    /* Vendor Specific (96-127) */
    A0_VENDOR_SPECIFIC      = 96   /* Área específica do fornecedor  */
} a0_offset_t;

/*
 * @brief Offsets de Memória para o Endereço A2h (Diagnósticos)
 * Conforme Tabela 4-3 e Tabela 9-16
 */
typedef enum {
    /* Limiares de Alarme e Aviso (0-55) [43-45] */
    A2_THRESHOLDS_BASE      = 0,   /* Início dos thresholds  */
    A2_OPT_THRESHOLDS_BASE  = 40,  /* Thresholds Laser/TEC  */

    /* Constantes de Calibração / Recursos Avançados (56-91) */
    A2_CAL_CONST_OR_ENHANCED = 56, /* Constantes ou Recursos Melhorados */
    A2_MAX_PWR_CONSUMPTION   = 66, /* Consumo máximo (LSB=0.1W) se bit A0.64.6=1 */

    A2_CC_DMI                = 95, /* Checksum bytes 0-94  */

    /* Dados em Tempo Real (96-109) [38, 39] */
    A2_TEMP_CURR             = 96,  /* Temperatura interna (q8.8) */
    A2_VCC_CURR              = 98,  /* Tensão de alimentação (LSB=100uV) */
    A2_TX_BIAS_CURR          = 100, /* Corrente de Bias do Laser  */
    A2_TX_POWER_CURR         = 102, /* Potência de transmissão  */
    A2_RX_POWER_CURR         = 104, /* Potência recebida */
    A2_OPT_LASER_TEMP_WAVE   = 106, /* Temperatura/Wavelength do Laser  */
    A2_OPT_TEC_CURR          = 108, /* Corrente TEC */

    /* Status e Controle [39-42] */
    STATUS_CONTROL        	     = 110, /* Bits de status e controle soft  */
    A2_ALARM_FLAGS           = 112, /* Flags de Alarme */
    A2_TX_INPUT_EQ_CTRL      = 114, /* Controle de Equalização de Entrada */
    A2_RX_OUT_EMPH_CTRL      = 115, /* Controle de Ênfase de Saída  */
    A2_WARNING_FLAGS         = 116, /* Flags de Aviso */
    A2_EXT_STATUS_CONTROL    = 118, /* Status/Controle estendido (PAM4/PL4)  */

    A2_PAGE_SELECT           = 127  /* Seletor de Página */
} a2_offset_t;

/*
 * @brief Offsets de Memória para A2h Página 02h (Tunability, RDT, RPM)
 * Conforme Tabelas 12-1 e 12-5 [64-68]
 */
typedef enum {
    A2_P02_FEAT_TUNABILITY   = 128, /* SFP-8690 Advertisement */
    A2_P02_FEAT_ADV          = 129, /* Advertisement RPM/RDT */
    A2_P02_RDT_CTRL          = 130, /* Modo RDT */
    A2_P02_RDT_VALUE         = 131, /* Valor RDT */
    A2_P02_CH_TUNING_START   = 144, /* Canais de sintonia (SFP-8690) */
    A2_P02_RPM_COR_LATCH     = 174, /* Alarme latched do Remote PM */
    A2_P02_RPM_STATUS        = 192, /* Status RPM (Escrita A5h reseta erro)  */
    A2_P02_RPM_ERR_COUNTERS  = 198, /* Contadores de erro RPM */
    A2_P02_RPM_TX_MOD_INDEX  = 211, /* Índice de Modulação/Enable TX RPM */
    A2_P02_RPM_USER_TX_DATA  = 240, /* Envio de dados de usuário RPM */
    A2_P02_RPM_USER_RX_DATA  = 248  /* Recebimento de dados de usuário RPM  */
} a2_p02_offset_t;

/*
 * @brief Offsets de Memória para A2h Página 03h (High Accuracy Timing)
 * Conforme Tabelas 13-1 e 13-3
 */
typedef enum {
    A2_P03_FORMAT_ID         = 128, /* CA1Bh=CALB ou 100Bh=LOOB  */
    A2_P03_VERSION           = 130, /* Versão (01h) */
    A2_P03_CALIB_DATE        = 131, /* Data de calibração  */
    A2_P03_CALIB_ID          = 134, /* ID único de calibração (CUI)  */
    A2_P03_STRATUM           = 140, /* Geração do calibrador */
    A2_P03_NB_LANES          = 150, /* Número de lanes (SFP-8472=1) */
    A2_P03_OP_MODE_ID        = 151, /* ID do modo operacional  */
    A2_P03_AVG_RX_DELAY      = 179, /* Atraso médio RX (ns, q16.16) */
    A2_P03_AVG_TX_DELAY      = 183, /* Atraso médio TX (ns, q16.16) */
    A2_P03_CC_CALIB          = 255  /* Checksum da página 03h  */
} a2_p03_offset_t;

/*
 * @brief Tipos de página suportados no seletor de página (A2h offset 127)
 */
typedef enum {
    PAGE_00 = 0x00, /* Página principal (Diagnósticos) */
    PAGE_01 = 0x01, /* Reservado */
    PAGE_02 = 0x02, /* Tunabilidade, RDT, RPM */
    PAGE_03 = 0x03, /* High Accuracy Timing */
    PAGE_04 = 0x04  /* Reservado para uso futuro */
} page_t;


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
#define TEMP_TO_DEGC(raw)    (((int16_t)(raw)) / 256.0f)   /* q8.8 para °C */
#define VCC_TO_VOLTS(raw)    ((raw) * 0.0001)             /* 100uV/LSB para Volts */
#define TX_BIAS_TO_MA(raw)      ((raw) * 2.0)                /* 2µA/LSB para mA */
#define POWER_TO_UW(raw)     ((raw) * 0.1)                /* 0.1µW/LSB para µW */


#endif /* DEFS_H */
