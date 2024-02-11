/*****************************************************************************************
 *
 * Frontend core driver
 *
 * Customized for adb_box, bska & bxzb models
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ****************************************************************************************/

#include "core.h"
/* Demodulators */
#include "stb0899_drv.h"
#include "stb0899_reg.h"
#include "stb0899_cfg.h"

/* Tuners */
#include "stb6100.h"
#include "stb6100_cfg.h"

#include <linux/platform_device.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/dvb/dmx.h>
#include <linux/proc_fs.h>
#include <pvr_config.h>

#define I2C_ADDR_STB0899 (0xd0 >> 1)  // d0 -> 0x68
#define I2C_ADDR_STB6100 (0xc0 >> 1)  // c0 -> 0x60

short paramDebug = 0;
static struct core *core[MAX_DVB_ADAPTERS];

static struct stb0899_config stb0899_config;
static struct stb6100_config stb6100_config;

static const struct stb0899_s1_reg stb0899_init_dev [] =
{
	{ STB0899_DISCNTRL1,   0x26 },
	{ STB0899_DISCNTRL2,   0x80 },
	{ STB0899_DISRX_ST0,   0x04 },
	{ STB0899_DISRX_ST1,   0x20 },
	{ STB0899_DISPARITY,   0x00 },
	{ STB0899_DISFIFO,     0x00 },
	{ STB0899_DISF22,      0x99 },
	{ STB0899_DISF22RX,    0x85 },  // 0xa8
	{ STB0899_ACRPRESC,    0x11 },
	{ STB0899_ACRDIV1,     0x0a },
	{ STB0899_ACRDIV2,     0x05 },
	{ STB0899_DACR1,       0x00 },
	{ STB0899_DACR2,       0x00 },
	{ STB0899_OUTCFG,      0x00 },
	{ STB0899_MODECFG,     0x00 },  // Inversion
	{ STB0899_IRQMSK_3,    0xf3 },
	{ STB0899_IRQMSK_2,    0xfc },
	{ STB0899_IRQMSK_1,    0xff },
	{ STB0899_IRQMSK_0,    0xff },
	{ STB0899_I2CCFG,      0x88 },
	{ STB0899_I2CRPT,      0x48 },  // 58 = ok
	{ STB0899_GPIO00CFG,   0x82 },
	{ STB0899_GPIO01CFG,   0x82 },  /* 0x02 -> LED green 0x82 -> LED orange */
	{ STB0899_GPIO02CFG,   0x82 },
	{ STB0899_GPIO03CFG,   0x82 },
	{ STB0899_GPIO04CFG,   0x82 },
	{ STB0899_GPIO05CFG,   0x82 },
	{ STB0899_GPIO06CFG,   0x82 },
	{ STB0899_GPIO07CFG,   0x82 },
	{ STB0899_GPIO08CFG,   0x82 },
	{ STB0899_GPIO09CFG,   0x82 },
	{ STB0899_GPIO10CFG,   0x82 },
	{ STB0899_GPIO11CFG,   0x82 },
	{ STB0899_GPIO12CFG,   0x82 },
	{ STB0899_GPIO13CFG,   0x82 },
	{ STB0899_GPIO14CFG,   0x82 },
	{ STB0899_GPIO15CFG,   0x82 },
	{ STB0899_GPIO16CFG,   0x82 },
	{ STB0899_GPIO17CFG,   0x82 },
	{ STB0899_GPIO18CFG,   0x82 },
	{ STB0899_GPIO19CFG,   0x82 },
	{ STB0899_GPIO20CFG,   0x82 },
	{ STB0899_SDATCFG,     0xb8 },
	{ STB0899_SCLTCFG,     0xba },
	{ STB0899_AGCRFCFG,    0x1c },  // 0x11 DVB-S; 0x1c DVB-S2 (1c, rjkm)
	{ STB0899_GPIO22,      0x82 },
	{ STB0899_GPIO21,      0x91 },
	{ STB0899_DIRCLKCFG,   0x82 },
	{ STB0899_CLKOUT27CFG, 0x7e },
	{ STB0899_STDBYCFG,    0x82 },
	{ STB0899_CS0CFG,      0x82 },
	{ STB0899_CS1CFG,      0x82 },
	{ STB0899_DISEQCOCFG,  0x20 },
	{ STB0899_NCOARSE,     0x15 },  // 0x15 = 27 Mhz Clock, F/3 = 198MHz, F/6 = 108MHz
	{ STB0899_SYNTCTRL,    0x00 },  // 0x00 = CLK from CLKI, 0x02 = CLK from XTALI
	{ STB0899_FILTCTRL,    0x00 },
	{ STB0899_SYSCTRL,     0x00 },
	{ STB0899_STOPCLK1,    0x20 },  // orig: 0x00 budget-ci: 0x20
	{ STB0899_STOPCLK2,    0x00 },
	{ STB0899_INTBUFCTRL,  0x0a },
	{ STB0899_AGC2I1,      0x00 },
	{ STB0899_AGC2I2,      0x00 },
	{ STB0899_AGCIQIN,     0x00 },
	{ STB0899_TSTRES,      0x40 },  //rjkm
	{ 0xffff,              0xff },
};

