#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <unistd.h>

/*
Header file giving access to generic spi transfers
*/

int spi_initialize(int* fd, char* device, uint8_t spi_mode, uint8_t bits_per_word, uint32_t speed_hz);
int spi_transfer(int fd, uint8_t *tx, uint8_t *rx, size_t length);
void spi_disable(int* fd);

#endif // SPI_H