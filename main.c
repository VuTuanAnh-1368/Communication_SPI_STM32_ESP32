#include "stm32f10x.h"

void SPI1_Init() ;
void SPI1_EnableSS();
void SPI1_TransmitString(const char *string);

int main() {
    SPI1_Init();
    while (1) {
        SPI1_TransmitString("hello esp");
        for (volatile int i = 0; i < 1000000; i++); 
    }
}

void SPI1_Init() {
    // Enable clock for GPIOA and SPI1
    RCC->APB2ENR |= (1 << 2) | (1 << 12);  // APB2ENR_IOPAEN = bit 2, APB2ENR_SPI1EN = bit 12

    // Configure GPIOA for SPI1 pins
    // Clear configuration first
    GPIOA->CRL &= ~((0xF << (4 * 4)) | (0xF << (5 * 4)) | (0xF << (6 * 4)) | (0xF << (7 * 4)));
    // Set new configuration
    GPIOA->CRL |= (0x3 << (4 * 4)) |    // PA4 as General purpose output push-pull (SS)
                  (0x9 << (5 * 4)) |    // PA5 as Alternate function output Push-pull (SCK)
                  (0x4 << (6 * 4)) |    // PA6 as Floating input (MISO)
                  (0x9 << (7 * 4));     // PA7 as Alternate function output Push-pull (MOSI)

    // Pull SS high to deselect the slave
    GPIOA->BSRR = (1 << 4);

    // SPI1 configuration: Master mode, CPOL=0, CPHA=0, 8-bit data frame, baud rate = fPCLK/256
    SPI1->CR1 = (1 << 2) |    // SPI_CR1_MSTR = bit 2
                (7 << 3) |    // SPI_CR1_BR = bits 3,4,5 set to 111 for fPCLK/256
                (1 << 9) |    // SPI_CR1_SSM = bit 9
                (1 << 8);     // SPI_CR1_SSI = bit 8

    // Enable SPI1
    SPI1->CR1 |= (1 << 6);    // SPI_CR1_SPE = bit 6
}

void SPI1_EnableSS() {
    // Pull SS low to select the slave
    GPIOA->BSRR = (1 << (4 + 16));  // BSRR reset bit for PA4
}

void SPI1_DisableSS() {
    // Pull SS high to deselect the slave
    GPIOA->BSRR = (1 << 4);  // BSRR set bit for PA4
}

void SPI1_Transmit(uint8_t data) {
    // Wait until transmit buffer is empty
    while (!(SPI1->SR & (1 << 1))); // SPI_SR_TXE = bit 1
    // Send the byte
    SPI1->DR = data;
    // Wait until transmission is complete
    while (!(SPI1->SR & (1 << 0))); // SPI_SR_RXNE = bit 0
    // Wait until SPI is not busy
    while (SPI1->SR & (1 << 7));    // SPI_SR_BSY = bit 7
}

void SPI1_TransmitString(const char *string) {
    SPI1_EnableSS();
    while (*string) {
        SPI1_Transmit(*string++);
    }
    SPI1_DisableSS();
}