static const struct stb0899_s2_reg stb0899_init_s2_demod[] =
{
	{ STB0899_OFF0_DMD_STATUS,         STB0899_BASE_DMD_STATUS,        0x00000103 }, /* DMDSTATUS      */
	{ STB0899_OFF0_CRL_FREQ,           STB0899_BASE_CRL_FREQ,          0x3ed1da56 }, /* CRLFREQ        */
	{ STB0899_OFF0_BTR_FREQ,           STB0899_BASE_BTR_FREQ,          0x00004000 }, /* BTRFREQ        */
	{ STB0899_OFF0_IF_AGC_GAIN,        STB0899_BASE_IF_AGC_GAIN,       0x00002ade }, /* IFAGCGAIN      */
	{ STB0899_OFF0_BB_AGC_GAIN,        STB0899_BASE_BB_AGC_GAIN,       0x000001bc }, /* BBAGCGAIN      */
	{ STB0899_OFF0_DC_OFFSET,          STB0899_BASE_DC_OFFSET,         0x00000200 }, /* DCOFFSET       */
	{ STB0899_OFF0_DMD_CNTRL,          STB0899_BASE_DMD_CNTRL,         0x0000000f }, /* DMDCNTRL       */

	{ STB0899_OFF0_IF_AGC_CNTRL,       STB0899_BASE_IF_AGC_CNTRL,      0x03fb4a20 }, /* IFAGCCNTRL     */

//	new value 0x00200C17	- the quality of the received HD channels improves
//	old value 0x00200c97
//	{ STB0899_OFF0_BB_AGC_CNTRL,       STB0899_BASE_BB_AGC_CNTRL,      0x00200c97 }, /* BBAGCCNTRL     */
	{ STB0899_OFF0_BB_AGC_CNTRL,       STB0899_BASE_BB_AGC_CNTRL,      0x00200C17 }, /* BBAGCCNTRL     */

	{ STB0899_OFF0_CRL_CNTRL,          STB0899_BASE_CRL_CNTRL,         0x00000016 }, /* CRLCNTRL       */
	{ STB0899_OFF0_CRL_PHS_INIT,       STB0899_BASE_CRL_PHS_INIT,      0x00000000 }, /* CRLPHSINIT     */
	{ STB0899_OFF0_CRL_FREQ_INIT,      STB0899_BASE_CRL_FREQ_INIT,     0x00000000 }, /* CRLFREQINIT    */
	{ STB0899_OFF0_CRL_LOOP_GAIN,      STB0899_BASE_CRL_LOOP_GAIN,     0x00000000 }, /* CRLLOOPGAIN    */
	{ STB0899_OFF0_CRL_NOM_FREQ,       STB0899_BASE_CRL_NOM_FREQ,      0x3ed097b6 }, /* CRLNOMFREQ     */
	{ STB0899_OFF0_CRL_SWP_RATE,       STB0899_BASE_CRL_SWP_RATE,      0x00000000 }, /* CRLSWPRATE     */
	{ STB0899_OFF0_CRL_MAX_SWP,        STB0899_BASE_CRL_MAX_SWP,       0x00000000 }, /* CRLMAXSWP      */
	{ STB0899_OFF0_CRL_LK_CNTRL,       STB0899_BASE_CRL_LK_CNTRL,      0x0f6cdc01 }, /* CRLLKCNTRL     */
	{ STB0899_OFF0_DECIM_CNTRL,        STB0899_BASE_DECIM_CNTRL,       0x00000000 }, /* DECIMCNTRL     */
	{ STB0899_OFF0_BTR_CNTRL,          STB0899_BASE_BTR_CNTRL,         0x00003993 }, /* BTRCNTRL       */
	{ STB0899_OFF0_BTR_LOOP_GAIN,      STB0899_BASE_BTR_LOOP_GAIN,     0x000d3c6f }, /* BTRLOOPGAIN    */
	{ STB0899_OFF0_BTR_PHS_INIT,       STB0899_BASE_BTR_PHS_INIT,      0x00000000 }, /* BTRPHSINIT     */
	{ STB0899_OFF0_BTR_FREQ_INIT,      STB0899_BASE_BTR_FREQ_INIT,     0x00000000 }, /* BTRFREQINIT    */
	{ STB0899_OFF0_BTR_NOM_FREQ,       STB0899_BASE_BTR_NOM_FREQ,      0x0238e38e }, /* BTRNOMFREQ     */
	{ STB0899_OFF0_BTR_LK_CNTRL,       STB0899_BASE_BTR_LK_CNTRL,      0x00000000 }, /* BTRLKCNTRL     */
	{ STB0899_OFF0_DECN_CNTRL,         STB0899_BASE_DECN_CNTRL,        0x00000000 }, /* DECNCNTRL      */
	{ STB0899_OFF0_TP_CNTRL,           STB0899_BASE_TP_CNTRL,          0x00000000 }, /* TPCNTRL        */
	{ STB0899_OFF0_TP_BUF_STATUS,      STB0899_BASE_TP_BUF_STATUS,     0x00000000 }, /* TPBUFSTATUS    */
	{ STB0899_OFF0_DC_ESTIM,           STB0899_BASE_DC_ESTIM,          0x00000000 }, /* DCESTIM        */
	{ STB0899_OFF0_FLL_CNTRL,          STB0899_BASE_FLL_CNTRL,         0x00000000 }, /* FLLCNTRL       */
	{ STB0899_OFF0_FLL_FREQ_WD,        STB0899_BASE_FLL_FREQ_WD,       0x40070000 }, /* FLLFREQWD      */
	{ STB0899_OFF0_ANTI_ALIAS_SEL,     STB0899_BASE_ANTI_ALIAS_SEL,    0x00000001 }, /* ANTIALIASSEL   */
	{ STB0899_OFF0_RRC_ALPHA,          STB0899_BASE_RRC_ALPHA,         0x00000002 }, /* RRCALPHA       */
	{ STB0899_OFF0_DC_ADAPT_LSHFT,     STB0899_BASE_DC_ADAPT_LSHFT,    0x00000000 }, /* DCADAPTISHFT   */
	{ STB0899_OFF0_IMB_OFFSET,         STB0899_BASE_IMB_OFFSET,        0x0000fe01 }, /* IMBOFFSET      */
	{ STB0899_OFF0_IMB_ESTIMATE,       STB0899_BASE_IMB_ESTIMATE,      0x00000000 }, /* IMBESTIMATE    */
	{ STB0899_OFF0_IMB_CNTRL,          STB0899_BASE_IMB_CNTRL,         0x00000001 }, /* IMBCNTRL       */
	{ STB0899_OFF0_IF_AGC_CNTRL2,      STB0899_BASE_IF_AGC_CNTRL2,     0x00005007 }, /* IFAGCCNTRL2    */
	{ STB0899_OFF0_DMD_CNTRL2,         STB0899_BASE_DMD_CNTRL2,        0x00000002 }, /* DMDCNTRL2      */
	{ STB0899_OFF0_TP_BUFFER,          STB0899_BASE_TP_BUFFER,         0x00000000 }, /* TPBUFFER       */
	{ STB0899_OFF0_TP_BUFFER1,         STB0899_BASE_TP_BUFFER1,        0x00000000 }, /* TPBUFFER1      */
	{ STB0899_OFF0_TP_BUFFER2,         STB0899_BASE_TP_BUFFER2,        0x00000000 }, /* TPBUFFER2      */
	{ STB0899_OFF0_TP_BUFFER3,         STB0899_BASE_TP_BUFFER3,        0x00000000 }, /* TPBUFFER3      */
	{ STB0899_OFF0_TP_BUFFER4,         STB0899_BASE_TP_BUFFER4,        0x00000000 }, /* TPBUFFER4      */
	{ STB0899_OFF0_TP_BUFFER5,         STB0899_BASE_TP_BUFFER5,        0x00000000 }, /* TPBUFFER5      */
	{ STB0899_OFF0_TP_BUFFER6,         STB0899_BASE_TP_BUFFER6,        0x00000000 }, /* TPBUFFER6      */
	{ STB0899_OFF0_TP_BUFFER7,         STB0899_BASE_TP_BUFFER7,        0x00000000 }, /* TPBUFFER7      */
	{ STB0899_OFF0_TP_BUFFER8,         STB0899_BASE_TP_BUFFER8,        0x00000000 }, /* TPBUFFER8      */
	{ STB0899_OFF0_TP_BUFFER9,         STB0899_BASE_TP_BUFFER9,        0x00000000 }, /* TPBUFFER9      */
	{ STB0899_OFF0_TP_BUFFER10,        STB0899_BASE_TP_BUFFER10,       0x00000000 }, /* TPBUFFER10     */
	{ STB0899_OFF0_TP_BUFFER11,        STB0899_BASE_TP_BUFFER11,       0x00000000 }, /* TPBUFFER11     */
	{ STB0899_OFF0_TP_BUFFER12,        STB0899_BASE_TP_BUFFER12,       0x00000000 }, /* TPBUFFER12     */
	{ STB0899_OFF0_TP_BUFFER13,        STB0899_BASE_TP_BUFFER13,       0x00000000 }, /* TPBUFFER13     */
	{ STB0899_OFF0_TP_BUFFER14,        STB0899_BASE_TP_BUFFER14,       0x00000000 }, /* TPBUFFER14     */
	{ STB0899_OFF0_TP_BUFFER15,        STB0899_BASE_TP_BUFFER15,       0x00000000 }, /* TPBUFFER15     */
	{ STB0899_OFF0_TP_BUFFER16,        STB0899_BASE_TP_BUFFER16,       0x0000ff00 }, /* TPBUFFER16     */
	{ STB0899_OFF0_TP_BUFFER17,        STB0899_BASE_TP_BUFFER17,       0x00000100 }, /* TPBUFFER17     */
	{ STB0899_OFF0_TP_BUFFER18,        STB0899_BASE_TP_BUFFER18,       0x0000fe01 }, /* TPBUFFER18     */
	{ STB0899_OFF0_TP_BUFFER19,        STB0899_BASE_TP_BUFFER19,       0x000004fe }, /* TPBUFFER19     */
	{ STB0899_OFF0_TP_BUFFER20,        STB0899_BASE_TP_BUFFER20,       0x0000cfe7 }, /* TPBUFFER20     */
	{ STB0899_OFF0_TP_BUFFER21,        STB0899_BASE_TP_BUFFER21,       0x0000bec6 }, /* TPBUFFER21     */
	{ STB0899_OFF0_TP_BUFFER22,        STB0899_BASE_TP_BUFFER22,       0x0000c2bf }, /* TPBUFFER22     */
	{ STB0899_OFF0_TP_BUFFER23,        STB0899_BASE_TP_BUFFER23,       0x0000c1c1 }, /* TPBUFFER23     */
	{ STB0899_OFF0_TP_BUFFER24,        STB0899_BASE_TP_BUFFER24,       0x0000c1c1 }, /* TPBUFFER24     */
	{ STB0899_OFF0_TP_BUFFER25,        STB0899_BASE_TP_BUFFER25,       0x0000c1c1 }, /* TPBUFFER25     */
	{ STB0899_OFF0_TP_BUFFER26,        STB0899_BASE_TP_BUFFER26,       0x0000c1c1 }, /* TPBUFFER26     */
	{ STB0899_OFF0_TP_BUFFER27,        STB0899_BASE_TP_BUFFER27,       0x0000c1c0 }, /* TPBUFFER27     */
	{ STB0899_OFF0_TP_BUFFER28,        STB0899_BASE_TP_BUFFER28,       0x0000c0c0 }, /* TPBUFFER28     */
	{ STB0899_OFF0_TP_BUFFER29,        STB0899_BASE_TP_BUFFER29,       0x0000c1c1 }, /* TPBUFFER29     */
	{ STB0899_OFF0_TP_BUFFER30,        STB0899_BASE_TP_BUFFER30,       0x0000c1c1 }, /* TPBUFFER30     */
	{ STB0899_OFF0_TP_BUFFER31,        STB0899_BASE_TP_BUFFER31,       0x0000c0c1 }, /* TPBUFFER31     */
	{ STB0899_OFF0_TP_BUFFER32,        STB0899_BASE_TP_BUFFER32,       0x0000c0c1 }, /* TPBUFFER32     */
	{ STB0899_OFF0_TP_BUFFER33,        STB0899_BASE_TP_BUFFER33,       0x0000c1c1 }, /* TPBUFFER33     */
	{ STB0899_OFF0_TP_BUFFER34,        STB0899_BASE_TP_BUFFER34,       0x0000c1c1 }, /* TPBUFFER34     */
	{ STB0899_OFF0_TP_BUFFER35,        STB0899_BASE_TP_BUFFER35,       0x0000c0c1 }, /* TPBUFFER35     */
	{ STB0899_OFF0_TP_BUFFER36,        STB0899_BASE_TP_BUFFER36,       0x0000c1c1 }, /* TPBUFFER36     */
	{ STB0899_OFF0_TP_BUFFER37,        STB0899_BASE_TP_BUFFER37,       0x0000c0c1 }, /* TPBUFFER37     */
	{ STB0899_OFF0_TP_BUFFER38,        STB0899_BASE_TP_BUFFER38,       0x0000c1c1 }, /* TPBUFFER38     */
	{ STB0899_OFF0_TP_BUFFER39,        STB0899_BASE_TP_BUFFER39,       0x0000c0c0 }, /* TPBUFFER39     */
	{ STB0899_OFF0_TP_BUFFER40,        STB0899_BASE_TP_BUFFER40,       0x0000c1c0 }, /* TPBUFFER40     */
	{ STB0899_OFF0_TP_BUFFER41,        STB0899_BASE_TP_BUFFER41,       0x0000c1c1 }, /* TPBUFFER41     */
	{ STB0899_OFF0_TP_BUFFER42,        STB0899_BASE_TP_BUFFER42,       0x0000c0c0 }, /* TPBUFFER42     */
	{ STB0899_OFF0_TP_BUFFER43,        STB0899_BASE_TP_BUFFER43,       0x0000c1c0 }, /* TPBUFFER43     */
	{ STB0899_OFF0_TP_BUFFER44,        STB0899_BASE_TP_BUFFER44,       0x0000c0c1 }, /* TPBUFFER44     */
	{ STB0899_OFF0_TP_BUFFER45,        STB0899_BASE_TP_BUFFER45,       0x0000c1be }, /* TPBUFFER45     */
	{ STB0899_OFF0_TP_BUFFER46,        STB0899_BASE_TP_BUFFER46,       0x0000c1c9 }, /* TPBUFFER46     */
	{ STB0899_OFF0_TP_BUFFER47,        STB0899_BASE_TP_BUFFER47,       0x0000c0da }, /* TPBUFFER47     */
	{ STB0899_OFF0_TP_BUFFER48,        STB0899_BASE_TP_BUFFER48,       0x0000c0ba }, /* TPBUFFER48     */
	{ STB0899_OFF0_TP_BUFFER49,        STB0899_BASE_TP_BUFFER49,       0x0000c1c4 }, /* TPBUFFER49     */
	{ STB0899_OFF0_TP_BUFFER50,        STB0899_BASE_TP_BUFFER50,       0x0000c1bf }, /* TPBUFFER50     */
	{ STB0899_OFF0_TP_BUFFER51,        STB0899_BASE_TP_BUFFER51,       0x0000c0c1 }, /* TPBUFFER51     */
	{ STB0899_OFF0_TP_BUFFER52,        STB0899_BASE_TP_BUFFER52,       0x0000c1c0 }, /* TPBUFFER52     */
	{ STB0899_OFF0_TP_BUFFER53,        STB0899_BASE_TP_BUFFER53,       0x0000c0c1 }, /* TPBUFFER53     */
	{ STB0899_OFF0_TP_BUFFER54,        STB0899_BASE_TP_BUFFER54,       0x0000c1c1 }, /* TPBUFFER54     */
	{ STB0899_OFF0_TP_BUFFER55,        STB0899_BASE_TP_BUFFER55,       0x0000c1c1 }, /* TPBUFFER55     */
	{ STB0899_OFF0_TP_BUFFER56,        STB0899_BASE_TP_BUFFER56,       0x0000c1c1 }, /* TPBUFFER56     */
	{ STB0899_OFF0_TP_BUFFER57,        STB0899_BASE_TP_BUFFER57,       0x0000c1c1 }, /* TPBUFFER57     */
	{ STB0899_OFF0_TP_BUFFER58,        STB0899_BASE_TP_BUFFER58,       0x0000c1c1 }, /* TPBUFFER58     */
	{ STB0899_OFF0_TP_BUFFER59,        STB0899_BASE_TP_BUFFER59,       0x0000c1c1 }, /* TPBUFFER59     */
	{ STB0899_OFF0_TP_BUFFER60,        STB0899_BASE_TP_BUFFER60,       0x0000c1c1 }, /* TPBUFFER60     */
	{ STB0899_OFF0_TP_BUFFER61,        STB0899_BASE_TP_BUFFER61,       0x0000c1c1 }, /* TPBUFFER61     */
	{ STB0899_OFF0_TP_BUFFER62,        STB0899_BASE_TP_BUFFER62,       0x0000c1c1 }, /* TPBUFFER62     */
	{ STB0899_OFF0_TP_BUFFER63,        STB0899_BASE_TP_BUFFER63,       0x0000c1c0 }, /* TPBUFFER63     */
	{ STB0899_OFF0_RESET_CNTRL,        STB0899_BASE_RESET_CNTRL,       0x00000001 }, /* RESETCNTRL     */
	{ STB0899_OFF0_ACM_ENABLE,         STB0899_BASE_ACM_ENABLE,        0x00005654 }, /* ACMENABLE      */
	{ STB0899_OFF0_DESCR_CNTRL,        STB0899_BASE_DESCR_CNTRL,       0x00000000 }, /* DESCRCNTRL     */
	{ STB0899_OFF0_CSM_CNTRL1,         STB0899_BASE_CSM_CNTRL1,        0x00020019 }, /* CSMCNTRL1      */
	{ STB0899_OFF0_CSM_CNTRL2,         STB0899_BASE_CSM_CNTRL2,        0x004b3237 }, /* CSMCNTRL2      */
	{ STB0899_OFF0_CSM_CNTRL3,         STB0899_BASE_CSM_CNTRL3,        0x0003dd17 }, /* CSMCNTRL3      */
	{ STB0899_OFF0_CSM_CNTRL4,         STB0899_BASE_CSM_CNTRL4,        0x00008008 }, /* CSMCNTRL4      */
	{ STB0899_OFF0_UWP_CNTRL1,         STB0899_BASE_UWP_CNTRL1,        0x002a3106 }, /* UWPCNTRL1      */
	{ STB0899_OFF0_UWP_CNTRL2,         STB0899_BASE_UWP_CNTRL2,        0x0006140a }, /* UWPCNTRL2      */
	{ STB0899_OFF0_UWP_STAT1,          STB0899_BASE_UWP_STAT1,         0x00008000 }, /* UWPSTAT1       */
	{ STB0899_OFF0_UWP_STAT2,          STB0899_BASE_UWP_STAT2,         0x00000000 }, /* UWPSTAT2       */
	{ STB0899_OFF0_DMD_STAT2,          STB0899_BASE_DMD_STAT2,         0x00000000 }, /* DMDSTAT2       */
	{ STB0899_OFF0_FREQ_ADJ_SCALE,     STB0899_BASE_FREQ_ADJ_SCALE,    0x00000471 }, /* FREQADJSCALE   */
	{ STB0899_OFF0_UWP_CNTRL3,         STB0899_BASE_UWP_CNTRL3,        0x017b0465 }, /* UWPCNTRL3      */
	{ STB0899_OFF0_SYM_CLK_SEL,        STB0899_BASE_SYM_CLK_SEL,       0x00000002 }, /* SYMCLKSEL      */
	{ STB0899_OFF0_SOF_SRCH_TO,        STB0899_BASE_SOF_SRCH_TO,       0x00196464 }, /* SOFSRCHTO      */
	{ STB0899_OFF0_ACQ_CNTRL1,         STB0899_BASE_ACQ_CNTRL1,        0x00000603 }, /* ACQCNTRL1      */
	{ STB0899_OFF0_ACQ_CNTRL2,         STB0899_BASE_ACQ_CNTRL2,        0x02046666 }, /* ACQCNTRL2      */
	{ STB0899_OFF0_ACQ_CNTRL3,         STB0899_BASE_ACQ_CNTRL3,        0x10046583 }, /* ACQCNTRL3      */
	{ STB0899_OFF0_FE_SETTLE,          STB0899_BASE_FE_SETTLE,         0x00010404 }, /* FESETTLE       */
	{ STB0899_OFF0_AC_DWELL,           STB0899_BASE_AC_DWELL,          0x0002aa8a }, /* ACDWELL        */
	{ STB0899_OFF0_ACQUIRE_TRIG,       STB0899_BASE_ACQUIRE_TRIG,      0x00000000 }, /* ACQUIRETRIG    */
	{ STB0899_OFF0_LOCK_LOST,          STB0899_BASE_LOCK_LOST,         0x00000001 }, /* LOCKLOST       */
	{ STB0899_OFF0_ACQ_STAT1,          STB0899_BASE_ACQ_STAT1,         0x00000500 }, /* ACQSTAT1       */
	{ STB0899_OFF0_ACQ_TIMEOUT,        STB0899_BASE_ACQ_TIMEOUT,       0x0028a0a0 }, /* ACQTIMEOUT     */
	{ STB0899_OFF0_ACQ_TIME,           STB0899_BASE_ACQ_TIME,          0x00000000 }, /* ACQTIME        */
	{ STB0899_OFF0_FINAL_AGC_CNTRL,    STB0899_BASE_FINAL_AGC_CNTRL,   0x00800c17 }, /* FINALAGCCNTRL  */
	{ STB0899_OFF0_FINAL_AGC_GAIN,     STB0899_BASE_FINAL_AGC_GAIN,    0x00000000 }, /* FINALAGCCGAIN  */
	{ STB0899_OFF0_EQUALIZER_INIT,     STB0899_BASE_EQUALIZER_INIT,    0x00000000 }, /* EQUILIZERINIT  */
	{ STB0899_OFF0_EQ_CNTRL,           STB0899_BASE_EQ_CNTRL,          0x00054802 }, /* EQCNTL         */
	{ STB0899_OFF0_EQ_I_INIT_COEFF_0,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF0  */
	{ STB0899_OFF1_EQ_I_INIT_COEFF_1,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF1  */
	{ STB0899_OFF2_EQ_I_INIT_COEFF_2,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF2  */
	{ STB0899_OFF3_EQ_I_INIT_COEFF_3,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF3  */
	{ STB0899_OFF4_EQ_I_INIT_COEFF_4,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF4  */
	{ STB0899_OFF5_EQ_I_INIT_COEFF_5,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000400 }, /* EQIINITCOEFF5  */
	{ STB0899_OFF6_EQ_I_INIT_COEFF_6,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF6  */
	{ STB0899_OFF7_EQ_I_INIT_COEFF_7,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF7  */
	{ STB0899_OFF8_EQ_I_INIT_COEFF_8,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF8  */
	{ STB0899_OFF9_EQ_I_INIT_COEFF_9,  STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF9  */
	{ STB0899_OFFa_EQ_I_INIT_COEFF_10, STB0899_BASE_EQ_I_INIT_COEFF_N, 0x00000000 }, /* EQIINITCOEFF10 */
	{ STB0899_OFF0_EQ_Q_INIT_COEFF_0,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF0  */
	{ STB0899_OFF1_EQ_Q_INIT_COEFF_1,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF1  */
	{ STB0899_OFF2_EQ_Q_INIT_COEFF_2,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF2  */
	{ STB0899_OFF3_EQ_Q_INIT_COEFF_3,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF3  */
	{ STB0899_OFF4_EQ_Q_INIT_COEFF_4,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF4  */
	{ STB0899_OFF5_EQ_Q_INIT_COEFF_5,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF5  */
	{ STB0899_OFF6_EQ_Q_INIT_COEFF_6,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF6  */
	{ STB0899_OFF7_EQ_Q_INIT_COEFF_7,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF7  */
	{ STB0899_OFF8_EQ_Q_INIT_COEFF_8,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF8  */
	{ STB0899_OFF9_EQ_Q_INIT_COEFF_9,  STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF9  */
	{ STB0899_OFFa_EQ_Q_INIT_COEFF_10, STB0899_BASE_EQ_Q_INIT_COEFF_N, 0x00000000 }, /* EQQINITCOEFF10 */
	{ STB0899_OFF0_EQ_I_OUT_COEFF_0,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT0  */
	{ STB0899_OFF1_EQ_I_OUT_COEFF_1,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT1  */
	{ STB0899_OFF2_EQ_I_OUT_COEFF_2,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT2  */
	{ STB0899_OFF3_EQ_I_OUT_COEFF_3,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT3  */
	{ STB0899_OFF4_EQ_I_OUT_COEFF_4,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT4  */
	{ STB0899_OFF5_EQ_I_OUT_COEFF_5,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT5  */
	{ STB0899_OFF6_EQ_I_OUT_COEFF_6,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT6  */
	{ STB0899_OFF7_EQ_I_OUT_COEFF_7,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT7  */
	{ STB0899_OFF8_EQ_I_OUT_COEFF_8,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT8  */
	{ STB0899_OFF9_EQ_I_OUT_COEFF_9,   STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT9  */
	{ STB0899_OFFa_EQ_I_OUT_COEFF_10,  STB0899_BASE_EQ_I_OUT_COEFF_N,  0x00000000 }, /* EQICOEFFSOUT10 */
	{ STB0899_OFF0_EQ_Q_OUT_COEFF_0,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT0  */
	{ STB0899_OFF1_EQ_Q_OUT_COEFF_1,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT1  */
	{ STB0899_OFF2_EQ_Q_OUT_COEFF_2,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT2  */
	{ STB0899_OFF3_EQ_Q_OUT_COEFF_3,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT3  */
	{ STB0899_OFF4_EQ_Q_OUT_COEFF_4,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT4  */
	{ STB0899_OFF5_EQ_Q_OUT_COEFF_5,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT5  */
	{ STB0899_OFF6_EQ_Q_OUT_COEFF_6,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT6  */
	{ STB0899_OFF7_EQ_Q_OUT_COEFF_7,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT7  */
	{ STB0899_OFF8_EQ_Q_OUT_COEFF_8,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT8  */
	{ STB0899_OFF9_EQ_Q_OUT_COEFF_9,   STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT9  */
	{ STB0899_OFFa_EQ_Q_OUT_COEFF_10,  STB0899_BASE_EQ_Q_OUT_COEFF_N,  0x00000000 }, /* EQQCOEFFSOUT10 */
	{ 0xffff,                          0xffffffff,                     0xffffffff },
};

static const struct stb0899_s1_reg stb0899_init_s1_demod[] =
{
	{ STB0899_DEMOD,          0x00 },
	{ STB0899_RCOMPC,         0xc9 },
	{ STB0899_AGC1CN,         0x01 },
	{ STB0899_AGC1REF,        0x10 },
	{ STB0899_RTC,            0x23 },
	{ STB0899_TMGCFG,         0x4e },
	{ STB0899_AGC2REF,        0x34 },
	{ STB0899_TLSR,           0x84 },
	{ STB0899_CFD,            0xf7 },
	{ STB0899_ACLC,           0x87 },
	{ STB0899_BCLC,           0x94 },
	{ STB0899_EQON,           0x41 },
	{ STB0899_LDT,            0xf1 },
	{ STB0899_LDT2,           0xe3 },
	{ STB0899_EQUALREF,       0xb4 },
	{ STB0899_TMGRAMP,        0x10 },
	{ STB0899_TMGTHD,         0x30 },
	{ STB0899_IDCCOMP,        0xfd },
	{ STB0899_QDCCOMP,        0xff },
	{ STB0899_POWERI,         0x0c },
	{ STB0899_POWERQ,         0x0f },
	{ STB0899_RCOMP,          0x6c },
	{ STB0899_AGCIQIN,        0x80 },
	{ STB0899_AGC2I1,         0x06 },
	{ STB0899_AGC2I2,         0x00 },
	{ STB0899_TLIR,           0x30 },
	{ STB0899_RTF,            0x7f },
	{ STB0899_DSTATUS,        0x00 },
	{ STB0899_LDI,            0xbc },
	{ STB0899_CFRM,           0xea },
	{ STB0899_CFRL,           0x31 },
	{ STB0899_NIRM,           0x2b },
	{ STB0899_NIRL,           0x80 },
	{ STB0899_ISYMB,          0x1d },
	{ STB0899_QSYMB,          0xa6 },
	{ STB0899_SFRH,           0x2f },
	{ STB0899_SFRM,           0x68 },
	{ STB0899_SFRL,           0x40 },
	{ STB0899_SFRUPH,         0x2f },
	{ STB0899_SFRUPM,         0x68 },
	{ STB0899_SFRUPL,         0x40 },
	{ STB0899_EQUAI1,         0x02 },
	{ STB0899_EQUAQ1,         0xff },
	{ STB0899_EQUAI2,         0x04 },
	{ STB0899_EQUAQ2,         0x05 },
	{ STB0899_EQUAI3,         0x02 },
	{ STB0899_EQUAQ3,         0xfd },
	{ STB0899_EQUAI4,         0x03 },
	{ STB0899_EQUAQ4,         0x07 },
	{ STB0899_EQUAI5,         0x08 },
	{ STB0899_EQUAQ5,         0xf5 },
	{ STB0899_DSTATUS2,       0x00 },
	{ STB0899_VSTATUS,        0x00 },
	{ STB0899_VERROR,         0x86 },
	{ STB0899_IQSWAP,         0x2a },
	{ STB0899_ECNT1M,         0x00 },
	{ STB0899_ECNT1L,         0x00 },
	{ STB0899_ECNT2M,         0x00 },
	{ STB0899_ECNT2L,         0x00 },
	{ STB0899_ECNT3M,         0x0a },
	{ STB0899_ECNT3L,         0xad },
	{ STB0899_FECAUTO1,       0x06 },
	{ STB0899_FECM,           0x01 },
	{ STB0899_VTH12,          0xb0 },
	{ STB0899_VTH23,          0x7a },
	{ STB0899_VTH34,          0x58 },
	{ STB0899_VTH56,          0x38 },
	{ STB0899_VTH67,          0x34 },
	{ STB0899_VTH78,          0x24 },
	{ STB0899_PRVIT,          0xff },
	{ STB0899_VITSYNC,        0x19 },
	{ STB0899_RSULC,          0xb1 },  /* DVB = 0xb1, DSS = 0xa1 */
	{ STB0899_TSULC,          0x42 },
	{ STB0899_RSLLC,          0x41 },
	{ STB0899_TSLPL,          0x12 },
	{ STB0899_TSCFGH,         0x0c },
	{ STB0899_TSCFGM,         0x00 },
	{ STB0899_TSCFGL,         0x00 },
	{ STB0899_TSOUT,          0x69 },  /* 0x0d for CAM */
	{ STB0899_RSSYNCDEL,      0x00 },
	{ STB0899_TSINHDELH,      0x02 },
	{ STB0899_TSINHDELM,      0x00 },
	{ STB0899_TSINHDELL,      0x00 },
	{ STB0899_TSLLSTKM,       0x1b },
	{ STB0899_TSLLSTKL,       0xb3 },
	{ STB0899_TSULSTKM,       0x00 },
	{ STB0899_TSULSTKL,       0x00 },
	{ STB0899_PCKLENUL,       0xbc },
	{ STB0899_PCKLENLL,       0xcc },
	{ STB0899_RSPCKLEN,       0xbd },
	{ STB0899_TSSTATUS,       0x90 },
	{ STB0899_ERRCTRL1,       0x93 },  // b6 - from the original driver
	{ STB0899_ERRCTRL2,       0xe3 },  // 95
	{ STB0899_ERRCTRL3,       0xb0 },  // 8d
	{ STB0899_DMONMSK1,       0x27 },
	{ STB0899_DMONMSK0,       0x03 },
	{ STB0899_DEMAPVIT,       0x5c },
	{ STB0899_PLPARM,         0x19 },
	{ STB0899_PDELCTRL,       0x48 },
	{ STB0899_PDELCTRL2,      0x00 },
	{ STB0899_BBHCTRL1,       0x00 },
	{ STB0899_BBHCTRL2,       0x00 },
	{ STB0899_HYSTTHRESH,     0x77 },
	{ STB0899_MATCSTM,        0x00 },
	{ STB0899_MATCSTL,        0x00 },
	{ STB0899_UPLCSTM,        0x00 },
	{ STB0899_UPLCSTL,        0x00 },
	{ STB0899_DFLCSTM,        0x00 },
	{ STB0899_DFLCSTL,        0x00 },
	{ STB0899_SYNCCST,        0x00 },
	{ STB0899_SYNCDCSTM,      0x00 },
	{ STB0899_SYNCDCSTL,      0x00 },
	{ STB0899_ISI_ENTRY,      0x00 },
	{ STB0899_ISI_BIT_EN,     0x00 },
	{ STB0899_MATSTRM,        0xf0 },
	{ STB0899_MATSTRL,        0x02 },
	{ STB0899_UPLSTRM,        0x45 },
	{ STB0899_UPLSTRL,        0x60 },
	{ STB0899_DFLSTRM,        0xe3 },
	{ STB0899_DFLSTRL,        0x00 },
	{ STB0899_SYNCSTR,        0x47 },
	{ STB0899_SYNCDSTRM,      0x05 },
	{ STB0899_SYNCDSTRL,      0x18 },
	{ STB0899_CFGPDELSTATUS1, 0x19 },
	{ STB0899_CFGPDELSTATUS2, 0x2b },
	{ STB0899_BBFERRORM,      0x00 },
	{ STB0899_BBFERRORL,      0x01 },
	{ STB0899_UPKTERRORM,     0x00 },
	{ STB0899_UPKTERRORL,     0x00 },
	{ 0xffff,                 0xff },
};

static const struct stb0899_s2_reg stb0899_init_s2_fec[] =
{
	{ STB0899_OFF0_BLOCK_LNGTH,     STB0899_BASE_BLOCK_LNGTH,     0x00000008 }, /* BLOCKLNGTH */
	{ STB0899_OFF0_ROW_STR,         STB0899_BASE_ROW_STR,         0x000000b4 }, /* ROWSTR     */
	{ STB0899_OFF0_BN_END_ADDR,     STB0899_BASE_BN_END_ADDR,     0x000004b5 }, /* BNANDADDR  */
	{ STB0899_OFF0_CN_END_ADDR,     STB0899_BASE_CN_END_ADDR,     0x00000b4b }, /* CNANDADDR  */
	{ STB0899_OFF0_INFO_LENGTH,     STB0899_BASE_INFO_LENGTH,     0x00000078 }, /* INFOLENGTH */
	{ STB0899_OFF0_BOT_ADDR,        STB0899_BASE_BOT_ADDR,        0x000001e0 }, /* BOT_ADDR   */
	{ STB0899_OFF0_BCH_BLK_LN,      STB0899_BASE_BCH_BLK_LN,      0x0000a8c0 }, /* BCHBLKLN   */
	{ STB0899_OFF0_BCH_T,           STB0899_BASE_BCH_T,           0x0000000c }, /* BCHT       */
	{ STB0899_OFF0_CNFG_MODE,       STB0899_BASE_CNFG_MODE,       0x00000001 }, /* CNFGMODE   */
	{ STB0899_OFF0_LDPC_STAT,       STB0899_BASE_LDPC_STAT,       0x0000000d }, /* LDPCSTAT   */
	{ STB0899_OFF0_ITER_SCALE,      STB0899_BASE_ITER_SCALE,      0x00000040 }, /* ITERSCALE  */
	{ STB0899_OFF0_INPUT_MODE,      STB0899_BASE_INPUT_MODE,      0x00000000 }, /* INPUTMODE  */
	{ STB0899_OFF0_LDPCDECRST,      STB0899_BASE_LDPCDECRST,      0x00000000 }, /* LDPCDECRST */
	{ STB0899_OFF0_CLK_PER_BYTE_RW, STB0899_BASE_CLK_PER_BYTE_RW, 0x00000008 }, /* CLKPERBYTE */
	{ STB0899_OFF0_BCH_ERRORS,      STB0899_BASE_BCH_ERRORS,      0x00000000 }, /* BCHERRORS  */
	{ STB0899_OFF0_LDPC_ERRORS,     STB0899_BASE_LDPC_ERRORS,     0x00000000 }, /* LDPCERRORS */
	{ STB0899_OFF0_BCH_MODE,        STB0899_BASE_BCH_MODE,        0x00000000 }, /* BCHMODE    */
	{ STB0899_OFF0_ERR_ACC_PER,     STB0899_BASE_ERR_ACC_PER,     0x00000008 }, /* ERRACCPER  */
	{ STB0899_OFF0_BCH_ERR_ACC,     STB0899_BASE_BCH_ERR_ACC,     0x00000000 }, /* BCHERRACC  */
	{ STB0899_OFF0_FEC_TP_SEL,      STB0899_BASE_FEC_TP_SEL,      0x00000000 }, /* FECTPSEL   */
	{ 0xffff,                       0xffffffff,                   0xffffffff },
};

static const struct stb0899_s1_reg stb0899_init_tst[] =
{
	{ STB0899_TSTCK,      0x00 },
	{ STB0899_TSTRES,     0x00 },
	{ STB0899_TSTOUT,     0x00 },
	{ STB0899_TSTIN,      0x00 },
	{ STB0899_TSTSYS,     0x00 },
	{ STB0899_TSTCHIP,    0x00 },
	{ STB0899_TSTFREE,    0x00 },
	{ STB0899_TSTI2C,     0x00 },
	{ STB0899_BITSPEEDM,  0x00 },
	{ STB0899_BITSPEEDL,  0x00 },
	{ STB0899_TBUSBIT,    0x00 },
	{ STB0899_TSTDIS,     0x00 },
	{ STB0899_TSTDISRX,   0x00 },
	{ STB0899_TSTJETON,   0x00 },
	{ STB0899_TSTDCADJ,   0x00 },
	{ STB0899_TSTAGC1,    0x00 },
	{ STB0899_TSTAGC1N,   0x00 },
	{ STB0899_TSTPOLYPH,  0x00 },
	{ STB0899_TSTR,       0x00 },
	{ STB0899_TSTAGC2,    0x00 },
	{ STB0899_TSTCTL1,    0x00 },
	{ STB0899_TSTCTL2,    0x00 },
	{ STB0899_TSTCTL3,    0x00 },
	{ STB0899_TSTDEMAP,   0x00 },
	{ STB0899_TSTDEMAP2,  0x00 },
	{ STB0899_TSTDEMMON,  0x00 },
	{ STB0899_TSTRATE,    0x00 },
	{ STB0899_TSTSELOUT,  0x00 },
	{ STB0899_TSYNC,      0x00 },
	{ STB0899_TSTERR,     0x00 },
	{ STB0899_TSTRAM1,    0x00 },
	{ STB0899_TSTVSELOUT, 0x00 },
	{ STB0899_TSTFORCEIN, 0x00 },
	{ STB0899_TSTRS1,     0x00 },
	{ STB0899_TSTRS2,     0x00 },
	{ STB0899_TSTRS3,     0x00 },
	{ STB0899_GHOSTREG,   0x81 },
	{ 0xffff,             0xff },
};

#define CORE_STB0899_DVBS2_ESNO_AVE              3
#define CORE_STB0899_DVBS2_ESNO_QUANT            32
#define CORE_STB0899_DVBS2_AVFRAMES_COARSE       10
#define CORE_STB0899_DVBS2_AVFRAMES_FINE         20
#define CORE_STB0899_DVBS2_MISS_THRESHOLD        6
#define CORE_STB0899_DVBS2_UWP_THRESHOLD_ACQ     1125
#define CORE_STB0899_DVBS2_UWP_THRESHOLD_TRACK   758
#define CORE_STB0899_DVBS2_UWP_THRESHOLD_SOF     1350
#define CORE_STB0899_DVBS2_SOF_SEARCH_TIMEOUT    1664100

#define CORE_STB0899_DVBS2_BTR_NCO_BITS          28
#define CORE_STB0899_DVBS2_BTR_GAIN_SHIFT_OFFSET 15
#define CORE_STB0899_DVBS2_CRL_NCO_BITS          30
#define CORE_STB0899_DVBS2_LDPC_MAX_ITER         70

static struct stb0899_config stb0899_config =
{
	.init_dev              = stb0899_init_dev,
	.init_s2_demod         = stb0899_init_s2_demod,
	.init_s1_demod         = stb0899_init_s1_demod,
	.init_s2_fec           = stb0899_init_s2_fec,
	.init_tst              = stb0899_init_tst,

	.lnb_enable            = NULL,
	.lnb_vsel              = NULL,

	.demod_address         = I2C_ADDR_STB0899,  /* I2C Address */
	.block_sync_mode       = STB0899_SYNC_FORCED,  /* ? */

	.xtal_freq             = 27000000,  /* Assume Hz ? */
	.inversion             = IQ_SWAP_ON,  /* ? */

	.lo_clk                = 76500000,
	.hi_clk                = 99000000,

	.ts_output_mode        = 0,  /* Use parallel mode */
	.clock_polarity        = 0,  /*  */
	.data_clk_parity       = 0,  /*  */
	.fec_mode              = 0,  /*  */

	.esno_ave              = CORE_STB0899_DVBS2_ESNO_AVE,
	.esno_quant            = CORE_STB0899_DVBS2_ESNO_QUANT,
	.avframes_coarse       = CORE_STB0899_DVBS2_AVFRAMES_COARSE,
	.avframes_fine         = CORE_STB0899_DVBS2_AVFRAMES_FINE,
	.miss_threshold        = CORE_STB0899_DVBS2_MISS_THRESHOLD,
	.uwp_threshold_acq     = CORE_STB0899_DVBS2_UWP_THRESHOLD_ACQ,
	.uwp_threshold_track   = CORE_STB0899_DVBS2_UWP_THRESHOLD_TRACK,
	.uwp_threshold_sof     = CORE_STB0899_DVBS2_UWP_THRESHOLD_SOF,
	.sof_search_timeout    = CORE_STB0899_DVBS2_SOF_SEARCH_TIMEOUT,

	.btr_nco_bits          = CORE_STB0899_DVBS2_BTR_NCO_BITS,
	.btr_gain_shift_offset = CORE_STB0899_DVBS2_BTR_GAIN_SHIFT_OFFSET,
	.crl_nco_bits          = CORE_STB0899_DVBS2_CRL_NCO_BITS,
	.ldpc_max_iter         = CORE_STB0899_DVBS2_LDPC_MAX_ITER,

	.tuner_get_frequency   = stb6100_get_frequency,
	.tuner_set_frequency   = stb6100_set_frequency,
	.tuner_set_bandwidth   = stb6100_set_bandwidth,
	.tuner_get_bandwidth   = stb6100_get_bandwidth,
	.tuner_set_rfsiggain   = NULL,
};

static struct stb6100_config stb6100_config =
{
	.tuner_address         = I2C_ADDR_STB6100,
	.refclock              = 27000000
};

static struct dvb_frontend *frontend_init(struct core_config *cfg, int i)
{
	struct dvb_frontend *frontend = NULL;

//	dprintk(50, "%s >\n", __func__);

	if (i > 0)
	{
		return NULL;
	}
//	dprintk(20,"Attaching STB0899\n");
	frontend = dvb_attach(stb0899_attach, &stb0899_config, cfg->i2c_adap);

	if (frontend)
	{
		dprintk(20, "STB0899 attached OK\n");

		if (dvb_attach(stb6100_attach, frontend, &stb6100_config, cfg->i2c_adap) == 0)
		{
			dprintk(1, "%s Error attaching STB6100\n", __func__);
			goto error_out;
		}
		dprintk(20, "STB6100 attached OK\n");
	}
	else
	{
		dprintk(1, "%s: Error attaching STB0899\n", __func__);
		goto error_out;
	}
	stb0899_config.lnb_enable = cfg->lnb_enable;
	stb0899_config.lnb_vsel   = cfg->lnb_vsel;
	return frontend;

error_out:
	dprintk(1, "%s Frontend registration failed!\n", __func__);
	if (frontend)
	{
		dvb_frontend_detach(frontend);
	}
	return NULL;
}

static struct dvb_frontend *init_fe_device(struct dvb_adapter *adapter, struct plat_tuner_config *tuner_cfg, int i)
{
	struct fe_core_state *state;
	struct dvb_frontend  *frontend;
	struct core_config   *cfg;

//	dprintk(50, "%s > (I2C bus = %d)\n", __func__, tuner_cfg->i2c_bus);

	cfg = kmalloc(sizeof(struct core_config), GFP_KERNEL);
	if (cfg == NULL)
	{
		dprintk(1, "%s kmalloc failed\n", __func__);
		return NULL;
	}
	/* initialize the config data */
	cfg->i2c_adap = i2c_get_adapter(tuner_cfg->i2c_bus);

//	dprintk(50, "%s i2c adapter = 0x%0x, bus = 0x%0x\n", __func__, cfg->i2c_adap, tuner_cfg->i2c_bus);

	cfg->i2c_addr = tuner_cfg->i2c_addr;

	if (cfg->i2c_adap == NULL)
	{
		dprintk(1, "%s Failed to allocate resources (%s)\n", __func__, (cfg->i2c_adap == NULL) ? "i2c" : "STPIO error");
		kfree(cfg);
		return NULL;
	}
	frontend = frontend_init(cfg, i);

	dprintk(50, "%s: frontend_init (frontend = 0x%x)\n", __func__, (unsigned int) frontend);

	if (frontend == NULL)
	{
		dprintk(1, "No frontend found !\n");
		return NULL;
	}
//	dprintk(50, "%s: Call dvb_register_frontend (adapter = 0x%x)\n", __func__, (unsigned int) adapter);

	if (dvb_register_frontend(adapter, frontend))
	{
		printk("[fe_core] %s: Frontend registration failed !\n", __func__);
		if (frontend->ops.release)
		{
			frontend->ops.release(frontend);
		}
		return NULL;
	}
	state = frontend->demodulator_priv;
	return frontend;
}

struct plat_tuner_config tuner_resources[] =
{
	[0] =
	{
		.adapter  = 0,
		.i2c_bus  = 0,
		.i2c_addr = I2C_ADDR_STB0899,
	},
};

void fe_core_register_frontend(struct dvb_adapter *dvb_adap)
{
	int i = 0;
	int vLoop = 0;

//	dprintk(50, "%s: > adb_box frontend core\n", __func__);

	core[i] = (struct core *)kmalloc(sizeof(struct core), GFP_KERNEL);
	if (!core[i])
	{
		return;
	}
	memset(core[i], 0, sizeof(struct core));

	core[i]->dvb_adapter = dvb_adap;
	dvb_adap->priv = core[i];

	dprintk(20, "Number of tuner(s) = %d\n", ARRAY_SIZE(tuner_resources));

	for (vLoop = 0; vLoop < ARRAY_SIZE(tuner_resources); vLoop++)
	{
		if (core[i]->frontend[vLoop] == NULL)
		{
//			dprintk(10, "%s: Initialize tuner %d i2c_addr: 0x%.2x\n", __func__, vLoop, tuner_resources[vLoop].i2c_addr);
			core[i]->frontend[vLoop] = init_fe_device(core[i]->dvb_adapter, &tuner_resources[vLoop], vLoop);
		}
	}
//	dprintk(50, "%s: <\n", __func__);
	return;
}
EXPORT_SYMBOL(fe_core_register_frontend);

int __init fe_core_init(void)
{
//	dprintk(20, "%s Frontend core loaded\n", __func__);
	return 0;
}

static void __exit fe_core_exit(void)
{
//	dprintk(20, "Frontend core unloaded\n");
}

module_init(fe_core_init);
module_exit(fe_core_exit);

module_param(paramDebug, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(paramDebug, "Debug Output 0=disabled (default), >1=enabled (level_");

MODULE_DESCRIPTION("STB0899_Frontend_core");
MODULE_AUTHOR("Open Vision developers");
MODULE_LICENSE("GPL");
// vim:ts=4
