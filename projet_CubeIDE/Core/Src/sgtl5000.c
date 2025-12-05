/*
 * sgtl5000.c
 *
 * Author:  Antle
 */

#include "sgtl5000.h"
#include "cmsis_os.h"

// --- Valeurs de Configuration (Privées au .c) ---
// Ces valeurs sont basées sur l'analyse de la datasheet (VDDA 1.8V, VDDIO 3.3V, 48kHz, Master)

#define VAL_LINREG_1V2         0x0008 // VDDD = 1.2V
#define VAL_ANA_PWR_STARTUP    0x7260 // Démarrage régulateur interne
#define VAL_REF_CTRL_VDDA_2    0x004E // VAG=0.9V & Bias -50%
#define VAL_LO_CTRL_VDDIO_2    0x0322 // LO_VAG=1.65V & Bias 0.36mA
#define VAL_SHORT_CTRL_75MA    0x1106 // Protection court-circuit 75mA
#define VAL_ANA_CTRL_MUTE      0x0133 // Init: Mute ON, ZCD ON
#define VAL_ANA_PWR_ENABLE     0x6AFF // Power Up: LineOut, HP, ADC, DAC
#define VAL_DIG_PWR_ENABLE     0x0073 // Power Up: I2S, DAP, DAC, ADC
#define VAL_CLK_48KHZ          0x0008 // Fs=48kHz, MCLK=256*Fs
#define VAL_I2S_MASTER         0x0080 // I2S Master Mode, SCLK 64*Fs
#define VAL_LO_VOL_0DB         0x0505 // LineOut Vol (Normalisé)
#define VAL_DAC_VOL_0DB        0x3C3C // DAC Vol 0dB
#define VAL_ADCDAC_UNMUTE      0x0000 // Unmute DAC
#define VAL_ANA_CTRL_ACTIVE    0x0022 // Final: Unmute HP/ADC, Select DAC

// --- Implémentation des fonctions ---

HAL_StatusTypeDef sgtl5000_i2c_read_register(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t * data)
{
	HAL_StatusTypeDef ret;
	uint8_t buffer[2];

	// Envoi de l'adresse du registre (16 bits, Big Endian)
	// Note: I2C_MEMADD_SIZE_16BIT gère l'envoi de l'adresse en 2 octets
	ret = HAL_I2C_Mem_Read(
			h_sgtl5000->hi2c,
			h_sgtl5000->dev_address,
			(uint16_t)reg_address,
			I2C_MEMADD_SIZE_16BIT,
			buffer,
			2,
			HAL_MAX_DELAY
	);

	if (ret == HAL_OK) {
		// Reconstitution de la donnée (Big Endian -> Little Endian pour le MCU)
		*data = (buffer[0] << 8) | buffer[1];
	}

	return ret;
}

HAL_StatusTypeDef sgtl5000_i2c_write_register(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t data)
{
	uint8_t buffer[2];

	// Préparation de la donnée (Little Endian -> Big Endian pour le SGTL)
	buffer[0] = (data >> 8) & 0xFF;
	buffer[1] = data & 0xFF;

	return HAL_I2C_Mem_Write(
			h_sgtl5000->hi2c,
			h_sgtl5000->dev_address,
			(uint16_t)reg_address,
			I2C_MEMADD_SIZE_16BIT,
			buffer,
			2,
			HAL_MAX_DELAY
	);
}

HAL_StatusTypeDef sgtl5000_i2c_set_bit(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t set_mask)
{
	HAL_StatusTypeDef ret;
	uint16_t val;

	ret = sgtl5000_i2c_read_register(h_sgtl5000, reg_address, &val);
	if (ret != HAL_OK) return ret;

	val |= set_mask;

	return sgtl5000_i2c_write_register(h_sgtl5000, reg_address, val);
}

HAL_StatusTypeDef sgtl5000_i2c_clear_bit(h_sgtl5000_t * h_sgtl5000, sgtl5000_registers_t reg_address, uint16_t clear_mask)
{
	HAL_StatusTypeDef ret;
	uint16_t val;

	ret = sgtl5000_i2c_read_register(h_sgtl5000, reg_address, &val);
	if (ret != HAL_OK) return ret;

	val &= ~clear_mask;

	return sgtl5000_i2c_write_register(h_sgtl5000, reg_address, val);
}

HAL_StatusTypeDef sgtl5000_init(h_sgtl5000_t * h_sgtl5000)
{
	HAL_StatusTypeDef ret = HAL_OK;

	// 1. Power Supply Configuration
	// Configure VDDD 1.2V
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_LINREG_CTRL, VAL_LINREG_1V2);
	// Power up internal regulator [cite: 1040]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_ANA_POWER, VAL_ANA_PWR_STARTUP);

	// Délai nécessaire pour la stabilisation de l'alimentation (FreeRTOS)
	osDelay(10);

	// 2. Reference Voltage and Bias
	// VDDA/2 et bias optimisé [cite: 1040]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_REF_CTRL, VAL_REF_CTRL_VDDA_2);
	// LineOut Reference [cite: 1040]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_LINE_OUT_CTRL, VAL_LO_CTRL_VDDIO_2);

	// 3. Other Analog Block Configurations
	// Short Detect [cite: 1040]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_SHORT_CTRL, VAL_SHORT_CTRL_75MA);
	// Init Ana Ctrl (Mute ON, ZCD ON) [cite: 1050]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_ANA_CTRL, VAL_ANA_CTRL_MUTE);

	// 4. Power up Inputs/Outputs/Digital Blocks
	// Power up Analog Blocks [cite: 1053]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_ANA_POWER, VAL_ANA_PWR_ENABLE);
	// Power up Digital Blocks [cite: 1059]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_DIG_POWER, VAL_DIG_PWR_ENABLE);

	// 5. System MCLK and Sample Clock
	// 48 kHz [cite: 1070]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_CLK_CTRL, VAL_CLK_48KHZ);
	// I2S Master Mode [cite: 1072]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_I2S_CTRL, VAL_I2S_MASTER);

	// 6. Niveaux et Volumes
	// LineOut Volume [cite: 1066]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_LINE_OUT_VOL, VAL_LO_VOL_0DB);
	// DAC Volume 0dB [cite: 1203]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_DAC_VOL, VAL_DAC_VOL_0DB);
	// Unmute DAC Soft Mute [cite: 1204]
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_ADCDAC_CTRL, VAL_ADCDAC_UNMUTE);

	// 7. Unmute Final
	// Activation finale des sorties (HP/ADC Unmute, DAC Select)
	ret |= sgtl5000_i2c_write_register(h_sgtl5000, SGTL5000_CHIP_ANA_CTRL, VAL_ANA_CTRL_ACTIVE);

	return ret;
}
