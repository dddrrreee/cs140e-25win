#include "rpi.h"
#include "spi.h"
#include "cycle-util.h"

enum { 
    // hack: for the moment, leave the same
    chip_0_mosi     = 10,
    chip_0_miso     = 9,
    chip_0_clk      = 11,
    chip_0_ce       = 8,

    chip_1_mosi     = chip_0_mosi,
    chip_1_miso     = chip_0_miso,
    chip_1_clk      = chip_0_clk,
    chip_1_ce       = 7,

    SPI_SCLK_LOW_TIME = 200,
    SPI_SCLK_HIGH_TIME = 200
};

#if 0
spi_t spi_n_init(unsigned chip_select, unsigned clock_divider) {
    spi_t s;

    switch(chip_select) {
    case 0:
        s = (spi_t) { 
                .chip = 0,
                .mosi = chip_0_mosi, 
                .miso = chip_0_miso, 
                .clk = chip_0_clk, 
                .ce = chip_0_ce
        };
        break;
    case 1:
        s = (spi_t) { 
                .chip = 1,
                .mosi = chip_1_mosi, 
                .miso = chip_1_miso, 
                .clk = chip_1_clk, 
                .ce = chip_1_ce
        };
        break;

    default: panic("unhandled\n");
    }
    gpio_set_output(s.mosi);
    gpio_set_input(s.miso);
    gpio_set_output(s.clk);
    gpio_set_output(s.ce);
    gpio_write(s.ce,1);

    return s;
}
#endif

spi_t sw_spi_init(spi_t s) {
    gpio_set_output(s.mosi);
    gpio_set_input(s.miso);
    gpio_set_output(s.clk);
    gpio_set_output(s.ce);
    gpio_write(s.ce,1);
    return s;
}

// initialize with the defaults.
spi_t spi_n_init(unsigned chip_select, unsigned clk_div) {
    return sw_spi_init(spi_mk(chip_select, clk_div));
}


static inline void wait_ncycles(uint32_t n) {
    uint32_t s = cycle_cnt_read();
    delay_ncycles(s, n);
}
static inline uint8_t xfer_byte(spi_t s, uint8_t b) {
    uint8_t byte = 0;

    for(int i = 7; i >= 0; i--) {
        gpio_write(s.mosi, (b >> i) &1);

        wait_ncycles(SPI_SCLK_LOW_TIME);

        gpio_write(s.clk, 1);


        byte |= gpio_read(s.miso) << i;

        wait_ncycles(SPI_SCLK_HIGH_TIME);

        gpio_write(s.clk, 0);
    }
    return byte;
}

int spi_n_transfer(spi_t s, uint8_t rx[], const uint8_t tx[], unsigned nbytes) {
    gpio_write(s.ce,0);
    for(unsigned i = 0; i < nbytes; i++) 
        rx[i] = xfer_byte(s,tx[i]);
    gpio_write(s.ce,1);
    return 1;
}
