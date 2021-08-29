#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>
#include "rfm69.h"

void rfm69_write(FILE *output, uint8_t address, uint8_t data)
{
    fprintf(output, "%d\t%d\n", address, data);
}


int rfm69_init(FILE *output)
{

//	RFM69 initialization
	rfm69_write(output, REGOPMODE, REGOPMODE_DEF | STBY_MODE);
	rfm69_write(output, REGDATAMODUL, REGDATAMODUL_DEF);

	rfm69_write(output, REGFDEVMSB, REGFDEVMSB_DEF);
	rfm69_write(output, REGFDEVLSB, REGFDEVLSB_DEF);

	rfm69_write(output, REGBITRATEMSB, REGBITRATEMSB_DEF);
	rfm69_write(output, REGBITRATELSB, REGBITRATELSB_DEF);

	rfm69_write(output, REGFRFMSB, REGFRFMSB_DEF);
	rfm69_write(output, REGFRFMID, REGFRFMID_DEF);
	rfm69_write(output, REGFRFLSB, REGFRFLSB_DEF);

	rfm69_write(output, REGAFCCTRL, REGAFCCTRL_DEF);

//	rfm69_write(output, REGLISTEN1, REGLISTEN1_DEF);
//	rfm69_write(output, REGLISTEN2, REGLISTEN2_DEF);
//	rfm69_write(output, REGLISTEN3, REGLISTEN3_DEF);

	rfm69_write(output, REGPALEVEL, REGPALEVEL_DEF);
	rfm69_write(output, REGPARAMP, REGPARAMP_DEF);
	rfm69_write(output, REGOCP, REGOCP_DEF);
	rfm69_write(output, REGLNA, REGLNA_DEF);

	rfm69_write(output, REGRXBW, REGRXBW_DEF);
	rfm69_write(output, REGAFCBW, REGAFCBW_DEF);

//	rfm69_write(output, REGOOKPEAK, REGOOKPEAK_DEF);
//	rfm69_write(output, REGOOKAVG, REGOOKAVG_DEF);
//	rfm69_write(output, REGOOKFIX, REGOOKFIX_DEF);

	rfm69_write(output, REGAFCFEI, REGAFCFEI_DEF);

	rfm69_write(output, REGDIOMAPPING1, REGDIOMAPPING1_DEF);
	rfm69_write(output, REGDIOMAPPING2, REGDIOMAPPING2_DEF);

	rfm69_write(output, REGRSSITHRESH, REGRSSITHRESH_DEF);

	rfm69_write(output, REGPREAMBLEMSB, REGPREAMBLEMSB_DEF);
	rfm69_write(output, REGPREAMBLELSB, REGPREAMBLELSB_DEF);

	rfm69_write(output, REGSYNCCONFIG, REGSYNCCONFIG_DEF);
	rfm69_write(output, REGSYNCVALUE1, REGSYNCVALUE1_DEF);
	rfm69_write(output, REGSYNCVALUE2, REGSYNCVALUE2_DEF);
	rfm69_write(output, REGSYNCVALUE3, REGSYNCVALUE3_DEF);
	rfm69_write(output, REGSYNCVALUE4, REGSYNCVALUE4_DEF);
	rfm69_write(output, REGSYNCVALUE5, REGSYNCVALUE5_DEF);
	rfm69_write(output, REGSYNCVALUE6, REGSYNCVALUE6_DEF);
	rfm69_write(output, REGSYNCVALUE7, REGSYNCVALUE7_DEF);
	rfm69_write(output, REGSYNCVALUE8, REGSYNCVALUE8_DEF);

	rfm69_write(output, REGPACKETCONFIG1, REGPACKETCONFIG1_DEF);
	rfm69_write(output, REGPAYLOADLENGHT, REGPAYLOADLENGHT_DEF);
	rfm69_write(output, REGNODEADRS, REGNODEADRS_DEF);
	rfm69_write(output, REGBROADCASTADRS, REGBROADCASTADRS_DEF);

	rfm69_write(output, REGAUTOMODES, REGAUTOMODES_DEF);

	rfm69_write(output, REGFIFOTHRES, REGFIFOTHRES_DEF);
	rfm69_write(output, REGPACKETCONFIG2, REGPACKETCONFIG2_DEF);

//	rfm69_write(output, REGAESKEY1, REGAESKEY1_DEF);
//	rfm69_write(output, REGAESKEY2, REGAESKEY2_DEF);
//	rfm69_write(output, REGAESKEY3, REGAESKEY3_DEF);
//	rfm69_write(output, REGAESKEY4, REGAESKEY4_DEF);
//	rfm69_write(output, REGAESKEY5, REGAESKEY5_DEF);
//	rfm69_write(output, REGAESKEY6, REGAESKEY6_DEF);
//	rfm69_write(output, REGAESKEY7, REGAESKEY7_DEF);
//	rfm69_write(output, REGAESKEY8, REGAESKEY8_DEF);
//	rfm69_write(output, REGAESKEY9, REGAESKEY9_DEF);
//	rfm69_write(output, REGAESKEY10, REGAESKEY10_DEF);
//	rfm69_write(output, REGAESKEY11, REGAESKEY11_DEF);
//	rfm69_write(output, REGAESKEY12, REGAESKEY12_DEF);
//	rfm69_write(output, REGAESKEY13, REGAESKEY13_DEF);
//	rfm69_write(output, REGAESKEY14, REGAESKEY14_DEF);
//	rfm69_write(output, REGAESKEY15, REGAESKEY15_DEF);
//	rfm69_write(output, REGAESKEY16, REGAESKEY16_DEF);

	rfm69_write(output, REGTESTDAGC, AFC_LOW_BETA_OFF);

	rfm69_write(output, REGIRQFLAGS2, 1<<FIFOOVERRUN);

	return 0;
}
