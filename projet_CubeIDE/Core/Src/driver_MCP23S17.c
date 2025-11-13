#include "main.h"
// MCP23S17 Definitions
#define MCP23S17_ADDR 0x00 // Assuming hardware address 000 (A2=A1=A0=0)
#define MCP23S17_WRITE (0x40 | (MCP23S17_ADDR << 1)) // Opcode for write
#define MCP23S17_READ (0x41 | (MCP23S17_ADDR << 1)) // Opcode for read

// Registers (BANK=0 mode, default)
#define MCP23S17_IODIRA 0x00
#define MCP23S17_IODIRB 0x01
#define MCP23S17_GPIOA 0x12
#define MCP23S17_GPIOB 0x13
#define MCP23S17_OLATA 0x14 // Optional latch registers
#define MCP23S17_OLATB 0x15
#define MCP23S17_IOCONA 0x0A // Configuration register

// SPI Handle (assuming defined in main or elsewhere)
extern SPI_HandleTypeDef hspi3;

// GPIO Pins
#define MCP_CS_GPIO_PORT GPIOB
#define MCP_CS_PIN GPIO_PIN_7
#define MCP_RESET_GPIO_PORT GPIOA
#define MCP_RESET_PIN GPIO_PIN_0

// Function Prototypes
void MCP23S17_Init(void);
void MCP23S17_WriteRegister(uint8_t reg, uint8_t value);
uint8_t MCP23S17_ReadRegister(uint8_t reg);
void MCP23S17_SetAllPinsHigh(void);

// Initialize MCP23S17
void MCP23S17_Init(void) {
// Set CS high initially
HAL_GPIO_WritePin(MCP_CS_GPIO_PORT, MCP_CS_PIN, GPIO_PIN_SET);

// Reset MCP23S17: pull low for 1ms, then high
HAL_GPIO_WritePin(MCP_RESET_GPIO_PORT, MCP_RESET_PIN, GPIO_PIN_RESET);
HAL_Delay(1);
HAL_GPIO_WritePin(MCP_RESET_GPIO_PORT, MCP_RESET_PIN, GPIO_PIN_SET);
HAL_Delay(1);

// Configure IOCON: Sequential operation disabled, etc. (default is fine, but set explicitly)
MCP23S17_WriteRegister(MCP23S17_IOCONA, 0x20); // BANK=0, SEQOP=0, etc.

// Set all pins as outputs
MCP23S17_WriteRegister(MCP23S17_IODIRA, 0x00); // All GPIOA outputs
MCP23S17_WriteRegister(MCP23S17_IODIRB, 0x00); // All GPIOB outputs
}

// Write to a register
void MCP23S17_WriteRegister(uint8_t reg, uint8_t value) {
uint8_t txData[3] = {MCP23S17_WRITE, reg, value};

HAL_GPIO_WritePin(MCP_CS_GPIO_PORT, MCP_CS_PIN, GPIO_PIN_RESET);
HAL_SPI_Transmit(&hspi3, txData, 3, HAL_MAX_DELAY);
HAL_GPIO_WritePin(MCP_CS_GPIO_PORT, MCP_CS_PIN, GPIO_PIN_SET);
}

// Read from a register (optional, for verification)
uint8_t MCP23S17_ReadRegister(uint8_t reg) {
uint8_t txData[3] = {MCP23S17_READ, reg, 0x00};
uint8_t rxData[3];

HAL_GPIO_WritePin(MCP_CS_GPIO_PORT, MCP_CS_PIN, GPIO_PIN_RESET);
HAL_SPI_TransmitReceive(&hspi3, txData, rxData, 3, HAL_MAX_DELAY);
HAL_GPIO_WritePin(MCP_CS_GPIO_PORT, MCP_CS_PIN, GPIO_PIN_SET);

return rxData[2];
}

// Set all pins to high
void MCP23S17_SetAllPinsHigh(void) {
MCP23S17_WriteRegister(MCP23S17_GPIOA, 0x1); // Set all GPIOA high
MCP23S17_WriteRegister(MCP23S17_GPIOB, 0x1); // Set all GPIOB high
// Alternatively, use OLAT if needed: MCP23S17_WriteRegister(MCP23S17_OLATA, 0xFF);
}

// In your main function, after HAL_Init() and SPI3 init:
// MCP23S17_Init();
// Then call MCP23S17_SetAllPinsHigh() when needed.