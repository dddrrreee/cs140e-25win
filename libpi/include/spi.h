#ifndef __SPI_H__
#define __SPI_H__
// right now we just specialize to the main SPI0 rpi interface; trivial to
// add spi1, spi2.

// pass either of these for chip select.
#define SPI_CE0 1111
#define SPI_CE1 2222

// clock divider should be a power of two less than 2^15
void spi_init(unsigned chip_select, unsigned clock_divider);

// tx and rx must be the same size; easy to have dedicated rx or tx.
int spi_transfer(uint8_t rx[], const uint8_t tx[], unsigned nbytes);


// interface that allows use of chip 1 + chip 0
typedef struct {
    unsigned chip;
    unsigned div;

    unsigned mosi,miso,clk,ce;
} spi_t;

int spi_n_transfer(spi_t s, uint8_t rx[], const uint8_t tx[], unsigned nbytes);
spi_t spi_n_init(unsigned chip_select, unsigned clk_div);


#endif
