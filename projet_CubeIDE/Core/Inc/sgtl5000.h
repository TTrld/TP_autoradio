/*
 * sgtl5000.h
 *
 * Author:  Antle
 */

#ifndef SGTL5000_SGTL5000_H_
#define SGTL5000_SGTL5000_H_

#include "main.h"

// Adresse I2C (7-bit address décalée pour la HAL) [cite: 928]
#define SGTL5000_I2C_ADDR    (0x0A << 1)

// --- Register Map ---
typedef enum sgtl5000_registers_enum
{
    SGTL5000_CHIP_ID                = 0x0000,
    SGTL5000_CHIP_DIG_POWER         = 0x0002,
    SGTL5000_CHIP_CLK_CTRL          = 0x0004,
    SGTL5000_CHIP_I2S_CTRL          = 0x0006,
    SGTL5000_CHIP_SSS_CTRL          = 0x000A,
    SGTL5000_CHIP_ADCDAC_CTRL       = 0x000E,
    SGTL5000_CHIP_DAC_VOL           = 0x0010,
    SGTL5000_CHIP_PAD_STRENGTH      = 0x0014,
    SGTL5000_CHIP_ANA_ADC_CTRL      = 0x0020,
    SGTL5000_CHIP_ANA_HP_CTRL       = 0x0022,
    SGTL5000_CHIP_ANA_CTRL          = 0x0024,
    SGTL5000_CHIP_LINREG_CTRL       = 0x0026,
    SGTL5000_CHIP_REF_CTRL          = 0x0028,
    SGTL5000_CHIP_MIC_CTRL          = 0x002A,
    SGTL5000_CHIP_LINE_OUT_CTRL     = 0x002C,
    SGTL5000_CHIP_LINE_OUT_VOL      = 0x002E,
    SGTL5000_CHIP_ANA_POWER         = 0x0030,
    SGTL5000_CHIP_PLL_CTRL          = 0x0032,
    SGTL5000_CHIP_CLK_TOP_CTRL      = 0x0034,
    SGTL5000_SHIP_ANA_STATUS        = 0x0036,
    SGTL5000_CHIP_ANA_TEST1         = 0x0038,
    SGTL5000_CHIP_ANA_TEST2         = 0x003A,
    SGTL5000_CHIP_SHORT_CTRL        = 0x003C,
    SGTL5000_DAP_CONTROL            = 0x0100,
    SGTL5000_DAP_PEQ                = 0x0102,
    SGTL5000_DAP_BASS_ENHANCE       = 0x0104,
    SGTL5000_DAP_BASS_ENHANCE_CTRL  = 0x0106,
    SGTL5000_DAP_AUDIO_EQ           = 0x0108,
    SGTL5000_DAP_SGTL_SURROUND      = 0x010A,
    SGTL5000_DAP_FILTER_COEF_ACCESS = 0x010C,
    SGTL5000_DAP_COEF_WR_B0_MSB     = 0x010E,
    SGTL5000_DAP_COEF_WR_B0_LSB     = 0x0110,
    // ... Autres registres DAP ...
    SGTL5000_DAP_MAIN_CHAN          = 0x0120,
    SGTL5000_DAP_MIX_CHAN           = 0x0122,
    SGTL5000_DAP_AVC_CTRL           = 0x0124
} sgtl5000_registers_t;

// --- Structure Handle ---
typedef struct h_sgtl5000_struct
{
    I2C_HandleTypeDef * hi2c;
    uint16_t dev_address;
} h_sgtl5000_t;

// --- Valeurs de Configuration (Exposées pour référence ou debug) ---
// Note: Ces valeurs sont utilisées en interne par sgtl5000.c mais peuvent être utiles ici.

// Alimentation
#define VAL_LINREG_1V2         0x0008
#define VAL_ANA_PWR_STARTUP    0x7260

// Références
#define VAL_REF_CTRL_VDDA_2    0x004E
#define VAL_LO_CTRL_VDDIO_2    0x0322

// Config Analogique
#define VAL_SHORT_CTRL_75MA    0x1106
#define VAL_ANA_CTRL_MUTE      0x0133
#define VAL_ANA_PWR_ENABLE     0x6AFF

// Numérique & Horloges
#define VAL_DIG_PWR_ENABLE     0x0073
#define VAL_CLK_48KHZ          0x0008
#define VAL_I2S_MASTER         0x0080

// Volumes & Final
#define VAL_LO_VOL_0DB         0x0505
#define VAL_DAC_VOL_0DB        0x3C3C
#define VAL_ADCDAC_UNMUTE      0x0000
#define VAL_ANA_CTRL_ACTIVE    0x0022

// --- Prototypes ---
HAL_StatusTypeDef sgtl5000_init(h_sgtl5000_t * h_sgtl5000);

// Fonctions bas niveau (utiles si vous voulez manipuler les registres manuellement ailleurs)
HAL_StatusTypeDef sgtl5000_i2c_read_register(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t * data);
HAL_StatusTypeDef sgtl5000_i2c_write_register(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t data);
HAL_StatusTypeDef sgtl5000_i2c_set_bit(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t set_mask);
HAL_StatusTypeDef sgtl5000_i2c_clear_bit(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t clear_mask);

#endif /* SGTL5000_SGTL5000_H_ */
