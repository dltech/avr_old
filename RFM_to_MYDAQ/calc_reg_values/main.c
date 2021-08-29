#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>
#include <locale.h>
#include "rfm69.h"

int main()
{
    FILE *output;

    setlocale(LC_ALL, "ENG");

    if( (output = fopen("registers.dat","w")) == NULL ) {
        puts("хренюшка произошла\n");
        return 0;
    }

    rfm69_init(output);

    fclose(output);
    if( (output = fopen("tx_reg.dat","w")) == NULL ) {
        puts("хренюшка произошла\n");
        return 0;
    }

    rfm69_write(output, REGOPMODE, REGOPMODE_DEF | TX_MODE);

	rfm69_write(output, REGFIFO, 0xee);									// packet size to FIFO
	rfm69_write(output, REGFIFO, 0xff);
    rfm69_write(output, REGIRQFLAGS2, 1<<FIFOOVERRUN);

    fclose(output);
    if( (output = fopen("rx_reg.dat","w")) == NULL ) {
        puts("хренюшка произошла\n");
        return 0;
    }

    rfm69_write(output, REGOPMODE, REGOPMODE_DEF | RX_MODE);
    rfm69_write(output, REGIRQFLAGS2, (1<<PAYLOADREADY));

    rfm69_write(output, REGOPMODE, REGOPMODE_DEF | STBY_MODE);

    fclose(output);

    return 0;
}
