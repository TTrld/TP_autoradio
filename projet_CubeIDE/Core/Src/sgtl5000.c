/*
 * sgtl5000.c
 *
 *  Created on: Nov 21, 2025
 *      Author: Florian
 */

#include <stdint.h>
#include "main.h"
#include "i2c.h"

#define SGTL5000_I2C_ADDR   0x14   // Addr HAL

extern I2C_HandleTypeDef hi2c2;

static void SGTL_Write(uint16_t reg, uint16_t val)
{
    uint8_t data[2] = { val >> 8, val & 0xFF };
    HAL_I2C_Mem_Write(&hi2c2, SGTL5000_I2C_ADDR, reg,
                      I2C_MEMADD_SIZE_16BIT, data, 2, 100);
}

void sgtl5000_init(void)
{
    HAL_Delay(50);

    // -------------------------------------------
    // 1) Désactiver linreg interne (puisque VDDD = 1.8V externe)
    // -------------------------------------------
    SGTL_Write(0x0026, 0x006C);

    // -------------------------------------------
    // 2) Régler les références analogiques
    // -------------------------------------------
    SGTL_Write(0x0028, 0x0132);

    // -------------------------------------------
    // 3) Configurer Line-Out pour VDDA=3.3V
    // -------------------------------------------
    SGTL_Write(0x002C, 0x0F22);

    // -------------------------------------------
    // 4) HPF et protections
    // -------------------------------------------
    SGTL_Write(0x003C, 0x1106);

    // -------------------------------------------
    // 5) Contrôles analogiques
    // -------------------------------------------
    SGTL_Write(0x0024, 0x0133);

    // -------------------------------------------
    // 6) Alimenter blocs analogiques
    // -------------------------------------------
    SGTL_Write(0x0030, 0x6AFF);

    // -------------------------------------------
    // 7) Alimenter blocs numériques
    // -------------------------------------------
    SGTL_Write(0x0002, 0x0063);

    // -------------------------------------------
    // 8) Volume Line-Out
    // -------------------------------------------
    SGTL_Write(0x002E, 0x1818);

    // -------------------------------------------
    // 9) Horloge (MCLK = 12.288 MHz)
    // -------------------------------------------
    SGTL_Write(0x0004, 0x0008);

    // -------------------------------------------
    // 10) I2S classique
    // -------------------------------------------
    // CHIP_I2S_CTRL :
    SGTL_Write(0x0006, 0x0130);

    // -------------------------------------------
    // 11) Contrôle ADC/DAC
    // -------------------------------------------
    SGTL_Write(0x000E, 0x0000);

    // -------------------------------------------
    // 12) Volume DAC
    // -------------------------------------------
    SGTL_Write(0x0010, 0x3C3C);
    HAL_Delay(20);
    SGTL_Write(0x0024, 0x0030);
}



