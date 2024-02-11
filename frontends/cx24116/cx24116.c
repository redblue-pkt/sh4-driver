/**************************************************************************
 *
 * Conexant CX24116/CX24118 - DVB-S(2) Satellite demod/tuner driver
 *
 * Copyright (C) 2006 Steven Toth <stoth@hauppauge.com>
 * Copyright (C) 2006 Georg Acher (acher(at)baycom(dot)de) for Reel Multimedia
 *                    Added DiSEqC, auto pilot tuning and hack for old DVB-API
 *
 * Version for Kathrein UFS-922 (early production with two Sharp BS2F7VZ0165
 *                               frontends and two STM LNBH23L LNB power
 *                               controllers; CX24118 tuner I2C connected to
 *                               I2C repeater in CX24116 demodulator)
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
 * TODO:
 * - add clean-up code
 * - add support for multiple adapters (low priority)
 *
 * Copyright (C) 2006 Steven Toth <stoth@hauppauge.com>
 * Copyright (C) 2006 Georg Acher (acher (at) baycom (dot) de) for Reel Multimedia
 *                    Added Diseqc, auto pilot tuning and hack for old DVB-API
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
 * TODO:
 * - add clean-up code
 * - add support for multiple adapters (low priority)
 *
 **************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------
 * 20210630 Audioniek       Added value for LNB power off.
 * 20210630 Audioniek       As driver is currently only used on UFS922,
 *                          removed/commented out everything relating to the
 *                          STM LNBH221 LNB power controller.
 * 20210630 Audioniek       Everything pertaining to DVB_API_VERSION < 5
 *                          commented out.
 * 20210630 Audioniek       Added more dprintk diagnostics.
 * 20210701 Audioniek       Correctly initialize frontend->id when
 *                          attaching.
 */
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/firmware.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else
#include <linux/stpio.h>
#endif
#include <asm/system.h>
#include <asm/io.h>
#include <linux/dvb/dmx.h>
#include <linux/proc_fs.h>

#include <linux/dvb/version.h>
#include <pvr_config.h>
#include <linux/platform_device.h>

#include "cx24116.h"


static short useUnknown = 1;

static short paramDebug = 0;

#define TAGDEBUG "[cx24116] "
#define dprintk(level, x...) \
do \
{ \
	if ((paramDebug >= level) || (level == 0)) \
	{ \
		printk(TAGDEBUG x); \
	} \
} while (0)

#define CX24116_DEFAULT_FIRMWARE "dvb-fe-cx21143.fw"
#define CX24116_SEARCH_RANGE_KHZ 5000
#define cMaxError 5

// Fast i2c delay (1ms ~ 1MHz) is only used to speed up the firmware
// download. All other read/write operations are executed with the default
// i2c bus speed (100kHz ~ 10ms).
#define I2C_FAST_DELAY 1

/* rolloff */
#define CX24116_ROLLOFF_020   (0x00)
#define CX24116_ROLLOFF_025   (0x01)
#define CX24116_ROLLOFF_035   (0x02)

/* pilot bit */
#define CX24116_PILOT_OFF     (0x00)
#define CX24116_PILOT_ON      (0x40)

/* known registers */
#define CX24116_REG_QUALITY   (0xd5)
#define CX24116_REG_STATUS    (0x9d)  /* signal high : status */
#define CX24116_REG_SIGNAL    (0x9e)  /* signal low           */
#define CX24116_REG_QSTATUS   (0xbc)

#define CX24116_REG_BER0      (0xc6)
#define CX24116_REG_BER8      (0xc7)
#define CX24116_REG_BER16     (0xc8)
#define CX24116_REG_BER24     (0xc9)

#define CX24116_REG_UCB0      (0xcb)
#define CX24116_REG_UCB8      (0xca)

#define CX24116_REG_FECSTATUS (0x9c)
#define CX24116_FEC_FECMASK   (0x1f)

/* arg buffer size */
#define CX24116_ARGLEN        (0x1e)

/* DiSEqC strategy (0=HACK, 1=CACHE) */
static int dsec = 0;

//0 = OFF
//1 = ON
//2 = AUTO
static int pilot = 0;

/* arg offset for DiSEqC */
#define CX24116_DISEQC_BURST  (1)
#define CX24116_DISEQC_ARG2_2 (2)  /* unknown value = 2 */
#define CX24116_DISEQC_ARG3_0 (3)  /* unknown value = 0 */
#define CX24116_DISEQC_ARG4_0 (4)  /* unknown value = 0 */
#define CX24116_DISEQC_MSGLEN (5)
#define CX24116_DISEQC_MSGOFS (6)

/* DiSEqC burst */
#define CX24116_DISEQC_MINI_A (0)
#define CX24116_DISEQC_MINI_B (1)

struct firmware_cmd
{
	enum cmds id;  /* Unique firmware command */
	int len;       /* Commands args len + id byte */
	char *name;
}

cx24116_COMMANDS[] =
{
	{ CMD_SET_VCO,     0x0A, "CMD_SET_VCO" },
	{ CMD_TUNEREQUEST, 0x13, "CMD_TUNEREQUEST" },
	{ CMD_MPEGCONFIG,  0x06, "CMD_MPEGCONFIG" },
	{ CMD_TUNERINIT,   0x03, "CMD_TUNERINIT" },
	{ CMD_BANDWIDTH,   0x02, "CMD_BANDWIDTH" },
	{ CMD_LNBCONFIG,   0x08, "CMD_LNBCONFIG" },
	{ CMD_LNBSEND,     0x0c, "CMD_LNBSEND" },
	{ CMD_SET_TONEPRE, 0x02, "CMD_SET_TONEPRE" },
	{ CMD_SET_TONE,    0x04, "CMD_SET_TONE" },
	{ CMD_TUNERSLEEP,  0x02, "CMD_TUNERSLEEP" },
	{ CMD_U1,          0x01, "CMD_U1" },
	{ CMD_U2,          0x0A, "CMD_U2" },
	{ CMD_GETAGC,      0x01, "CMD_GETAGC" },
	{ CMD_MAX,         0x00, "CMD_MAX" }
};

/* A table of modulation, fec and configuration bytes for the demod.
 * Not all S2 modulation schemes are supported and not all rates
 * with a scheme are supported; especially, no auto detect when in
 * S2 mode.
 */
struct cx24116_modfec
{
#if 0 //DVB_API_VERSION < 5
	enum dvbfe_delsys     delsys;
	enum dvbfe_modulation modulation;
	enum dvbfe_fec        fec;
#else
	fe_delivery_system_t  delsys;
	fe_modulation_t       modulation;
	fe_code_rate_t        fec;
#endif
	u8 mask;  /* In DVB-S mode this is used to autodetect */
	u8 val;   /* Passed to the firmware to indicate mode selection */
} cx24116_MODFEC_MODES[] =
{
#if 0 //DVB_API_VERSION < 5
	/* QPSK. For unknown rates we set to hardware to to detect 0xfe 0x30 */
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_NONE, 0xfe, 0x30 },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_1_2,  0x02, 0x2e },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_2_3,  0x04, 0x2f },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_3_4,  0x08, 0x30 },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_4_5,  0xfe, 0x30 },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_5_6,  0x20, 0x31 },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_6_7,  0xfe, 0x30 },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_7_8,  0x80, 0x32 },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_8_9,  0xfe, 0x30 },
	{ DVBFE_DELSYS_DVBS,  DVBFE_MOD_QPSK, DVBFE_FEC_AUTO, 0xfe, 0x30 },
	/* NBC-QPSK */
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_1_2,  0x00, 0x04 },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_3_5,  0x00, 0x05 },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_2_3,  0x00, 0x06 },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_3_4,  0x00, 0x07 },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_4_5,  0x00, 0x08 },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_5_6,  0x00, 0x09 },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_8_9,  0x00, 0x0a },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_QPSK, DVBFE_FEC_9_10, 0x00, 0x0b },
	/* 8PSK */
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_8PSK, DVBFE_FEC_3_5,  0x00, 0x0c },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_8PSK, DVBFE_FEC_2_3,  0x00, 0x0d },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_8PSK, DVBFE_FEC_3_4,  0x00, 0x0e },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_8PSK, DVBFE_FEC_5_6,  0x00, 0x0f },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_8PSK, DVBFE_FEC_8_9,  0x00, 0x10 },
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_8PSK, DVBFE_FEC_9_10, 0x00, /*0x11 pvrmain sets: */ 0x0b },
	/* : redirect AUTO for test */
	{ DVBFE_DELSYS_DVBS2, DVBFE_MOD_8PSK, DVBFE_FEC_AUTO, 0x00, 0x0b },
};
#else
	/* QPSK. For unknown rates we set to hardware to to detect 0xfe 0x30 */
	{ SYS_DVBS,  QPSK,  FEC_NONE, 0xfe, 0x30 },
	{ SYS_DVBS,  QPSK,  FEC_1_2,  0x02, 0x2e },
	{ SYS_DVBS,  QPSK,  FEC_2_3,  0x04, 0x2f },
	{ SYS_DVBS,  QPSK,  FEC_3_4,  0x08, 0x30 },
	{ SYS_DVBS,  QPSK,  FEC_4_5,  0xfe, 0x30 },
	{ SYS_DVBS,  QPSK,  FEC_5_6,  0x20, 0x31 },
	{ SYS_DVBS,  QPSK,  FEC_6_7,  0xfe, 0x30 },
	{ SYS_DVBS,  QPSK,  FEC_7_8,  0x80, 0x32 },
	{ SYS_DVBS,  QPSK,  FEC_8_9,  0xfe, 0x30 },
	{ SYS_DVBS,  QPSK,  FEC_AUTO, 0xfe, 0x30 },
	/* NBC-QPSK */
	{ SYS_DVBS2, QPSK,  FEC_1_2,  0x00, 0x04 },
	{ SYS_DVBS2, QPSK,  FEC_3_5,  0x00, 0x05 },
	{ SYS_DVBS2, QPSK,  FEC_2_3,  0x00, 0x06 },
	{ SYS_DVBS2, QPSK,  FEC_3_4,  0x00, 0x07 },
	{ SYS_DVBS2, QPSK,  FEC_4_5,  0x00, 0x08 },
	{ SYS_DVBS2, QPSK,  FEC_5_6,  0x00, 0x09 },
	{ SYS_DVBS2, QPSK,  FEC_8_9,  0x00, 0x0a },
	{ SYS_DVBS2, QPSK,  FEC_9_10, 0x00, 0x0b },
	/* 8PSK */
	{ SYS_DVBS2, PSK_8, FEC_3_5,  0x00, 0x0c },
	{ SYS_DVBS2, PSK_8, FEC_2_3,  0x00, 0x0d },
	{ SYS_DVBS2, PSK_8, FEC_3_4,  0x00, 0x0e },
	{ SYS_DVBS2, PSK_8, FEC_5_6,  0x00, 0x0f },
	{ SYS_DVBS2, PSK_8, FEC_8_9,  0x00, 0x10 },
	{ SYS_DVBS2, PSK_8, FEC_9_10, 0x00, /*0x11 pvrmain sets: */ 0x0b },
	/* : redirect AUTO for test */
	{ SYS_DVBS2, PSK_8, FEC_AUTO, 0x00, 0x0b },
};
#endif

struct cx24116_U2
{
	u8 U2_1;
	u8 U2_2;
	u8 U2_3;
	u8 U2_4;
	u8 U2_5;
	u8 U2_6;
	u8 U2_7;
	u8 U2_8;
	u8 U2_9;
} cx24116_U2_TABLE[] =
{
	/* fec none */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	/* fec none */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	/* 1/2      */ { 0x10, 0x01, 0x2a, 0x08, 0x01, 0x2a, 0x08, 0x01, 0x00 },  // ok
	/* 1/2      */ { 0x10, 0x01, 0x2a, 0x08, 0x01, 0x2a, 0x08, 0x01, 0x00 },  // ok
	/* 2/3      */ { 0x10, 0x02, 0x54, 0x10, 0x01, 0xbf, 0x0c, 0x01, 0x55 },  // ok
	/* 2/3      */ { 0x10, 0x02, 0x54, 0x10, 0x01, 0xbf, 0x0c, 0x01, 0x55 },  // ok
	/* 3/4      */ { 0x10, 0x03, 0x7e, 0x18, 0x02, 0x54, 0x10, 0x01, 0x80 },  // ok
	/* 3/4      */ { 0x10, 0x03, 0x7e, 0x18, 0x02, 0x54, 0x10, 0x01, 0x80 },  // ok
	/* 4/5      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 4/5      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 5/6      */ { 0x10, 0x05, 0xd2, 0x28, 0x03, 0x7e, 0x18, 0x01, 0xaa },  // ok
	/* 5/6      */ { 0x10, 0x05, 0xd2, 0x28, 0x03, 0x7e, 0x18, 0x01, 0xaa },  // ok
	/* 6/7      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 6/7      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 7/8      */ { 0x10, 0x08, 0x26, 0x38, 0x04, 0xa8, 0x20, 0x01, 0xc0 },  // ok
	/* 7/8      */ { 0x10, 0x08, 0x26, 0x38, 0x04, 0xa8, 0x20, 0x01, 0xc0 },  // ok
	/* 8/9      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 8/9      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* fec auto */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // ok
	/* fec auto */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // ok
	/* NBC-QPSK */
	/* 1/2      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 1/2      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 3/5      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 3/5      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 2/3      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 2/3      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 3/4      */ { 0x10, 0x06, 0x44, 0xa4, 0x03, 0xe2, 0x29, 0x01, 0x9d },  // ok
	/* 3/4      */ { 0x10, 0x06, 0x44, 0xa4, 0x03, 0xe2, 0x29, 0x01, 0x9d },  // ok
	/* 4/5      */ { 0x10, 0x06, 0xb0, 0x38, 0x03, 0xfa, 0x65, 0x01, 0xae },  // ok
	/* 4/5      */ { 0x10, 0x06, 0xb0, 0x38, 0x03, 0xfa, 0x65, 0x01, 0xae },  // ok
	/* 5/6      */ { 0x10, 0x00, 0x77, 0x00, 0x00, 0x42, 0x47, 0x01, 0xcb },  // ok->waiting on feedback
	/* 5/6      */ { 0x10, 0x00, 0x77, 0x00, 0x00, 0x42, 0x47, 0x01, 0xcb },  // ok->waiting on feedback
	/* 8/9      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 8/9      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 9/10     */ { 0x10, 0x07, 0x89, 0x80, 0x03, 0xe2, 0x29, 0x01, 0xf0 },  // ok
	/* 9/10     */ { 0x10, 0x07, 0x89, 0x80, 0x03, 0xe2, 0x29, 0x01, 0xf0 },  // ok
	/* 8PSK */
	/* 3/5      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 3/5      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 2/3      */ { 0x10, 0x00, 0x5f, 0x18, 0x00, 0x2c, 0x3f, 0x02, 0x26 },  // 0x00 0x05 0x98 0x00 0x02 0xd3 pilot off
	/* 2/3      */ { 0x10, 0x01, 0xdb, 0x78, 0x00, 0xe2, 0x5f, 0x02, 0x19 },  // 0x00 0x1b 0xf8 0x00 0x0e 0x73 pilot on
	/* 3/4      */ { 0x10, 0x06, 0x44, 0xa4, 0x02, 0x97, 0xb1, 0x02, 0x6a },  // ok
	/* 3/4      */ { 0x10, 0x06, 0x44, 0xa4, 0x02, 0xa7, 0x1d, 0x02, 0x5c },  // 0x00 0x5e 0x64 0x00 0x2b 0x59 pilot on
	/* 5/6      */ { 0x10, 0x00, 0x77, 0x00, 0x00, 0x2c, 0x3f, 0x02, 0xb0 },  // 0x00 0x07 0x00 0x00 0x02 0xd3 pilot off
	/* 5/6      */ { 0x10, 0x02, 0x53, 0x00, 0x00, 0xe2, 0x5f, 0x02, 0xa0 },  // ok
	/* 8/9      */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 8/9      */ { 0x10, 0x07, 0x71, 0x98, 0x02, 0xa7, 0x1d, 0x02, 0xce },  // 0x00 0x70 0x18 0x00 0x2b 0x59 pilot on
	/* 9/10     */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
	/* 9/10     */ { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // unknown
};

#if 0 //DVB_API_VERSION < 5
static struct dvbfe_info dvbs_info =
{
	.name                = "Conexant CX24116 DVB-S",
	.delivery            = DVBFE_DELSYS_DVBS,
	.delsys              =
	{
		.dvbs.modulation = DVBFE_MOD_QPSK,
		.dvbs.fec        = DVBFE_FEC_1_2
		                 | DVBFE_FEC_2_3
		                 | DVBFE_FEC_3_4
		                 | DVBFE_FEC_4_5
		                 | DVBFE_FEC_5_6
		                 | DVBFE_FEC_6_7
		                 | DVBFE_FEC_7_8
		                 | DVBFE_FEC_AUTO
	},
	.frequency_min       =  950000,
	.frequency_max       = 2150000,
	.frequency_step      = 1011,
	.frequency_tolerance = 5000,
	.symbol_rate_min     = 1000000,
	.symbol_rate_max     = 45000000
};

static const struct dvbfe_info dvbs2_info =
{
	.name                  = "Conexant CX24116 DVB-S2",
	.delivery              = DVBFE_DELSYS_DVBS2,
	.delsys                =
	{
		.dvbs2.modulation  = DVBFE_MOD_QPSK
		                   | DVBFE_MOD_8PSK,

		/* TODO: Review these */
		.dvbs2.fec         = DVBFE_FEC_1_4
		                   | DVBFE_FEC_1_3
		                   | DVBFE_FEC_2_5
		                   | DVBFE_FEC_1_2
		                   | DVBFE_FEC_3_5
		                   | DVBFE_FEC_2_3
		                   | DVBFE_FEC_3_4
		                   | DVBFE_FEC_4_5
		                   | DVBFE_FEC_5_6
		                   | DVBFE_FEC_8_9
		                   | DVBFE_FEC_9_10
	},
	.frequency_min         =  950000,
	.frequency_max         = 2150000,
	.frequency_step        = 0,
	.symbol_rate_min       =  1000000,
	.symbol_rate_max       = 45000000,
	.symbol_rate_tolerance = 0
};
#endif

static int lnbh23_writereg_lnb_supply(struct cx24116_state *state, char data);

struct config_entry
{
	int count;
	struct plat_tuner_config tuner_cfg[MAX_TUNERS_PER_ADAPTER];
};

static struct config_entry config_data[MAX_DVB_ADAPTERS];

static struct cx24116_core *core[MAX_DVB_ADAPTERS];

#if 0 //DVB_API_VERSION < 5
static int cx24116_lookup_fecmod(struct cx24116_state *state, enum dvbfe_delsys d, enum dvbfe_modulation m, enum dvbfe_fec f)
{
	unsigned int i;
	int ret = -EOPNOTSUPP;

	dprintk(20, "delsys %d, modulation %d, fec = %d\n", d, m, f);
	for (i = 0; i < sizeof(cx24116_MODFEC_MODES) / sizeof(struct cx24116_modfec); i++)
	{
		if ((d == cx24116_MODFEC_MODES[i].delsys)
		&&  (m == cx24116_MODFEC_MODES[i].modulation)
		&&  (f == cx24116_MODFEC_MODES[i].fec))
		{
			ret = i;
			break;
		}
	}
	dprintk(100, "%s < ret = %d\n", __func__, ret);
	return ret;
}
#endif

/* *****************************
 *  Toggle tuner enable (reset)
 */
static int cx24116_reset(struct cx24116_state *state)
{
	const struct cx24116_config *cfg = state->config;
#if defined(UFS922)
	int frontend_nr = (cfg->i2c_bus == 1 ? 0 : 1);
#else
	int frontend_nr = 0;
#endif

	dprintk(100, "%s >\n", __func__);

	if (cfg->tuner_enable_pin != NULL)
	{
		stpio_set_pin(cfg->tuner_enable_pin, !cfg->tuner_enable_act);
		msleep(1);
		stpio_set_pin(cfg->tuner_enable_pin, cfg->tuner_enable_act);
	}
	dprintk(20, "Frontend %d has been reset\n", frontend_nr);

	// Set LNB power off by default
	lnbh23_writereg_lnb_supply(state, cfg->disable);
	dprintk(100, "%s <\n", __func__);
	return 0;
}

/* *********************************
 *  Write a single CX24116 register
 */
static int cx24116_writereg(struct cx24116_state *state, int reg, int data)
{
	u8 buf[] = { reg, data };
	struct i2c_msg msg =
	{ .addr = state->config->i2c_addr, .flags = 0, .buf = buf, .len = 2 };
	int err = 0;

	dprintk(100, "%s > addr 0x%02x, reg 0x%02x, value 0x%02x\n", __func__, state->config->i2c_addr, reg, data);

	err = i2c_transfer(state->config->i2c_adap, &msg, 1);
	if (err != 1)
	{
		dprintk(1, "%s: writereg error(err == %i, reg == 0x%02x, data == 0x%02x), bus %d\n", __func__, err, reg, data, state->config->i2c_bus);
		err = -EREMOTEIO;
	}
	return err;
}

/* *****************************************************************
 *  Bulk byte writes to a single I2C address, for 32k firmware load
 */
static int cx24116_writeregN(const struct cx24116_state *state, int reg, const u8 *data, u16 len)
{
	int ret = -EREMOTEIO;
	struct i2c_msg msg;
	u8 *buf;
	u32 timeout;

#ifndef CONFIG_I2C_STM
	struct i2c_algo_bit_data *algo_data = state->config->i2c_adap->algo_data;
	int udelay = algo_data->udelay;
#endif

	buf = kmalloc(len + 1, GFP_KERNEL);
	if (buf == NULL)
	{
		dprintk(1, "%s: Unable to kmalloc\n", __func__);
		ret = -ENOMEM;
		goto error;
	}
	*(buf) = reg;
	memcpy(buf + 1, data, len);

	msg.addr = state->config->i2c_addr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = len + 1;

	dprintk(70, "%s: reg 0x%02x, len = %d, addr 0x%02x, bus %d\n", __func__, reg, len, state->config->i2c_addr, state->config->i2c_bus);
	ret = i2c_transfer(state->config->i2c_adap, &msg, 1);

#ifndef CONFIG_I2C_STM
#else
	timeout = state->config->i2c_adap->timeout;
	dprintk(20, "Old timeout %d\n", timeout);
	state->config->i2c_adap->timeout = 20;
#endif
	if (ret != 1)
	{
		dprintk(1, "%s: writereg error(err == %i, reg == 0x%02x\n", __func__, ret, reg);
		ret = -EREMOTEIO;
	}
#ifndef CONFIG_I2C_STM
#else
	state->config->i2c_adap->timeout = timeout;
#endif

error:
	kfree(buf);
	return ret;
}

/* ******************************************
 *  Read one CX24116 register
 */
static int cx24116_readreg(struct cx24116_state *state, u8 reg)
{
	int ret;
	u8 b0[] = { reg };
	u8 b1[] = { 0 };
	struct i2c_msg msg[] =
	{
		{ .addr = state->config->i2c_addr, .flags = 0,        .buf = b0, .len = 1 },
		{ .addr = state->config->i2c_addr, .flags = I2C_M_RD, .buf = b1, .len = 1 }
	};

	dprintk(100, "%s > flag = %d\n", __func__, I2C_M_RD);
	dprintk(50, "%s: i2c_addr = 0x%02x\n", __func__, state->config->i2c_addr);
	ret = i2c_transfer(state->config->i2c_adap, msg, 2);

	if (ret != 2)
	{
		dprintk(1, "%s: reg=0x%x (error=%d)\n", __func__, reg, ret);
		return ret;
	}
	dprintk(100, "%s: < reg 0x%02x, value 0x%02x\n", __func__, reg, b1[0]);
	return b1[0];
}

/* **********************************************
 *
 */
static int cx24116_read_status(struct dvb_frontend *fe, fe_status_t *status)
{
	struct cx24116_state *state = fe->demodulator_priv;

	int lock = cx24116_readreg(state, CX24116_REG_STATUS);

	dprintk(100, "%s > status = 0x%02x\n", __func__, lock);

	*status = 0;

	if (lock & 0x01)
	{
		*status |= FE_HAS_SIGNAL;
	}
	if (lock & 0x02)
	{
		*status |= FE_HAS_CARRIER;
	}
	if (lock & 0x04)
	{
		*status |= FE_HAS_VITERBI;
	}
	if (lock & 0x08)
	{
//		dprintk(20, "Demodulator is locked\n");
		*status |= FE_HAS_SYNC | FE_HAS_LOCK;
	}
	return 0;
}

/* **********************************************
 *
 */
#define FE_IS_TUNED (FE_HAS_SIGNAL + FE_HAS_LOCK + FE_HAS_CARRIER + FE_HAS_VITERBI)
static int cx24116_is_tuned(struct dvb_frontend *fe)
{
	fe_status_t tunerstat;

	cx24116_read_status(fe, &tunerstat);
	return ((tunerstat & FE_IS_TUNED) == FE_IS_TUNED);
}

#if 0 // DVB_API_VERSION < 5
/* **********************************************
 *
 */
static int cx24116_get_tune_settings(struct dvb_frontend *fe, struct dvb_frontend_tune_settings *fetunesettings)
{
	struct cx24116_state *state = fe->demodulator_priv;

	dprintk(100, "%s > \n", __func__);

	/* FIXME: Hab das jetzt mal eingebaut, da bei SET_FRONTEND DVB-S2 nicht gehandelt wird.
	 * Im Prinzip setze ich hier die Werte aus SET_FRONTEND (siehe dvb-core) mal von min_delay
	 * abgesehen.
	 * DIES MUSS MAL BEOBACHTET WERDEN
	 */
	fetunesettings->step_size = state->dcur.symbol_rate / 16000;
	fetunesettings->max_drift = state->dcur.symbol_rate / 2000;
	fetunesettings->min_delay_ms = 500; // For pilot auto tune
	dprintk(100, "%s < \n", __func__);
	return 0;
}
#endif

/* **********************************************
 *
 */
static int cx24116_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	struct cx24116_state *state = fe->demodulator_priv;

	dprintk(100, "%s >\n", __func__);

	*ber = (cx24116_readreg(state, CX24116_REG_BER24) << 24)
	     | (cx24116_readreg(state, CX24116_REG_BER16) << 16)
	     | (cx24116_readreg(state, CX24116_REG_BER8)  <<  8)
	     |  cx24116_readreg(state, CX24116_REG_BER0);
	dprintk(100, "%s <\n", __func__);
	return 0;
}

/* **********************************************
 *
 */
static int cx24116_read_snr(struct dvb_frontend *fe, u16 *snr)
{
	struct cx24116_state *state = fe->demodulator_priv;

	u8 snr_reading;
	static const u32 snr_tab[] =  /* 10 x Table (rounded up) */
	{
		0x00000, 0x0199a, 0x03333, 0x04ccd,
		0x06667, 0x08000, 0x0999a, 0x0b333,
		0x0cccd, 0x0e667, 0x10000, 0x1199a,
		0x13333, 0x14ccd, 0x16667, 0x18000
	};

	dprintk(100, "%s >\n", __func__);

	snr_reading = cx24116_readreg(state, CX24116_REG_QUALITY);

	if (snr_reading >= 0xa0 /* 100% */)
	{
		*snr = 0xffff;
	}
	else
	{
		*snr =  snr_tab[(snr_reading & 0xf0) >> 4]
		     + (snr_tab[(snr_reading & 0x0f)] >> 4);
	}
	dprintk(100, "%s < SNR (raw / cooked) = (0x%02x / 0x%04x)\n", __func__, snr_reading, *snr);
	return 0;
}

/* **********************************************
 *
 */
static int cx24116_read_ucblocks(struct dvb_frontend *fe, u32 *ucblocks)
{
	struct cx24116_state *state = fe->demodulator_priv;

	dprintk(100, "%s >\n", __func__);

	*ucblocks = (cx24116_readreg(state, CX24116_REG_UCB8) << 8)
	          |  cx24116_readreg(state, CX24116_REG_UCB0);
	dprintk(100, "%s < ucblocks = %d\n", __func__, *ucblocks);
	return 0;
}

/* **********************************************
 *
 */
#if 0 //DVB_API_VERSION < 5

#define FEC_S2_QPSK_1_2  (fe_code_rate_t)(FEC_AUTO + 1)
#define FEC_S2_QPSK_2_3  (fe_code_rate_t)(FEC_S2_QPSK_1_2 + 1)
#define FEC_S2_QPSK_3_4  (fe_code_rate_t)(FEC_S2_QPSK_2_3 + 1)
#define FEC_S2_QPSK_5_6  (fe_code_rate_t)(FEC_S2_QPSK_3_4 + 1)
#define FEC_S2_QPSK_7_8  (fe_code_rate_t)(FEC_S2_QPSK_5_6 + 1)
#define FEC_S2_QPSK_8_9  (fe_code_rate_t)(FEC_S2_QPSK_7_8 + 1)
#define FEC_S2_QPSK_3_5  (fe_code_rate_t)(FEC_S2_QPSK_8_9 + 1)
#define FEC_S2_QPSK_4_5  (fe_code_rate_t)(FEC_S2_QPSK_3_5 + 1)
#define FEC_S2_QPSK_9_10 (fe_code_rate_t)(FEC_S2_QPSK_4_5 + 1)
#define FEC_S2_8PSK_1_2  (fe_code_rate_t)(FEC_S2_QPSK_9_10 + 1)
#define FEC_S2_8PSK_2_3  (fe_code_rate_t)(FEC_S2_8PSK_1_2 + 1)
#define FEC_S2_8PSK_3_4  (fe_code_rate_t)(FEC_S2_8PSK_2_3 + 1)
#define FEC_S2_8PSK_5_6  (fe_code_rate_t)(FEC_S2_8PSK_3_4 + 1)
#define FEC_S2_8PSK_7_8  (fe_code_rate_t)(FEC_S2_8PSK_5_6 + 1)
#define FEC_S2_8PSK_8_9  (fe_code_rate_t)(FEC_S2_8PSK_7_8 + 1)
#define FEC_S2_8PSK_3_5  (fe_code_rate_t)(FEC_S2_8PSK_8_9 + 1)
#define FEC_S2_8PSK_4_5  (fe_code_rate_t)(FEC_S2_8PSK_3_5 + 1)
#define FEC_S2_8PSK_9_10 (fe_code_rate_t)(FEC_S2_8PSK_4_5 + 1)

/* Why isn't this a generic frontend core function? */
static enum dvbfe_fec cx24116_convert_oldfec_to_new(enum fe_code_rate c)
{
	enum dvbfe_fec fec = DVBFE_FEC_NONE;

	switch (c)
	{
		case FEC_NONE:
		{
			fec = DVBFE_FEC_NONE;
			break;
		}
		case FEC_1_2:
		{
			fec = DVBFE_FEC_1_2;
			break;
		}
		case FEC_2_3:
		{
			fec = DVBFE_FEC_2_3;
			break;
		}
		case FEC_3_4:
		{
			fec = DVBFE_FEC_3_4;
			break;
		}
		case FEC_4_5:
		{
			fec = DVBFE_FEC_4_5;
			break;
		}
		case FEC_5_6:
		{
			fec = DVBFE_FEC_5_6;
			break;
		}
		case FEC_6_7:
		{
			fec = DVBFE_FEC_6_7;
			break;
		}
		case FEC_7_8:
		{
			fec = DVBFE_FEC_7_8;
			break;
		}
		case FEC_8_9:
		{
			fec = DVBFE_FEC_8_9;
			break;
		}
		case FEC_AUTO:
		{
			fec = DVBFE_FEC_AUTO;
			break;
		}
//enigma2
		case FEC_S2_QPSK_1_2:
		{
			fec = DVBFE_FEC_1_2;
			break;
		}
		case FEC_S2_QPSK_2_3:
		{
			fec = DVBFE_FEC_2_3;
			break;
		}
		case FEC_S2_QPSK_3_4:
		{
			fec = DVBFE_FEC_3_4;
			break;
		}
		case FEC_S2_QPSK_5_6:
		{
			fec = DVBFE_FEC_5_6;
			break;
		}
		case FEC_S2_QPSK_7_8:
		{
			fec = DVBFE_FEC_7_8;
			break;
		}
		case FEC_S2_QPSK_8_9:
		{
			fec = DVBFE_FEC_8_9;
			break;
		}
		case FEC_S2_QPSK_3_5:
		{
			fec = DVBFE_FEC_3_5;
			break;
		}
		case FEC_S2_QPSK_4_5:
		{
			fec = DVBFE_FEC_4_5;
			break;
		}
		case FEC_S2_QPSK_9_10:
		{
			fec = DVBFE_FEC_9_10;
			break;
		}
		case FEC_S2_8PSK_1_2:
		{
			fec = DVBFE_FEC_1_2;
			break;
		}
		case FEC_S2_8PSK_2_3:
		{
			fec = DVBFE_FEC_2_3;
			break;
		}
		case FEC_S2_8PSK_3_4:
		{
			fec = DVBFE_FEC_3_4;
			break;
		}
		case FEC_S2_8PSK_5_6:
		{
			fec = DVBFE_FEC_5_6;
			break;
		}
		case FEC_S2_8PSK_7_8:
		{
			fec = DVBFE_FEC_7_8;
			break;
		}
		case FEC_S2_8PSK_8_9:
		{
			fec = DVBFE_FEC_8_9;
			break;
		}
		case FEC_S2_8PSK_3_5:
		{
			fec = DVBFE_FEC_3_5;
			break;
		}
		case FEC_S2_8PSK_4_5:
		{
			fec = DVBFE_FEC_4_5;
			break;
		}
		case FEC_S2_8PSK_9_10:
		{
			fec = DVBFE_FEC_9_10;
			break;
		}
		default:
		{
			fec = FEC_NONE;
			break;
		}
	}
	return fec;
}

/* **********************************************
 *
 */
/* Why isn't this a generic frontend core function? */
static enum dvbfe_fec cx24116_convert_newfec_to_old(enum dvbfe_fec c, enum dvbfe_delsys d, enum dvbfe_modulation m)
{
	fe_code_rate_t fec;

	dprintk(100, "%s >\n", __func__);
	dprintk(50, "delsys %d, fec %d, mod %d\n", d, c, m);

	if (d == DVBFE_DELSYS_DVBS)
	{
		switch (c)
		{
			case DVBFE_FEC_NONE:
			{
				fec = FEC_NONE;
				break;
			}
			case DVBFE_FEC_1_2:
			{
				fec = FEC_1_2;
				break;
			}
			case DVBFE_FEC_2_3:
			{
				fec = FEC_2_3;
				break;
			}
			case DVBFE_FEC_3_4:
			{
				fec = FEC_3_4;
				break;
			}
			case DVBFE_FEC_4_5:
			{
				fec = FEC_4_5;
				break;
			}
			case DVBFE_FEC_5_6:
			{
				fec = FEC_5_6;
				break;
			}
			case DVBFE_FEC_6_7:
			{
				fec = FEC_6_7;
				break;
			}
			case DVBFE_FEC_7_8:
			{
				fec = FEC_7_8;
				break;
			}
			case DVBFE_FEC_8_9:
			{
				fec = FEC_8_9;
				break;
			}
			default:
			{
				fec = FEC_NONE;
				break;
			}
		}
	}
	else
	{
		if (m == DVBFE_MOD_QPSK)
		{
			switch (c)
			{
				case DVBFE_FEC_1_2:
				{
					fec = FEC_S2_QPSK_1_2;
					break;
				}
				case DVBFE_FEC_2_3:
				{
					fec = FEC_S2_QPSK_2_3;
					break;
				}
				case DVBFE_FEC_3_4:
				{
					fec = FEC_S2_QPSK_3_4;
					break;
				}
				case DVBFE_FEC_5_6:
				{
					fec = FEC_S2_QPSK_5_6;
					break;
				}
				case DVBFE_FEC_7_8:
				{
					fec = FEC_S2_QPSK_7_8;
					break;
				}
				case DVBFE_FEC_8_9:
				{
					fec = FEC_S2_QPSK_8_9;
					break;
				}
				case DVBFE_FEC_3_5:
				{
					fec = FEC_S2_QPSK_3_5;
					break;
				}
				case DVBFE_FEC_4_5:
				{
					fec = FEC_S2_QPSK_4_5;
					break;
				}
				case DVBFE_FEC_9_10:
				{
					fec = FEC_S2_QPSK_9_10;
					break;
				}
				default:
				{
					fec = FEC_NONE;
					break;
				}
			}
		}
		else
		{
			switch (c)
			{
				case DVBFE_FEC_1_2:
				{
					fec = FEC_S2_8PSK_1_2;
					break;
				}
				case DVBFE_FEC_2_3:
				{
					fec = FEC_S2_8PSK_2_3;
					break;
				}
				case DVBFE_FEC_3_4:
				{
					fec = FEC_S2_8PSK_3_4;
					break;
				}
				case DVBFE_FEC_5_6:
				{
					fec = FEC_S2_8PSK_5_6;
					break;
				}
				case DVBFE_FEC_7_8:
				{
					fec = FEC_S2_8PSK_7_8;
					break;
				}
				case DVBFE_FEC_8_9:
				{
					fec = FEC_S2_8PSK_8_9;
					break;
				}
				case DVBFE_FEC_3_5:
				{
					fec = FEC_S2_8PSK_3_5;
					break;
				}
				case DVBFE_FEC_4_5:
				{
					fec = FEC_S2_8PSK_4_5;
					break;
				}
				case DVBFE_FEC_9_10:
				{
					fec = FEC_S2_8PSK_9_10;
					break;
				}
				default:
				{
					fec = FEC_NONE;
					break;
				}
			}
		}
	}
	dprintk(50, "%s: fec = %d\n", __func__, fec);
	dprintk(100, "%s <\n", __func__);
	return fec;
}

// Hack
static enum dvbfe_modulation cx24116_get_modulation_from_fec(enum fe_code_rate c)
{
	switch (c)
	{
		case FEC_NONE:
		case FEC_1_2:
		case FEC_2_3:
		case FEC_3_4:
		case FEC_4_5:
		case FEC_5_6:
		case FEC_6_7:
		case FEC_7_8:
		case FEC_8_9:
		case FEC_AUTO:
		{
			return DVBFE_MOD_QPSK;
//			break;
		}
		case FEC_S2_QPSK_1_2:
		case FEC_S2_QPSK_2_3:
		case FEC_S2_QPSK_3_4:
		case FEC_S2_QPSK_5_6:
		case FEC_S2_QPSK_7_8:
		case FEC_S2_QPSK_8_9:
		case FEC_S2_QPSK_3_5:
		case FEC_S2_QPSK_4_5:
		case FEC_S2_QPSK_9_10:
		{
			return DVBFE_MOD_QPSK;
//			break;
		}
		case FEC_S2_8PSK_1_2:
		case FEC_S2_8PSK_2_3:
		case FEC_S2_8PSK_3_4:
		case FEC_S2_8PSK_5_6:
		case FEC_S2_8PSK_7_8:
		case FEC_S2_8PSK_8_9:
		case FEC_S2_8PSK_3_5:
		case FEC_S2_8PSK_4_5:
		case FEC_S2_8PSK_9_10:
		{
			return DVBFE_MOD_8PSK;
//			break;
		}
		default:
		{
			return DVBFE_MOD_QPSK;
//			break;
		}
	}
}

/* Why isn't this a generic frontend core function? */
static int cx24116_create_new_qpsk_feparams(struct dvb_frontend *fe, struct dvb_frontend_parameters *pFrom, struct dvbfe_params *pTo)
{
	int ret = 0;
	int rolloff;
	enum dvbfe_rolloff Rollofftab[4] =
	{
		DVBFE_ROLLOFF_35, DVBFE_ROLLOFF_25, DVBFE_ROLLOFF_20,
		DVBFE_ROLLOFF_UNKNOWN
	};

	dprintk(100, "%s >\n", __func__);

	memset(pTo, 0, sizeof(struct dvbfe_params));
	dprintk(20, "(vor Umwandlung) FREQ %i SR %i FEC %x \n", pFrom->frequency, pFrom->u.qpsk.symbol_rate, pFrom->u.qpsk.fec_inner);
	pTo->frequency = pFrom->frequency;
	// HACK from E2: Bits 2..3 are rolloff in DVB-S2; inversion can have normally values 0,1,2
	pTo->inversion = pFrom->inversion & 3;
	// modulation in bits 23-16s
	if (pFrom->u.qpsk.fec_inner > FEC_AUTO)
	{
		switch (pFrom->inversion & 0xc)
		{
			default:  // unknown rolloff
			case 0:   // 0.35
			{
				rolloff = 0;
				break;
			}
			case 4:  // 0.25
			{
				rolloff = 1;
				break;
			}
			case 8:  // 0.20
			{
				rolloff = 2;
				break;
			}
		}
		if (pFrom->u.qpsk.fec_inner > FEC_S2_QPSK_9_10)
		{
			switch (pFrom->inversion & 0x30)
			{
				case 0:  // pilot off
				{
					pilot = 0;
					break;
				}
				case 0x10:  // pilot on
				{
					pilot = 1;
					break;
				}
				case 0x20:  // pilot auto
				{
					pilot = 2;
					break;
				}
			}
		}
		pTo->delivery = DVBFE_DELSYS_DVBS2;
		pTo->delsys.dvbs.rolloff = Rollofftab[rolloff & 3];
	}
	else
	{
		pTo->delivery = DVBFE_DELSYS_DVBS;
//FIXME: ist das richtig?
		pTo->delsys.dvbs.rolloff = DVBFE_ROLLOFF_35;
	}

	pTo->delsys.dvbs.modulation = cx24116_get_modulation_from_fec(pFrom->u.qpsk.fec_inner);

	pTo->delsys.dvbs.symbol_rate = pFrom->u.qpsk.symbol_rate;
	pTo->delsys.dvbs.fec = cx24116_convert_oldfec_to_new(pFrom->u.qpsk.fec_inner & 255);

	dprintk(20, "FREQ %i SR %i FEC %x FECN %08x MOD %i DS %i \n",
		pTo->frequency, pTo->delsys.dvbs.symbol_rate,
		pFrom->u.qpsk.fec_inner,
		pTo->delsys.dvbs.fec, pTo->delsys.dvbs.modulation, pTo->delivery);

	/* I guess we could do more validation on the old fe params and return error */
	return ret;
}

/* Why isn't this a generic frontend core function? */
static int cx24116_create_old_qpsk_feparams(struct dvb_frontend *fe, struct dvbfe_params *pFrom, struct dvb_frontend_parameters *pTo)
{
	int ret = 0;

	dprintk(100, "%s >\n", __func__);

	memset(pTo, 0, sizeof(struct dvb_frontend_parameters));

	pTo->frequency = pFrom->frequency;
	pTo->inversion = pFrom->inversion;

	switch (pFrom->delivery)
	{
		case DVBFE_DELSYS_DVBS:
		{
			pTo->u.qpsk.fec_inner = cx24116_convert_newfec_to_old(pFrom->delsys.dvbs.fec, DVBFE_DELSYS_DVBS, pFrom->delsys.dvbs.modulation);
			pTo->u.qpsk.symbol_rate = pFrom->delsys.dvbs.symbol_rate;
			break;
		}
		case DVBFE_DELSYS_DVBS2:
		{
			pTo->u.qpsk.fec_inner = cx24116_convert_newfec_to_old(pFrom->delsys.dvbs2.fec, DVBFE_DELSYS_DVBS2, pFrom->delsys.dvbs2.modulation);
			pTo->u.qpsk.symbol_rate = pFrom->delsys.dvbs2.symbol_rate;
			break;
		}
		default:
		{
			ret = -1;
		}
	}
	/* I guess we could do more validation on the old fe params and return error */
	return ret;
}
#endif

/* Wait for LNB */
static int cx24116_wait_for_lnb(struct dvb_frontend *fe)
{
	struct cx24116_state *state = fe->demodulator_priv;
	int i;

	dprintk(100, "%s > qstatus = 0x%02x\n", __func__, cx24116_readreg(state, CX24116_REG_QSTATUS));

	/* Wait for up to 500 ms */
	for (i = 0; i < 50; i++)
	{
		if (cx24116_readreg(state, CX24116_REG_QSTATUS) & 0x20)
		{
			return 0;
		}
		msleep(10);
	}
	dprintk(1, "%s < LNB not ready\n", __func__);
	return -ETIMEDOUT;  /* -EBUSY ? */
}

static int cx24116_load_firmware(struct dvb_frontend *fe, const struct firmware *fw);

#ifdef EARLY_FW_DOWNLOAD
static void cx24116_load_fw(const struct firmware *fw, void *context)
{
	struct cx24116_state *state = context;
	int syschipid, gotreset;

	dprintk(100, "\n%s(%p, %p)\n\n", __func__, fw, context);
	gotreset = cx24116_readreg(state, 0x20); // implicit watchdog
	syschipid = cx24116_readreg(state, 0x94);
	if (fw != NULL)
	{
		int ret = cx24116_load_firmware(&state->frontend, fw);
		//release_firmware (fw);
		if (ret)
		{
			dprintk(1, "%s: Writing firmware to device failed\n", __func__);
		}
	}
	else
	{
		dprintk(1, "%s: Firmware not found or timed out\n", __func__);
	}
	dprintk(100, "%s < ...done\n\n", __func__);
	up(&state->fw_load_sem);
}
#endif

static int cx24116_firmware_ondemand(struct dvb_frontend *fe)
{
	struct cx24116_state *state = fe->demodulator_priv;
	int ret = 0;
	int syschipid, gotreset;
#if defined(UFS922)
	int frontend_nr = (state->config->i2c_bus == 1 ? 0 : 1);
#else
	int frontend_nr = 0;
#endif
	const struct firmware *fw;

	dprintk(100, "%s >\n", __func__);

	/* lock semaphore to ensure data consistency */
	if (down_trylock(&state->fw_load_sem))
	{
		dprintk(50, "%s < FW upload already running\n", __func__);
		return 0;
	}
	gotreset = cx24116_readreg(state, 0x20);  // implicit watchdog
	syschipid = cx24116_readreg(state, 0x94);

	dprintk(70, "gotreset %d, syschipid 0x%02x\n", gotreset, syschipid);

	if ((gotreset || syschipid != 5) && (state->not_responding <= cMaxError))
	{
		state->not_responding++;

		dprintk(20, "Start Firmware upload on frontend %d ... %d\n", frontend_nr, state->not_responding);

		ret = request_firmware(&fw, CX24116_DEFAULT_FIRMWARE, &state->config->i2c_adap->dev);
		dprintk(20, "Waiting for firmware (%s) upload(2)...\n", CX24116_DEFAULT_FIRMWARE);
		if (ret)
		{
			dprintk(1, "%s: No firmware uploaded (%d - timeout or file not found?)\n", __func__, ret);
			up(&state->fw_load_sem);
			return ret;
		}
		ret = cx24116_load_firmware(fe, fw);
		if (ret)
		{
			dprintk(1, "%s: Writing firmware to device failed\n", __func__);
		}
		release_firmware(fw);
		dprintk(20, "%s: Firmware upload %s\n", __func__, ret == 0 ? "complete" : "failed");
	}
	else
	{
		dprintk(30, "%s: Firmware upload not needed\n", __func__);
	}
	/* unlock semaphore */
	up(&state->fw_load_sem);
	dprintk(100, "%s <\n", __func__);
	return ret;
}

#ifdef FIXME
static int fw_loader_thread(void *data)
{
	struct dvb_frontend *fe = data;
	struct cx24116_state *state = fe->demodulator_priv;

	daemonize("cx24116_fw_loader");
	allow_signal(SIGTERM);

	dprintk(20, "FW loader thread (%d)\n", state->thread_id);
	cx24116_firmware_ondemand(fe);
	state->thread_id = 0;
	return 0;
}
#endif

/* Take a basic firmware command structure, format it and forward it for processing */
static int cx24116_cmd_execute(struct dvb_frontend *fe, struct cx24116_cmd *cmd)
{
	struct cx24116_state *state = fe->demodulator_priv;
	unsigned int i;
	int ret;

	dprintk(100, "%s >\n", __func__);

	/* Load the firmware if required */
	if ((ret = cx24116_firmware_ondemand(fe)) != 0)
	{
		dprintk(1, "%s: Unable initialize the firmware\n", __func__);
		return ret;
	}

	for (i = 0; i < (sizeof(cx24116_COMMANDS) / sizeof(struct firmware_cmd)); i++)
	{
		if (cx24116_COMMANDS[i].id == CMD_MAX)
		{
			return -EINVAL;
		}
		if (cx24116_COMMANDS[i].id == cmd->id)
		{
			cmd->len = cx24116_COMMANDS[i].len;
			break;
		}
	}
	cmd->args[0x00] = cmd->id;

	/* Write the command */
	for (i = 0; i < cmd->len; i++)
	{
		if (i < cmd->len)
		{
			dprintk(50, "%s: 0x%02x == 0x%02x\n", __func__, i, cmd->args[i]);
			cx24116_writereg(state, i, cmd->args[i]);
		}
		else
		{
			cx24116_writereg(state, i, 0);
		}
	}
	/* Start execution and wait for cmd to terminate */
	cx24116_writereg(state, 0x1f, 0x01);
	while (cx24116_readreg(state, 0x1f))
	{
		msleep(10);
		if (i++ > 64 /*128 */)
		{
			/* Avoid looping forever if the firmware does no respond */
			dprintk(1, "%s: Firmware not responding\n", __func__);
			return -EREMOTEIO;
		}
	}
	return 0;
}

/* XXX Actual scale is non-linear */
static int cx24116_read_signal_strength(struct dvb_frontend *fe, u16 *signal_strength)
{
	struct cx24116_state *state = fe->demodulator_priv;
	u16 sig_reading;

	dprintk(100, "%s >\n", __func__);

	if (useUnknown == 1)
	{
		/* Dagobert: fixme: this does not work currently. Do not know why, but
		 * register 0x9d is always 0x2f and 0x9e is always 0x00. Mysteriously,
		 * on ufs922 this works great and using the orig soft the regs
		 * have correct values...
		 */
		sig_reading = ((cx24116_readreg(state, CX24116_REG_STATUS) & 0xc0) << 8)
		            |  (cx24116_readreg(state, CX24116_REG_SIGNAL) << 6);
	}
	else
	{
		sig_reading = (cx24116_readreg(state, CX24116_REG_STATUS) & 0xc0)
		            | (cx24116_readreg(state, CX24116_REG_SIGNAL) << 6);
	}
	*signal_strength = 0 - sig_reading;
	dprintk(100, "%s < Signal strength (raw / cooked) = (0x%04x / 0x%04x)\n", __func__, sig_reading, *signal_strength);
	return 0;
}

static int cx24116_load_firmware(struct dvb_frontend *fe, const struct firmware *fw)
{
	struct cx24116_state *state = fe->demodulator_priv;
	struct cx24116_cmd cmd;
	int ret;

	dprintk(100, "%s >\n", __func__);
	dprintk(20, "Firmware size is %zu bytes (%02x %02x .. %02x %02x)\n", fw->size, fw->data[0], fw->data[1], fw->data[fw->size - 2], fw->data[fw->size - 1]);

	/* Toggle 88x SRST pin to reset demod */
	cx24116_reset(state);

	/* Init PLL */
	if (useUnknown == 0)
	{
		cx24116_writereg(state, 0xE5, 0x00);
	}
	cx24116_writereg(state, 0xF1, 0x08);
	if (useUnknown == 0)
	{
		cx24116_writereg(state, 0xF2, 0x13);
	}
	else
	{
		cx24116_writereg(state, 0xF2, 0x12);
	}
	/* Start PLL */
	if (useUnknown == 0)
	{
		cx24116_writereg(state, 0xe0, 0x03);
		cx24116_writereg(state, 0xe0, 0x00);
	}
	/* Begin the firmware load process */
	/* Prepare the demod, load the firmware, cleanup after load */
	cx24116_writereg(state, 0xF3, 0x46);
	cx24116_writereg(state, 0xF9, 0x00);

	cx24116_writereg(state, 0xF0, 0x03);
	cx24116_writereg(state, 0xF4, 0x81);
	cx24116_writereg(state, 0xF5, 0x00);
	cx24116_writereg(state, 0xF6, 0x00);

	/* write the entire firmware as one transaction */
	cx24116_writeregN(state, 0xF7, (u8 *)fw->data, fw->size);

	cx24116_writereg(state, 0xF4, 0x10);
	cx24116_writereg(state, 0xF0, 0x00);
	cx24116_writereg(state, 0xF8, 0x06);

	/* Firmware CMD 10: Main init */
	cmd.id = CMD_SET_VCO;
	cmd.args[0x01] = 0x05;
	if (useUnknown == 0)
	{
		cmd.args[0x02] = 0xdc;
		cmd.args[0x03] = 0xda;
		cmd.args[0x04] = 0xae;
		cmd.args[0x05] = 0xaa;
	}
	else
	{
		cmd.args[0x02] = 0x8d;
		cmd.args[0x03] = 0xdc;
		cmd.args[0x04] = 0xb8;
		cmd.args[0x05] = 0x5e;
	}
	cmd.args[0x06] = 0x04;
	cmd.args[0x07] = 0x9d;
	cmd.args[0x08] = 0xfc;
	cmd.args[0x09] = 0x06;
	ret = cx24116_cmd_execute(fe, &cmd);
	if (ret != 0)
	{
		return ret;
	}
	cx24116_writereg(state, 0x9d, 0x00);

	/* Firmware CMD 14: Tuner Init */
	cmd.id = CMD_TUNERINIT;
	cmd.args[0x01] = 0x00;
	cmd.args[0x02] = 0x00;
	ret = cx24116_cmd_execute(fe, &cmd);
	if (ret != 0)
	{
		return ret;
	}
	cx24116_writereg(state, 0xe5, 0x00);

	/* Firmware CMD 13: MPEG/TS output config */
	cmd.id = CMD_MPEGCONFIG;
	cmd.args[0x01] = 0x01;
	cmd.args[0x02] = 0x70;
	cmd.args[0x03] = 0x00;
	cmd.args[0x04] = 0x01;
	cmd.args[0x05] = 0x00;

	ret = cx24116_cmd_execute(fe, &cmd);
	if (ret != 0)
	{
		return ret;
	}
	if (useUnknown == 1)
	{
		cx24116_writereg(state, 0xe0, 0x08);
	}
	// Firmware CMD 20: LNB/Diseqc Config
	cmd.id = CMD_LNBCONFIG;

	cmd.args[1] = 0;
	if (useUnknown == 0)
	{
		cmd.args[2] = 0x10;
		cmd.args[3] = 0x00;
		cmd.args[4] = 0x8f;
		cmd.args[5] = 0x28;
		cmd.args[6] = 0x00;  // Disable tone burst. Temporarily enabled later
		cmd.args[7] = 0x01;
	}
	else
	{
		cmd.args[2] = 0x03;
		cmd.args[3] = 0x00;
		cmd.args[4] = 0x8e;
		cmd.args[5] = 0xff;
		cmd.args[6] = 0x02;
		cmd.args[7] = 0x01;
	}
	ret = cx24116_cmd_execute(fe, &cmd);
	if (ret != 0)
	{
		return ret;
	}
	state->not_responding = 0;
	return 0;
}

static int cx24116_set_tone(struct dvb_frontend *fe, fe_sec_tone_mode_t tone)
{
	struct cx24116_cmd cmd;
	int ret;

	dprintk(100, "%s > tone %s\n", __func__, tone ? "on" : "off");
	if ((tone != SEC_TONE_ON) && (tone != SEC_TONE_OFF))
	{
		dprintk(1, "%s: Invalid value: tone = %d\n", __func__, tone);
		return -EINVAL;
	}

	/* Wait for LNB ready */
	ret = cx24116_wait_for_lnb(fe);
	if (ret != 0)
	{
		return ret;
	}
	msleep(100);

	/* This is always done before the tone is set */
	cmd.id = CMD_SET_TONEPRE;
	cmd.args[0x01] = 0x00;
	ret = cx24116_cmd_execute(fe, &cmd);
	if (ret != 0)
	{
		return ret;
	}
	/* Now we set the tone */
	cmd.id = CMD_SET_TONE;
	cmd.args[0x01] = 0x00;
	cmd.args[0x02] = 0x00;
	switch (tone)
	{
		case SEC_TONE_ON:
		{
			dprintk(20, "Setting tone on\n");
			cmd.args[0x03] = 0x01;
			break;
		}
		case SEC_TONE_OFF:
		{
			dprintk(20, "Setting tone off\n");
			cmd.args[0x03] = 0x00;
			break;
		}
	}
	ret = cx24116_cmd_execute(fe, &cmd);
	msleep(100);
	return ret;
}

/* Initialise DiSEqC */
static int cx24116_diseqc_init(struct dvb_frontend *fe)
{
	struct cx24116_state *state = fe->demodulator_priv;

	/* Prepare a DiSEqC command */
	state->dsec_cmd.id = CMD_LNBSEND;

	/* DiSEqC burst */
	state->dsec_cmd.args[CX24116_DISEQC_BURST] = CX24116_DISEQC_MINI_A;

	/* Unknown */
	state->dsec_cmd.args[CX24116_DISEQC_ARG2_2] = 0x02;
	state->dsec_cmd.args[CX24116_DISEQC_ARG3_0] = 0x00;
	/* Continuation flag? */
	state->dsec_cmd.args[CX24116_DISEQC_ARG4_0] = 0x00;

	/* DiSEqC message length */
	state->dsec_cmd.args[CX24116_DISEQC_MSGLEN] = 0x00;

	/* Command length */
	state->dsec_cmd.len = CX24116_DISEQC_MSGOFS;
	return 0;
}

/* Send with derived burst (hack) / previous burst OR cache DiSEqC message */
static int cx24116_send_diseqc_msg(struct dvb_frontend *fe, struct dvb_diseqc_master_cmd *d)
{
	struct cx24116_state *state = fe->demodulator_priv;
	int i, ret;

	/* Dump DiSEqC message */
	if (paramDebug > 50)
	{
		dprintk(1, "%s (", __func__);
		for (i = 0; i < d->msg_len;)
		{
			printk("0x%02x", d->msg[i]);
			if (++i < d->msg_len)
			{
				printk(", ");
			}
		}
		printk(") dsec=%s\n", (dsec) ? "CACHE" : "HACK");
	}

	/* Validate length */
	if (d->msg_len > (CX24116_ARGLEN - CX24116_DISEQC_MSGOFS))
	{
		return -EINVAL;
	}
	/* DiSEqC message */
	for (i = 0; i < d->msg_len; i++)
	{
		state->dsec_cmd.args[CX24116_DISEQC_MSGOFS + i] = d->msg[i];
	}
	/* DiSEqC message length */
	state->dsec_cmd.args[CX24116_DISEQC_MSGLEN] = d->msg_len;

	/* Command length */
	state->dsec_cmd.len = CX24116_DISEQC_MSGOFS + state->dsec_cmd.args[CX24116_DISEQC_MSGLEN];

	if (dsec)
	{
		/* Return with command/message cached (diseqc_send_burst MUST follow) (dsec=CACHE) */
		return 0;
	}

	/* Hack: Derive burst from command else use previous burst */
	if (d->msg_len >= 4 && d->msg[2] == 0x38)
	{
		state->dsec_cmd.args[CX24116_DISEQC_BURST] = (d->msg[3] >> 2) & 1;
	}
	dprintk(50, "%s burst=%d\n", __func__, state->dsec_cmd.args[CX24116_DISEQC_BURST]);
	/* Wait for LNB ready */
	ret = cx24116_wait_for_lnb(fe);
	if (ret != 0)
	{
		return ret;
	}
	/* Wait for voltage/min repeat delay */
	msleep(100);

	/* Command */
	return cx24116_cmd_execute(fe, &state->dsec_cmd);
}

/* Send DiSEqC burst */
static int cx24116_diseqc_send_burst(struct dvb_frontend *fe, fe_sec_mini_cmd_t burst)
{
	struct cx24116_state *state = fe->demodulator_priv;
	int ret;

	dprintk(100, "%s > burst=%d dsec=%s\n", __func__, (int) burst, (dsec) ? "CACHE" : "HACK");

	/* DiSEqC burst */
	if (burst == SEC_MINI_A)
	{
		state->dsec_cmd.args[CX24116_DISEQC_BURST] = CX24116_DISEQC_MINI_A;
	}
	else if (burst == SEC_MINI_B)
	{
		state->dsec_cmd.args[CX24116_DISEQC_BURST] = CX24116_DISEQC_MINI_B;
	}
	else
	{
		return -EINVAL;
	}
	if (!dsec)
	{
		/* Return when using derived burst strategy (dsec=HACK) */
		return 0;
	}

	/* Wait for LNB ready */
	ret = cx24116_wait_for_lnb(fe);
	if (ret != 0)
	{
		return ret;
	}
	/* Wait for voltage/min repeat delay */
	msleep(100);

	/* Command */
	return cx24116_cmd_execute(fe, &state->dsec_cmd);
}

static int cx24116_sleep(struct dvb_frontend *fe)
{
	struct cx24116_state *state = fe->demodulator_priv;
	struct cx24116_cmd cmd;

	dprintk(100, "%s >\n", __func__);

	cmd.id = CMD_TUNERSLEEP;
	cmd.args[1] = 1;
	cx24116_cmd_execute(fe, &cmd);

	// Shutdown clocks
	cx24116_writereg(state, 0xea, 0xff);
	cx24116_writereg(state, 0xe1, 1);
	cx24116_writereg(state, 0xe0, 1);
	return 0;
}

static void cx24116_release(struct dvb_frontend *fe)
{
	struct cx24116_state *state = fe->demodulator_priv;

	dprintk(100, "%s >\n", __func__);

	if (state->config != NULL)
	{
		if (state->config->tuner_enable_pin != NULL)
		{
			stpio_free_pin(state->config->tuner_enable_pin);
		}
		if (state->config->lnb_enable_pin != NULL)
		{
			stpio_free_pin(state->config->lnb_enable_pin);
		}
		if (state->config->lnb_vsel_pin != NULL)
		{
			stpio_free_pin(state->config->lnb_vsel_pin);
		}
	}
	kfree(state->config);
	kfree(state);
}

static int cx24116_set_inversion(struct cx24116_state *state, fe_spectral_inversion_t inversion)
{
	dprintk(100, "%s > (inversion=%d)\n", __func__, inversion);

	switch (inversion)
	{
		case INVERSION_OFF:
		{
			dprintk(20, "Inversion is off\n");
			state->dnxt.inversion_val = 0x00;
			break;
		}
		case INVERSION_ON:
		{
			dprintk(20, "Inversion is on\n");
			state->dnxt.inversion_val = 0x04;
			break;
		}
		case INVERSION_AUTO:
		{
			dprintk(20, "Inversion is automatic\n");
			state->dnxt.inversion_val = 0x0C;
			break;
		}
		default:
		{
			dprintk(1, "%s: < ret=-EINVAL\n", __func__);
			return -EINVAL;
		}
	}
	state->dnxt.inversion = inversion;
	return 0;
}

#if 0 //DVB_API_VERSION < 5
static int cx24116_set_fec(struct cx24116_state *state, struct dvbfe_params *p)
{
	int ret = -1;

	dprintk(100, "%s >\n", __func__);

	switch (p->delivery)
	{
		case DVBFE_DELSYS_DVBS:
		{
			ret = cx24116_lookup_fecmod(state, p->delivery, p->delsys.dvbs.modulation, p->delsys.dvbs.fec);
			break;
		}
		case DVBFE_DELSYS_DVBS2:
		{
			ret = cx24116_lookup_fecmod(state, p->delivery, p->delsys.dvbs2.modulation, p->delsys.dvbs2.fec);
			break;
		}
		default:
		{
			dprintk(1, "%s < return -EOPNOTSUPP\n", __func__);
			ret = -EOPNOTSUPP;
		}
	}
	if (ret >= 0)
	{
		state->dnxt.fec_val = cx24116_MODFEC_MODES[ret].val;
		state->dnxt.fec_mask = cx24116_MODFEC_MODES[ret].mask;
		dprintk(20, "%s: fec/mask = 0x%02x/0x%02x\n", __func__, state->dnxt.fec_val, state->dnxt.fec_mask);

		state->dnxt.fec_numb = ret;
		ret = 0;
	}
	dprintk(100, "%s < ret = %d\n", __func__, ret);
	return ret;
}

static int cx24116_set_symbolrate(struct cx24116_state *state, struct dvbfe_params *p)
{
	int ret = 0;

	dprintk(100, "%s >\n", __func__);

#ifdef __TDT__
	switch (p->delivery)
	{
		case DVBFE_DELSYS_DVBS:
		{
			/* check if symbol rate is within limits */
			if ((p->delsys.dvbs.symbol_rate > state->frontend.ops.info.symbol_rate_max)
			||  (p->delsys.dvbs.symbol_rate < state->frontend.ops.info.symbol_rate_min))
			{
				dprintk(1, "%s < symbol rate %u not in range (%u/%u)\n", __func__, p->delsys.dvbs.symbol_rate, state->frontend.ops.info.symbol_rate_min, state->frontend.ops.info.symbol_rate_max);
				ret = -EOPNOTSUPP;
			}
			else
			{
				state->dnxt.symbol_rate = p->delsys.dvbs.symbol_rate;
			}
			break;
		}
		case DVBFE_DELSYS_DVBS2:
		{
			/* check if symbol rate is within limits */
			if ((p->delsys.dvbs2.symbol_rate > state->frontend.ops.info.symbol_rate_max)
			||  (p->delsys.dvbs2.symbol_rate < state->frontend.ops.info.symbol_rate_min))
			{
				dprintk(1, "%s < symbol rate %u not in range (%u/%u)\n", __func__, p->delsys.dvbs.symbol_rate, state->frontend.ops.info.symbol_rate_min, state->frontend.ops.info.symbol_rate_max);
				ret = -EOPNOTSUPP;
			}
			else
			{
				state->dnxt.symbol_rate = p->delsys.dvbs2.symbol_rate;
			}
			break;
		default:
		{
			dprintk(1, "%s < (return -ENOTSUPP)\n", __func__);
			ret = -EOPNOTSUPP;
		}
	}
	dprintk(20, "%s symbol_rate = %u\n", __func__, state->dnxt.symbol_rate);
#else
	switch (p->delivery)
	{
		case DVBFE_DELSYS_DVBS:
		{
			state->dnxt.symbol_rate = p->delsys.dvbs.symbol_rate;
			break;
		}
		case DVBFE_DELSYS_DVBS2:
		{
			state->dnxt.symbol_rate = p->delsys.dvbs2.symbol_rate;
			break;
		}
		default:
		{
			dprintk(1, "%s <(return -ENOTSUPP)\n", __func__);
			ret = -EOPNOTSUPP;
		}
	}
	dprintk(20, "symbol_rate = %d\n", state->dnxt.symbol_rate);

	/* check if symbol rate is within limits */
	if ((state->dnxt.symbol_rate > state->frontend.ops.info.symbol_rate_max)
	||  (state->dnxt.symbol_rate < state->frontend.ops.info.symbol_rate_min))
	{
		ret = -EOPNOTSUPP;
	}
#endif
	dprintk(100, "%s <\n", __func__);
	return ret;
}

static int cx24116_get_inversion(struct cx24116_state *state, fe_spectral_inversion_t *inversion)
{
	dprintk(100, "%s >\n", __func__);
	*inversion = state->dcur.inversion;
	return 0;
}

static int cx24116_get_params(struct dvb_frontend *fe, struct dvbfe_params *p)
{
	struct cx24116_state *state = fe->demodulator_priv;
	int ret = 0;
	s16 frequency_offset;
	s16 sr_offset;

	dprintk(100, "%s >\n", __func__);

	frequency_offset = (cx24116_readreg(state, 0x9f) << 8)
	                 +  cx24116_readreg(state, 0xa0);
	sr_offset = 0;  //(cx24116_readreg(state, 0xa1)<<8)+cx24116_readreg(state, 0xa2);

	p->frequency = state->dcur.frequency + frequency_offset;  // unit seems to be 2kHz

	if (cx24116_get_inversion(state, &p->inversion) != 0)
	{
		dprintk(1, "%s: Failed to get inversion status\n", __func__);
		return -EREMOTEIO;
	}
	p->delivery = state->dcur.delivery;

	switch (p->delivery)
	{
		case DVBFE_DELSYS_DVBS2:
		{
			p->delsys.dvbs2.fec = state->dcur.fec;
			p->delsys.dvbs2.symbol_rate = state->dcur.symbol_rate + sr_offset * 1000;
			break;
		}
		case DVBFE_DELSYS_DVBS:
		{
			p->delsys.dvbs.fec = state->dcur.fec;
			p->delsys.dvbs.symbol_rate = state->dcur.symbol_rate + sr_offset * 1000;
			break;
		}
		default:
		{
			ret = -ENOTSUPP;
		}
	}
	return ret;
}

/* Dagobert: enable highSR dynamically */
#define TSMergerBaseAddress 0x19242000
#define TS_1394_CFG         0x0810

unsigned long reg_tsm_config = 0;

static int cx24116_set_params(struct dvb_frontend *fe, struct dvbfe_params *p)
{
	struct cx24116_state *state = fe->demodulator_priv;
	struct cx24116_cmd cmd;
	int ret, i, above30msps;
	u8 status, retune = 1;

	dprintk(100, "%s >\n", __func__);

//FIXME: Das mit den cx24116_tunesettings dnxt und dcur ist doch auch
//totaler quatsch. warum merke ich mir nicht dvbfe_params???

	state->dnxt.delivery = p->delivery;
	state->dnxt.frequency = p->frequency;

	if (p->delivery == DVBFE_DELSYS_DVBS2)
	{
		state->dnxt.rolloff = p->delsys.dvbs2.rolloff;
		state->dnxt.fec = p->delsys.dvbs2.fec;
		state->dnxt.modulation = p->delsys.dvbs2.modulation;
	}
	else
	{
		state->dnxt.rolloff = p->delsys.dvbs.rolloff;
		state->dnxt.fec = p->delsys.dvbs.fec;
		state->dnxt.modulation = p->delsys.dvbs.modulation;
	}
	if ((ret = cx24116_set_inversion(state, p->inversion)) != 0)
	{
		return ret;
	}
	if ((ret = cx24116_set_fec(state, p)) != 0)
	{
		return ret;
	}
	if ((ret = cx24116_set_symbolrate(state, p)) != 0)
	{
		return ret;
	}
	/* discard the 'current' tuning parameters and prepare to tune */
	memcpy(&state->dcur, &state->dnxt, sizeof(state->dcur));
	dprintk(20, "FREQ %i SR %i FECN %d DS %i INV = %d ROLL = %d INV_VAL = %d VAL/MASK %d/%d\n",
		state->dcur.frequency, state->dcur.symbol_rate, state->dcur.fec,
		state->dcur.delivery, state->dcur.inversion, state->dcur.rolloff,
		state->dcur.inversion_val, state->dcur.fec_val, state->dcur.fec_mask);

	/* Prepare a tune request */
	cmd.id = CMD_TUNEREQUEST;

	/* Frequency */
	cmd.args[0x01] = (state->dcur.frequency & 0xff0000) >> 16;
	cmd.args[0x02] = (state->dcur.frequency & 0x00ff00) >> 8;
	cmd.args[0x03] = (state->dcur.frequency & 0x0000ff);

	/* Symbol Rate */
	cmd.args[0x04] = ((state->dcur.symbol_rate / 1000) & 0xff00) >> 8;
	cmd.args[0x05] = ((state->dcur.symbol_rate / 1000) & 0x00ff);

	/* Automatic Inversion */
	cmd.args[0x06] = state->dcur.inversion_val;

	/* Modulation / FEC */
	cmd.args[0x07] = state->dcur.fec_val;

	cmd.args[0x08] = CX24116_SEARCH_RANGE_KHZ >> 8;
	cmd.args[0x09] = CX24116_SEARCH_RANGE_KHZ & 0xff;
	cmd.args[0x0a] = 0x00;
	cmd.args[0x0b] = 0x00;

	if (state->dcur.rolloff == DVBFE_ROLLOFF_25)
	{
		cmd.args[0x0c] = 0x01;
	}
	else if (state->dcur.rolloff == DVBFE_ROLLOFF_20)
	{
		cmd.args[0x0c] = 0x00;
	}
	else
	{
		cmd.args[0x0c] = 0x02;  // 0.35
	}
	cmd.args[0x0d] = state->dcur.fec_mask;

	above30msps = (state->dcur.symbol_rate > 30000000);

	if (above30msps)
	{
		/* 2010/01/07: should be revised with new fw202rc !!!!!
		 * currently I do not get channels with these
		 * symbol rates so I cannot test.
		 *
		 */
		cmd.args[0x0e] = 0x04;
		cmd.args[0x0f] = 0x00;
		cmd.args[0x10] = 0x01;
		cmd.args[0x11] = 0x77 /* 0xEC */ ;
		cmd.args[0x12] = 0x36 /* 0xFA */ ;

		/* Set Reset unknown */
		cx24116_writereg(state, 0xF9, 0x01);  /* DVB S1/2 Mode */
		cx24116_writereg(state, 0xF3, 0x44);  /* Clock Devider */
	}
	else
	{
		cmd.args[0x0e] = 0x06;
		cmd.args[0x0f] = 0x00;
		cmd.args[0x10] = 0x00;

		if (useUnknown == 0)
		{
			cmd.args[0x11] = 0xFA;  /* 0xEC */
			cmd.args[0x12] = 0x24;  /* 0xFA */
		}
		else
		{
			cmd.args[0x11] = 0xec;
			cmd.args[0x12] = 0xfa;
		}
		/* Set/Reset unknown */
		cx24116_writereg(state, 0xF9, 0x00);  /* DVB S1/2 Mode */
		cx24116_writereg(state, 0xF3, 0x46);  /* Clock Devider */
	}

	if ((state->dcur.delivery == DVBFE_DELSYS_DVBS2) || (state->dcur.fec != DVBFE_FEC_AUTO))  // fec_auto
	{
		retune = 2;
	}
	do
	{
		/* Reset status register */
		status = cx24116_readreg(state, CX24116_REG_STATUS) & 0xc0;
		cx24116_writereg(state, CX24116_REG_STATUS, status);

		/* Tune */
		ret = cx24116_cmd_execute(fe, &cmd);
		if (ret != 0)
		{
			return ret;
		}
		/* Wait for up to 500 ms */
		for (i = 0; i < 50; i++)
		{
			if (cx24116_is_tuned(fe))
			{
				dprintk(20, "Tuned, retune count = %d\n", retune);
				goto tuned;
			}
			msleep(10);
		}
		dprintk(20, "Retuned %d time(s)\n", retune);

		if (state->dcur.delivery == DVBFE_DELSYS_DVBS2)
		{
			/* Toggle pilot bit */
			cmd.args[0x07] ^= 0x40;
			dprintk(20, "Toggle pilot\n");
		}
		else
		{
			/* dvbs try fec_auto on second try */
			cmd.args[0x07] = 0x2e;
			cmd.args[0x0d] = 0xfe;
			state->dcur.fec = DVBFE_FEC_AUTO;
			state->dcur.fec_numb = 9;
		}
	}
	while (--retune);

tuned:  /* Set/Reset B/W */
	if (useUnknown == 1)
	{
		cmd.id = CMD_GETAGC;
		ret = cx24116_cmd_execute(fe, &cmd);
		cx24116_writereg(state, CX24116_REG_STATUS, 0x0f);
	}
	cmd.id = CMD_BANDWIDTH;
	cmd.args[0x01] = 0x00;
	cmd.len = 0x02;
	ret = cx24116_cmd_execute(fe, &cmd);
	if (ret != 0)
	{
		return ret;
	}
	if (useUnknown == 1)
	{
		status = cx24116_readreg(state, CX24116_REG_FECSTATUS);
		dprintk(20, "Tuned, fec=%02x new fec=%02x\n", state->dcur.fec_numb, status);
		if ((state->dcur.fec_numb != (status & CX24116_FEC_FECMASK)) && (state->dcur.delivery == DVBFE_DELSYS_DVBS))
		{
			state->dcur.fec_numb = status & CX24116_FEC_FECMASK;
			state->dcur.fec = cx24116_MODFEC_MODES[state->dcur.fec_numb].fec;
		}
		cmd.id = CMD_U1;
		ret = cx24116_cmd_execute(fe, &cmd);
		dprintk(70, "U1 data %02x ", cmd.args[0x07]);
		for (i = 0; i < 6; i++)
		{
			state->dcur.U1[i] = cx24116_readreg(state, i + 1);
			dprintk(1, " %02x", state->dcur.U1[i]);
		}
		ret = state->dcur.fec_numb * 2;

		if ((cmd.args[0x07] & 0x40) == 0x40)
		{
			ret++;
			pilot = 1;
		}
		else
		{
			pilot = 0;
		}
		dprintk(70, "cmd_u2 index = %d\n", ret);

		cmd.id = CMD_U2;
		cmd.args[1] = cx24116_U2_TABLE[ret].U2_1;
		cmd.args[2] = cx24116_U2_TABLE[ret].U2_2;
		cmd.args[3] = cx24116_U2_TABLE[ret].U2_3;
		cmd.args[4] = cx24116_U2_TABLE[ret].U2_4;
		cmd.args[5] = cx24116_U2_TABLE[ret].U2_5;
		cmd.args[6] = cx24116_U2_TABLE[ret].U2_6;
		cmd.args[7] = cx24116_U2_TABLE[ret].U2_7;
		cmd.args[8] = cx24116_U2_TABLE[ret].U2_8;
		cmd.args[9] = cx24116_U2_TABLE[ret].U2_9;

		ret = cx24116_cmd_execute(fe, &cmd);
		if (paramDebug > 49)
		{
			dprintk(1, "U2 data (pilot = %d): ", pilot);
			for (i = 0; i < 10 ; i++)
			{
				printk("0x%02x ", cmd.args[i]);
			}
			printk("\n");
		}
	}
	dprintk(100, "%s < ret = %d\n", __func__, ret);
	return ret;
}

static int cx24116_get_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	struct dvbfe_params feparams;
	int ret;

	dprintk(100, "%s >\n", __func__);

	ret = cx24116_get_params(fe, &feparams);
	if (ret != 0)
	{
		return ret;
	}
	return cx24116_create_old_qpsk_feparams(fe, &feparams, p);
}
#else

static int cx24116_lookup_fecmod(struct cx24116_state *state, fe_delivery_system_t d, fe_modulation_t m, fe_code_rate_t f)
{
	int i, ret = -EOPNOTSUPP;

	dprintk(100, "%s > (modulation 0x%02x, coderate 0x%02x)\n", __func__, m, f);

	for (i = 0; i < ARRAY_SIZE(cx24116_MODFEC_MODES); i++)
	{
		if ((d == cx24116_MODFEC_MODES[i].delsys)
		&&  (m == cx24116_MODFEC_MODES[i].modulation)
		&&  (f == cx24116_MODFEC_MODES[i].fec))
		{
			ret = i;
			break;
		}
	}
	return ret;
}

static int cx24116_set_fec(struct cx24116_state *state, fe_delivery_system_t delsys, fe_modulation_t mod, fe_code_rate_t fec)
{
	int ret = 0;

	dprintk(100, "%s > mod=0x%02x, fec=0x%02x)\n", __func__, mod, fec);

	ret = cx24116_lookup_fecmod(state, delsys, mod, fec);

	if (ret < 0)
	{
		dprintk(1, "%s < lookup FEC failed\n", __func__);
		return ret;
	}
	state->dnxt.fec = fec;
	state->dnxt.fec_val = cx24116_MODFEC_MODES[ret].val;
	state->dnxt.fec_mask = cx24116_MODFEC_MODES[ret].mask;
	dprintk(100, "%s < mask/val = 0x%02x/0x%02x\n", __func__, state->dnxt.fec_mask, state->dnxt.fec_val);
	state->dnxt.fec_numb = ret;
	return 0;
}

static int cx24116_set_symbolrate(struct cx24116_state *state, u32 rate)
{
	dprintk(100, "%s > symbolrate = %d\n", __func__, rate);

	/* check if symbol rate is within limits */
	if ((rate > state->frontend.ops.info.symbol_rate_max)
	||  (rate < state->frontend.ops.info.symbol_rate_min))
	{
		dprintk(1, "%s: unsupported symbol_rate = %d\n", __func__, rate);
		return -EOPNOTSUPP;
	}
	state->dnxt.symbol_rate = rate;
	dprintk(100, "%s <\n", __func__);
	return 0;
}
#endif

static int cx24116_set_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
#if  0 //DVB_API_VERSION < 5
	int ret = 0;
	struct dvbfe_params newfe;

	dprintk(100, "%s >\n", __func__);

	ret = cx24116_create_new_qpsk_feparams(fe, p, &newfe);

	if (ret != 0)
	{
		return ret;
	}
	return cx24116_set_params(fe, &newfe);
#else
	struct cx24116_state *state = fe->demodulator_priv;
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
	struct cx24116_cmd cmd;
	fe_status_t tunerstat;
	int i, status, ret, retune = 1;
	int above30msps;

	dprintk(100, "%s >\n", __func__);

	switch (c->delivery_system)
	{
		case SYS_DVBS:
		{
			dprintk(30, "DVB-S delivery system selected\n");
			/* Only QPSK is supported for DVB-S */
			if (c->modulation != QPSK)
			{
				dprintk(1, "%s < Unsupported modulation selected (%d)\n", __func__, c->modulation);
				return -EOPNOTSUPP;
			}
			/* Pilot doesn't exist in DVB-S, turn bit off */
			state->dnxt.pilot_val = CX24116_PILOT_OFF;

			/* DVB-S only supports 0.35 */
			if (c->rolloff != ROLLOFF_35)
			{
				dprintk(1, "%s < Unsupported rolloff selected (%d)\n", __func__, c->rolloff);
				return -EOPNOTSUPP;
			}
			state->dnxt.rolloff_val = CX24116_ROLLOFF_035;
			break;
		}
		case SYS_DVBS2:
		{
			dprintk(30, "DVB-S2 delivery system selected\n");
			/*
			 * NBC 8PSK/QPSK with DVB-S is supported for DVB-S2,
			 * but not hardware auto detection
			 */
			if (c->modulation != PSK_8 && c->modulation != QPSK)
			{
				dprintk(1, "%s < Unsupported modulation selected (%d)\n", __func__, c->modulation);
				return -EOPNOTSUPP;
			}

			switch (c->pilot)
			{
				case PILOT_AUTO:  /* Not supported but emulated */
				{
					dprintk(30, "Pilot auto\n");
					state->dnxt.pilot_val = (c->modulation == QPSK) ? CX24116_PILOT_OFF : CX24116_PILOT_ON;
					retune++;
					break;
				}
				case PILOT_OFF:
				{
					dprintk(30, "Pilot off\n");
					state->dnxt.pilot_val = CX24116_PILOT_OFF;
					break;
				}
				case PILOT_ON:
				{
					dprintk(30, "Pilot on\n");
					state->dnxt.pilot_val = CX24116_PILOT_ON;
					break;
				}
				default:
				{
					dprintk(1, "%s < Unsupported pilot mode selected (%d)\n", __func__, c->pilot);
					return -EOPNOTSUPP;
				}
			}

			switch (c->rolloff)
			{
				case ROLLOFF_20:
				{
					state->dnxt.rolloff_val = CX24116_ROLLOFF_020;
					break;
				}
				case ROLLOFF_25:
				{
					state->dnxt.rolloff_val = CX24116_ROLLOFF_025;
					break;
				}
				case ROLLOFF_35:
				{
					state->dnxt.rolloff_val = CX24116_ROLLOFF_035;
					break;
				}
				case ROLLOFF_AUTO:  /* Rolloff must be explicit */
				default:
				{
					dprintk(1, "%s < Unsupported rolloff selected (%d)\n", __func__, c->rolloff);
					return -EOPNOTSUPP;
				}
			}
			break;
		}
		default:
		{
			dprintk(1, "%s < Unsupported delivery system selected (%d)\n", __func__, c->delivery_system);
			return -EOPNOTSUPP;
		}
	}
	state->dnxt.delsys = c->delivery_system;
	state->dnxt.modulation = c->modulation;
	state->dnxt.frequency = c->frequency;
	state->dnxt.pilot = c->pilot;
	state->dnxt.rolloff = c->rolloff;

	ret = cx24116_set_inversion(state, c->inversion);
	if (ret !=  0)
	{
		return ret;
	}
	/* FEC_NONE/AUTO for DVB-S2 is not supported and detected here */
	ret = cx24116_set_fec(state, c->delivery_system, c->modulation, c->fec_inner);
	if (ret != 0)
	{
		return ret;
	}
	ret = cx24116_set_symbolrate(state, c->symbol_rate);
	if (ret != 0)
	{
		return ret;
	}
	/* discard the 'current' tuning parameters and prepare to tune */
	memcpy(&state->dcur, &state->dnxt, sizeof(state->dcur));

	dprintk(10, "%s: delsys      = %d\n", __func__, state->dcur.delsys);
	dprintk(10, "%s: modulation  = %d\n", __func__, state->dcur.modulation);
	dprintk(10, "%s: frequency   = %d kHz\n", __func__, state->dcur.frequency);
	dprintk(10, "%s: pilot       = %d (val = 0x%02x)\n", __func__, state->dcur.pilot, state->dcur.pilot_val);
	dprintk(10, "%s: retune      = %d\n", __func__, retune);
	dprintk(10, "%s: rolloff     = %d (val = 0x%02x)\n", __func__, state->dcur.rolloff, state->dcur.rolloff_val);
	dprintk(10, "%s: symbol_rate = %d MS/s\n", __func__, state->dcur.symbol_rate / 1000);
	dprintk(10, "%s: FEC         = %d (mask/val = 0x%02x/0x%02x)\n", __func__, state->dcur.fec, state->dcur.fec_mask, state->dcur.fec_val);
	dprintk(10, "%s: Inversion   = %d (val = 0x%02x)\n", __func__, state->dcur.inversion, state->dcur.inversion_val);

	/* Prepare a tune request */
	cmd.id = CMD_TUNEREQUEST;

	/* Frequency */
	cmd.args[0x01] = (state->dcur.frequency & 0xff0000) >> 16;
	cmd.args[0x02] = (state->dcur.frequency & 0x00ff00) >> 8;
	cmd.args[0x03] = (state->dcur.frequency & 0x0000ff);

	/* Symbol Rate */
	cmd.args[0x04] = ((state->dcur.symbol_rate / 1000) & 0xff00) >> 8;
	cmd.args[0x05] = ((state->dcur.symbol_rate / 1000) & 0x00ff);

	/* Automatic Inversion */
	cmd.args[0x06] = state->dcur.inversion_val;

	/* Modulation / FEC */
	cmd.args[0x07] = state->dcur.fec_val | state->dcur.pilot_val;
	cmd.args[0x08] = CX24116_SEARCH_RANGE_KHZ >> 8;
	cmd.args[0x09] = CX24116_SEARCH_RANGE_KHZ & 0xff;
	cmd.args[0x0a] = 0x00;
	cmd.args[0x0b] = 0x00;
	cmd.args[0x0c] = state->dcur.rolloff_val;
	cmd.args[0x0d] = state->dcur.fec_mask;

	above30msps = (state->dcur.symbol_rate > 30000000);

	if (above30msps)
	{
		/* 2010/01/07: should be revised with new fw202rc !!!!!
		 * currently I does not get channels with this
		 * symbol rates so I cant test.
		 *
		 */
		cmd.args[0x0e] = 0x04;
		cmd.args[0x0f] = 0x00;
		cmd.args[0x10] = 0x01;
		cmd.args[0x11] = 0x77;  /* 0xEC */
		cmd.args[0x12] = 0x36;  /* 0xFA */

		/* Set Reset unknown */
		cx24116_writereg(state, 0xF9, 0x01);  /* DVB S1/2 Mode */
		cx24116_writereg(state, 0xF3, 0x44);  /* Clock Devider */
	}
	else
	{
		cmd.args[0x0e] = 0x06;
		cmd.args[0x0f] = 0x00;
		cmd.args[0x10] = 0x00;

		if (useUnknown == 0)
		{
			cmd.args[0x11] = 0xFA;  /* 0xEC */
			cmd.args[0x12] = 0x24;  /* 0xFA */
		}
		else
		{
			cmd.args[0x11] = 0xec;
			cmd.args[0x12] = 0xfa;
		}

		/* Set/Reset unknown */
		cx24116_writereg(state, 0xF9, 0x00);  /* DVB-S1/2 Mode */
		cx24116_writereg(state, 0xF3, 0x46);  /* Clock Devider */
	}

	do
	{
		/* Reset status register */
		status = cx24116_readreg(state, CX24116_REG_STATUS) & 0xc0;
		cx24116_writereg(state, CX24116_REG_STATUS, status);

		/* Tune */
		ret = cx24116_cmd_execute(fe, &cmd);
		if (ret != 0)
		{
			return ret;
		}
		/* Wait for up to 500 ms */
		for (i = 0; i < 50; i++)
		{
			if (cx24116_is_tuned(fe))
			{
				dprintk(20, "Tuned; retune count = %d\n", retune);
				goto tuned;
			}
			msleep(10);
		}
		dprintk(30, "Retuned %d time(s)\n", retune);

		/* Toggle pilot bit when in auto-pilot */
		if ((state->dcur.pilot == PILOT_AUTO)
		||  (c->delivery_system == SYS_DVBS2))
		{
			cmd.args[0x07] ^= CX24116_PILOT_ON;
			dprintk(30, "Toggle pilot\n");
		}
		else if (c->delivery_system != SYS_DVBS2)
		{
			/* dvbs try fec_auto on second try */
			cmd.args[0x07] = 0x2e;
			cmd.args[0x0d] = 0xfe;
			state->dcur.fec = FEC_AUTO;
			state->dcur.fec_numb = 9;
		}
	}
	while (--retune);

tuned:  /* Set/Reset B/W */
	if (useUnknown == 1)
	{
		cmd.id = CMD_GETAGC;
		ret = cx24116_cmd_execute(fe, &cmd);
		cx24116_writereg(state, CX24116_REG_STATUS, 0x0f);
	}
	cmd.id = CMD_BANDWIDTH;
	cmd.args[0x01] = 0x00;
	cmd.len = 0x02;
	ret = cx24116_cmd_execute(fe, &cmd);
	if (ret != 0)
	{
		return ret;
	}
	if (useUnknown == 1)
	{
		status = cx24116_readreg(state, CX24116_REG_FECSTATUS);
		dprintk(20, "Tuned, fec=%02x new fec=%02x\n", state->dcur.fec_numb, status);

		if ((state->dcur.fec_numb != (status & CX24116_FEC_FECMASK)) && (c->delivery_system == SYS_DVBS))
		{
			state->dcur.fec_numb = status & CX24116_FEC_FECMASK;
			state->dcur.fec = cx24116_MODFEC_MODES[state->dcur.fec_numb].fec;
			dprintk(50, "fec=%02x fec_numb=%02x\n", state->dcur.fec, state->dcur.fec_numb);
		}
		cmd.id = CMD_U1;
		ret = cx24116_cmd_execute(fe, &cmd);
		if (paramDebug > 49)
		{
			dprintk(1, "U1 data %02x ", cmd.args[0x07]);
			for (i = 0; i < 6; i++)
			{
				state->dcur.U1[i] = cx24116_readreg(state, i + 1);
				printk(" %02x", state->dcur.U1[i]);
			}
			printk("\n");
		}
		ret = state->dcur.fec_numb * 2;

		if ((cmd.args[0x07] & 0x40) == 0x40)
		{
			ret++;
			pilot = 1;
		}
		else
		{
			pilot = 0;
		}
		dprintk(70, "cmd_u2 index = %d\n", ret);

		cmd.id = CMD_U2;
		cmd.args[1] = cx24116_U2_TABLE[ret].U2_1;
		cmd.args[2] = cx24116_U2_TABLE[ret].U2_2;
		cmd.args[3] = cx24116_U2_TABLE[ret].U2_3;
		cmd.args[4] = cx24116_U2_TABLE[ret].U2_4;
		cmd.args[5] = cx24116_U2_TABLE[ret].U2_5;
		cmd.args[6] = cx24116_U2_TABLE[ret].U2_6;
		cmd.args[7] = cx24116_U2_TABLE[ret].U2_7;
		cmd.args[8] = cx24116_U2_TABLE[ret].U2_8;
		cmd.args[9] = cx24116_U2_TABLE[ret].U2_9;

		ret = cx24116_cmd_execute(fe, &cmd);

		if (paramDebug >= 70)
		{
			dprintk(1, "U2 data (pilot = %d): ", pilot);
			for (i = 0; i < 10 ; i++)
			{
				printk("0x%02x ", cmd.args[i]);
			}
			printk("\n");
		}
	}
	dprintk(100, "%s < %d\n", __func__, ret);
	return ret;
#endif
}

#if DVB_API_VERSION >= 5
static int cx24116_tune(struct dvb_frontend *fe, struct dvb_frontend_parameters *params,
			unsigned int mode_flags, unsigned int *delay, fe_status_t *status)
{
	dprintk(100, "%s >\n", __func__);

	*delay = HZ / 5;
	if (params)
	{
		int ret = cx24116_set_frontend(fe, params);
		if (ret)
		{
			return ret;
		}
	}
	return cx24116_read_status(fe, status);
}

static int cx24116_get_property(struct dvb_frontend *fe, struct dtv_property *tvp)
{
	dprintk(100, "%s >\n", __func__);

	/* get delivery system info */
	if (tvp->cmd == DTV_DELIVERY_SYSTEM)
	{
		switch (tvp->u.data)
		{
			case SYS_DVBS2:
			case SYS_DVBS:
			case SYS_DSS:
			{
				break;
			}
			default:
			{
				dprintk(1, "%s < EINVAL\n", __func__);
				return -EINVAL;
			}
		}
	}
	dprintk(100, "%s <\n", __func__);
	return 0;
}
#endif

static int cx24116_fe_init(struct dvb_frontend *fe)
{
	struct cx24116_state *state = fe->demodulator_priv;
	struct cx24116_cmd cmd;

	// Powerup
	cx24116_writereg(state, 0xe0, 0);
	cx24116_writereg(state, 0xe1, 0);
	cx24116_writereg(state, 0xea, 0);

	cmd.id = CMD_TUNERSLEEP;
	cmd.args[1] = 0;
	cx24116_cmd_execute(fe, &cmd);

	return cx24116_diseqc_init(fe);
}

/*
 * LNBH23 register description:
 * I2C bus 0 and 1, addr 0x0a (or 0x0b, with ADDR pin = H)
 * seen values: 0xd4 (vertical) 0xdc (horizontal), 0xd0 shutdown
 *
 * Audioniek added:
 * 0xd4 -> 11010100 -> PCL=1, TTX=1, TEN=0, LLC=1, VSEL=0, EN=1 -> Vout=14.4V
 * 0xdc -> 11011100 -> PCL=1, TTX=1, TEN=0, LLC=1, VSEL=1, EN=1 -> Vout=19.5V
 * 0xd0 -> 11010000 -> PCL=1, TTX=1, TEN=0, LLC=1, VSEL=0, EN=0 -> Vout=0V
 * 0xd8 -> 11011000 -> PCL=1, TTX=1, TEN=0, LLC=1, VSEL=1, EN=0 -> Vout=0V
 * therefore: 0xd0 and 0xd8 are both valid for Vout=0
 *
 * From Documentation (can be downloaded on st.com):
 *
 * System register, write
 * MSB                           LSB
 * PCL TTX TEN LLC VSEL EN TEST4 TEST5 Function
 *      0       0    0   1  0     0    Vout=13.4V, Vup=14.5V
 *      0       0    1   1  0     0    Vout=18.4V, Vup=19.5V
 *      0       1    0   1  0     0    Vout=14.4V, Vup=15.5V
 *      0       1    1   1  0     0    Vout=19.5V, VUP=20.6V
 *      0   0            1  0     0    22kHz tone is disabled, EXTM disabled
 *      1   0            1  0     0    22kHz tone is controlled by DSQIN pin
 *      1   1            1  0     0    22kHz tone is ON, DSQIN pin disabled
 *      0                1  0     0    VoRX output is ON, VoTX tone generator is OFF
 *      1                1  0     0    VoRX output is ON, VoTX tone generator is ON
 *  0   X                1  0     0    Pulsed (dynamic) current limiting is selected
 *  1   X                1  0     0    Static current limiting is selected
 *  X   X   X   X    X   0  X     X    Power blocks disabled
 *
 * System register, read
 * MSB                           LSB (Note: S means read back as written)
 * TEST1 TEST2 TEST3 LLC VSEL EN OTF OLF Function
 *                    S   S    S  0   X  Tj < 135 degrees C (normal operation)
 *                    S   S    S  1   X  Tj > 150 degrees C (power blocks disabled)
 *                    S   S    S  X   0  Io < Iomax (normal operation)
 *                    S   S    S  X   1  Io > Iomax (Overload protection triggered)
 *   X    X     X                        These bits must be ignored
 *
 */
static int lnbh23_writereg_lnb_supply(struct cx24116_state *state, char data)
{
	int ret = -EREMOTEIO;
	struct i2c_msg msg;
	u8 buf;

	buf = data;

	msg.addr = state->config->i2c_addr_lnb_supply;
	msg.flags = 0;
	msg.buf = &buf;
	msg.len = 1;

	dprintk(50, "%s: write 0x%02x to I2C address 0x%02x\n", __func__, data & 0xff, msg.addr & 0xff);
	ret = i2c_transfer(state->config->i2c_adap, &msg, 1);

	if (ret != 1)
	{
#if 0  // try LNB221 (other I2C address, but same (potentially erroneous) value)	
		msg.addr = 0x08;  // get LNB221 I2C address
		msg.flags = 0;
		msg.buf = &buf;
		msg.len = 1;
		if ((ret = i2c_transfer(state->config->i2c_adap, &msg, 1)) != 1)
		{
			dprintk(1, "%s: writereg error(err == %i)\n", __func__, ret);
			ret = -EREMOTEIO;
		}
#else
		ret = -EREMOTEIO;
#endif
	}
	return ret;
}

int set_voltage(struct dvb_frontend *fe, fe_sec_voltage_t voltage)
{
	struct cx24116_state *state = fe->demodulator_priv;
	int ret = 0;
	int frontend_nr;

	dprintk(100, "%s >\n", __func__);

#if defined(UFS922)
	frontend_nr = (state->config->i2c_bus = 1 ? 0 : 1);
#else
	frontend_nr = 0;
#endif
	switch (voltage)
	{
		case SEC_VOLTAGE_OFF:
		{
			dprintk(20, "Frontend %d: switch LNB voltage off\n", frontend_nr);
			lnbh23_writereg_lnb_supply(state, state->config->disable);
			break;
		}
		case SEC_VOLTAGE_13:
		{
			dprintk(20, "Frontend %d: set LNB voltage to 13V (vertical)\n", frontend_nr);
			lnbh23_writereg_lnb_supply(state, state->config->vertical);
			break;
		}
		case SEC_VOLTAGE_18:
		{
			dprintk(20, "Frontend %d: set LNB voltage to 18V (horizontal)\n", frontend_nr);
			lnbh23_writereg_lnb_supply(state, state->config->horizontal);
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	return ret;
}

#if 0 //DVB_API_VERSION < 5
static int cx24116_get_info(struct dvb_frontend *fe, struct dvbfe_info *fe_info)
{
	dprintk(100, "%s >\n", __func__);

	switch (fe_info->delivery)
	{
		case DVBFE_DELSYS_DVBS:
		{
			dprintk(20, "%s: DVB-S\n", __func__);
			memcpy(fe_info, &dvbs_info, sizeof(dvbs_info));
			break;
		}
		case DVBFE_DELSYS_DVBS2:
		{
			dprintk(20, "%s: DVB-S2\n", __func__);
			memcpy(fe_info, &dvbs2_info, sizeof(dvbs2_info));
			break;
		}
		default:
		{
			dprintk(1, "%s: Invalid argument\n", __func__);
			return -EINVAL;
		}
	}
	return 0;
}

/* TODO: The hardware does DSS too, how does the kernel demux handle this? */
static int cx24116_get_delsys(struct dvb_frontend *fe, enum dvbfe_delsys *fe_delsys)
{
	dprintk(100, "%s >\n", __func__);
	*fe_delsys = DVBFE_DELSYS_DVBS | DVBFE_DELSYS_DVBS2;
	return 0;
}

/* TODO: is this necessary? */
static enum dvbfe_algo cx24116_get_algo(struct dvb_frontend *fe)
{
	dprintk(100, "%s >\n", __func__);
	return DVBFE_ALGO_SW;
}
#else
static int cx24116_get_algo(struct dvb_frontend *fe)
{
	dprintk(100, "%s >\n", __func__);
	return DVBFE_ALGO_HW;
}
#endif

static struct dvb_frontend_ops cx24116_ops;

struct dvb_frontend *cx24116_fe_qpsk_attach(struct cx24116_config *config)
{
	struct cx24116_state *state = NULL;
	int ret;
#if defined(UFS922)
	int frontend_nr = (config->i2c_bus == 1 ? 0 : 1);
#else
	int frontend_nr = 0;
#endif

	dprintk(100, "%s >\n", __func__);

	/* allocate memory for the internal state */
	state = kmalloc(sizeof(struct cx24116_state), GFP_KERNEL);
	if (state == NULL)
	{
		dprintk(1, "%s: kmalloc failed\n", __func__);
		return NULL;
	}
	dprintk(20, "Attaching frontend %d\n", frontend_nr);

	/* setup the state */
	memcpy((void *)&state->ops, &cx24116_ops, sizeof(struct dvb_frontend_ops));

	state->config = config;
	dprintk(50, "Probing frontend %d (I2C bus %d, I2C address 0x%02x)\n", frontend_nr, config-> i2c_bus, config->i2c_addr); 
	cx24116_reset(state);

	/* check if the demod is present */
	ret = (cx24116_readreg(state, 0xFF) << 8) | cx24116_readreg(state, 0xFE);
	if (ret != 0x0501)
	{
		dprintk(50, "ret = %04x\n", ret & 0xffff);
		dprintk(5, "Invalid probe, probably not a CX24116 device\n");
		kfree(state);
		return NULL;
	}
	state->not_responding = 0;

	/* create dvb_frontend */
	dprintk(20, "CX24116 frontend %d found\n", frontend_nr); 
	state->frontend.ops = state->ops;
	state->frontend.demodulator_priv = state;
	state->frontend.id = frontend_nr;

#if defined(TUNER_PROCFS)
	state->value[0] = 0x01;
	state->value[1] = 0x75;
	state->value[2] = 0x00;
	state->value[3] = 0x03;
	state->value[4] = 0x00;
#endif
	return &state->frontend;
}

#if defined(TUNER_PROCFS)
static int tuner_write_proc(struct file *file, const char __user *buf,  unsigned long count, void *data)
{
	char *page;
	ssize_t ret = -ENOMEM;
	struct cx24116_cmd cmd;
	struct dvb_frontend *fe = data;
	struct cx24116_state *state = fe->demodulator_priv;
	char s1[10], s2[10], s3[10], s4[10], s5[10];
	int result;

	page = (char *) __get_free_page(GFP_KERNEL);
	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
		{
			goto out;
		}
		result = sscanf(page, "%3s %3s %3s %3s %3s", s1, s2, s3, s4, s5);

		if (result != 5)
		{
			dprintk(1, %s: "result = %d\n", __func__, result);
			return -EINVAL;
		}
		sscanf(s1, "%hhd", &state->value[0]);
		sscanf(s2, "%hhd", &state->value[1]);
		sscanf(s3, "%hhd", &state->value[2]);
		sscanf(s4, "%hhd", &state->value[3]);
		sscanf(s5, "%hhd", &state->value[4]);

		printk("0x%x 0x%x 0x%x 0x%x 0x%x\n", state->value[0], state->value[1], state->value[2], state->value[3], state->value[4]);

		/* Firmware CMD 13: MPEG/TS output config */
		cmd.id = CMD_MPEGCONFIG;
		cmd.args[0x01] = state->value[0];
		cmd.args[0x02] = state->value[1];
		cmd.args[0x03] = state->value[2];
		cmd.args[0x04] = state->value[3];
		cmd.args[0x05] = state->value[4];

		ret = cx24116_cmd_execute(fe, &cmd);
		if (ret != 0)
		{
			ret = -EFAULT;
			goto out;
		}
		ret = count;
	}

out:
	free_page((unsigned long) page);
	return ret;
}

static int tuner_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	struct dvb_frontend *fe = data;
	struct cx24116_state *state = fe->demodulator_priv;

	dprintk(100, "%s > %d\n", __func__, count);

	len = sprintf(page, "%d %d %d %d %d\n", state->value[0], state->value[1], state->value[2], state->value[3], state->value[4]);
	*eof = 0;

	*start = page;
	return len;
}
#endif

static struct dvb_frontend *init_cx24116_device(struct dvb_adapter *adapter, struct plat_tuner_config *tuner_cfg)
{
	struct cx24116_state *state;
	struct dvb_frontend *frontend;
	struct cx24116_config *cfg;
#if defined(UFS922)
	int frontend_nr = (tuner_cfg->i2c_bus == 1 ? 0 : 1);
#else
	int frontend_nr = 0;
#endif

	dprintk(100, "%s > (I2C bus: %d)\n", __func__, tuner_cfg->i2c_bus);

	cfg = kmalloc(sizeof(struct cx24116_config), GFP_KERNEL);
	if (cfg == NULL)
	{
		dprintk(1, "%s: kmalloc failed\n", __func__);
		return NULL;
	}
	/* initialize the config data */
	cfg->i2c_adap = i2c_get_adapter(tuner_cfg->i2c_bus);
	cfg->i2c_bus = tuner_cfg->i2c_bus;
	cfg->i2c_addr = tuner_cfg->i2c_addr;
	dprintk(20, "Allocating PIO %d.%d as tuner enable\n", tuner_cfg->tuner_enable[0], tuner_cfg->tuner_enable[1]);
	cfg->tuner_enable_pin = stpio_request_pin(tuner_cfg->tuner_enable[0], tuner_cfg->tuner_enable[1], "tuner enabl", STPIO_OUT);
	if (cfg->tuner_enable_pin == NULL)
	{
		dprintk(1, "%s: Allocating tuner enable pin %d.%d failed for tuner %d\n", __func__, tuner_cfg->tuner_enable[0], tuner_cfg->tuner_enable[1], frontend_nr);
	}
	cfg->i2c_addr_lnb_supply = tuner_cfg->lnb_vsel[0];
	cfg->vertical = tuner_cfg->lnb_vsel[1];
	cfg->horizontal = tuner_cfg->lnb_vsel[2];
	cfg->disable = tuner_cfg->lnb_vsel[3];

	cfg->lnb_enable_pin = NULL;
	cfg->lnb_vsel_pin = NULL;

	if ((cfg->i2c_adap == NULL)
	||  (cfg->tuner_enable_pin == NULL)
	||  (cfg->i2c_addr_lnb_supply == 0x00))
	{
		dprintk(1, "%s: failed to allocate resources %x %p\n", __func__,cfg->i2c_addr_lnb_supply, cfg->i2c_adap);

		dprintk(20, "%s: I2C address LNB power controller 0x%02x\n", __func__, cfg->i2c_addr_lnb_supply);
		dprintk(20, "%s: I2C adapter = %p\n", __func__, cfg->i2c_adap);

		if (cfg->tuner_enable_pin != NULL)
		{
			stpio_free_pin(cfg->tuner_enable_pin);
		}
		if (cfg->lnb_enable_pin != NULL)
		{
			stpio_free_pin(cfg->lnb_enable_pin);
		}
		if (cfg->lnb_vsel_pin != NULL)
		{
			stpio_free_pin(cfg->lnb_vsel_pin);
		}
		kfree(cfg);
		return NULL;
	}
	// set pin polarities
	cfg->tuner_enable_act = tuner_cfg->tuner_enable[2];
	cfg->lnb_enable_act = tuner_cfg->lnb_enable[2];
	cfg->lnb_vsel_act = tuner_cfg->lnb_vsel[2];

	frontend = cx24116_fe_qpsk_attach(cfg);

	if (frontend == NULL)
	{
		if (cfg->tuner_enable_pin != NULL)
		{
			stpio_free_pin(cfg->tuner_enable_pin);
		}
		if (cfg->lnb_enable_pin != NULL)
		{
			stpio_free_pin(cfg->lnb_enable_pin);
		}
		if (cfg->lnb_vsel_pin != NULL)
		{
			stpio_free_pin(cfg->lnb_vsel_pin);
		}
		kfree(cfg);
		dprintk(1, "%s: cx24116_fe_qpsk_attach failed: not a cx24116?\n", __func__);
		return NULL;
	}
	dprintk(30, "%s: Call dvb_register_frontend (adapter = 0x%x)\n", __func__, (unsigned int)adapter);

	if (dvb_register_frontend(adapter, frontend))
	{
		dprintk(1, "%s: Frontend registration failed !\n", __func__);
		if (frontend->ops.release)
		{
			frontend->ops.release(frontend);
		}
		return NULL;
	}
	/* start fw loader thread */
	state = frontend->demodulator_priv;

	/* The semaphore should be initialized with 0 when using the
	   nowait firmware request.
	   When using the on-demand loading or the loader thread the
	   semaphore should be set to 1. */
	sema_init(&state->fw_load_sem, 1);

	/* TODO: enable early firmware download once /sys is mounted prior
	   to loading the driver, otherwise, the firmware request would fail */
#ifdef EARLY_FW_DOWNLOAD 
	ret = request_firmware_nowait(THIS_MODULE, 1, CX24116_DEFAULT_FIRMWARE, &state->config->i2c_adap->dev, state, cx24116_load_fw);
	if (ret)
	{
		dprintk(1, "%s: request_firmware_nowait failed\n", __func__);
	}
#endif

#if defined(TUNER_PROCFS)
	/* FIXME: how many procfs entries are necessary? */
	if ((state->proc_tuner = create_proc_entry("tuner", 0, proc_root_driver)))
	{
		state->proc_tuner->data = frontend;
		state->proc_tuner->read_proc = tuner_read_proc;
		state->proc_tuner->write_proc = tuner_write_proc;
	}
	else
	{
		dprintk(1, "%s: Failed to create procfs entry\n", __func__);
	}
#endif
	return frontend;
}

static void update_cx24116_devices(void)
{
	int i, j;

	dprintk(100, "%s >\n", __func__);

	/* loop over the configuration entries and try to
	   register new devices */
	for (i = 0; i < MAX_DVB_ADAPTERS; i++)
	{
		if (core[i] == NULL)
		{
			break;
		}
		for (j = 0; j < config_data[i].count; j++)
		{
			if (core[i]->frontend[j] == NULL)
			{
				/* found new entry for this adapter */
				core[i]->frontend[j] = init_cx24116_device(core[i]->dvb_adap, &config_data[i].tuner_cfg[j]);
			}
		}
	}
}

/*
* This function only retrieves the configuration data.
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static int cx24116_probe(struct platform_device *pdev)
#else
static int cx24116_probe(struct device *dev)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
	struct platform_device *pdev = to_platform_device(dev);
#endif
	struct plat_tuner_data *plat_data = pdev->dev.platform_data;
	int i;
	int j;
	int index;

	dprintk(100, "%s >\n", __func__);

	if (plat_data == NULL)
	{
		dprintk(1, "%s: No platform device data found\n");
		return -1;
	}
	dprintk(20, "Number of frontends = %d\n", plat_data->num_entries);
	/* loop over the list of provided devices and add the new ones
	   to the configuration array */
	for (i = 0; i < plat_data->num_entries; i++)
	{
		if ((plat_data->tuner_cfg[i].adapter >= 0)
		&&  (plat_data->tuner_cfg[i].adapter < MAX_DVB_ADAPTERS))
		{
			/* check whether the configuration already exists */
			index = plat_data->tuner_cfg[i].adapter;  // get adapter number

			for (j = 0; j < config_data[index].count; j++)
			{
				if ((plat_data->tuner_cfg[i].i2c_bus == config_data[index].tuner_cfg[j].i2c_bus)
				&&  (plat_data->tuner_cfg[i].i2c_addr == config_data[index].tuner_cfg[j].i2c_addr))
				{
					/* same bus, same address - discard */
					break;
				}
			}
			if ((j == config_data[index].count)
			&&  (j < (MAX_TUNERS_PER_ADAPTER - 1)))
			{
				/* a new config entry */
				dprintk(20, "%s: New device config i2c (I2C bus %d, I2C addr 0x%02x)\n", __func__, plat_data->tuner_cfg[i].i2c_bus, plat_data->tuner_cfg[i].i2c_addr);
				config_data[index].tuner_cfg[j] = plat_data->tuner_cfg[i];  // j?
				config_data[index].count++;
			}
		}
	}
	update_cx24116_devices();
	dprintk(100, "%s <\n", __func__);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static int cx24116_remove(struct platform_device *pdev)
#else
static int cx24116_remove(struct device *dev)
#endif
{
	/* TODO: add code to free resources */

	dprintk(1, "%s: not implemented yet\n", __func__);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static struct platform_driver cx24116_driver =
{
	.probe  = cx24116_probe,
	.remove = cx24116_remove,
	.driver	=
	{
		.name  = "cx24116",
		.owner = THIS_MODULE,
	},
};
#else
static struct device_driver cx24116_driver =
{
	.name   = "cx24116",
	.bus    = &platform_bus_type,
	.owner  = THIS_MODULE,
	.probe  = cx24116_probe,
	.remove = cx24116_remove,
};
#endif

/* FIXME: move the tuner configuration data either to the
   board/stb71xx/setup.c or to a module handling configuration */
struct plat_tuner_config tuner_resources[] =
{
	/* ufs922 tuner resources */
	[0] =  // marked LNB1 on rear panel
	{
		.adapter = 0,
		.i2c_bus = 0,
		.i2c_addr = 0x05,
		.tuner_enable = { 2, 4, 1 },
		.lnb_enable = { -1, -1, -1 },
		.lnb_vsel = { 0x0a, 0xc4, 0xcc, 0xc0 },
	},
	[1] =  // marked LNB2 on rear panel
	{
		.adapter = 0,
		.i2c_bus = 1,
		.i2c_addr = 0x05,
		.tuner_enable = { 2, 5, 1 },
		.lnb_enable = { -1, -1, -1 },
		.lnb_vsel = { 0x0a, 0xc4, 0xcc, 0xc0 },
	},
};

struct plat_tuner_data tuner_data =
{
	.num_entries = ARRAY_SIZE(tuner_resources),
	.tuner_cfg = tuner_resources
};

static struct platform_device tuner_device =
{
	.name = "cx24116",
	.dev  =
	{
		.platform_data = &tuner_data
	}
};

static struct platform_device *pvr_devices[] =
{
	&tuner_device,
};

void cx24116_register_frontend(struct dvb_adapter *dvb_adap)
{
	int i = 0;

	dprintk(20, "%s: > cx24116 DVB: 0.50 \n", __func__);

	/* TODO: add an array of cores to support multiple adapters */
	if (core[i])
	{
		dprintk(1, "%s: CX24116 core already registered\n", __func__);
		return;
	}
	core[i] = (struct cx24116_core *) kmalloc(sizeof(struct cx24116_core), GFP_KERNEL);
	if (!core[i])
	{
		dprintk(1, "%s: Out of memory\n", __func__);
		return;
	}
	memset(core[0], 0, sizeof(*core[i]));

	core[i]->dvb_adap = dvb_adap;

	dvb_adap->priv = core[i];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	if (platform_driver_register(&cx24116_driver) < 0)
#else
	if (driver_register(&cx24116_driver) < 0)
#endif
	{
		dprintk(1, "%s: Error registering device driver\n", __func__);
	}
	else
	{
		/* check whether there is anything to update */
		update_cx24116_devices();
	}
	/* FIXME: remove the resource registration once the
	   configuration is provided externally */
	dprintk(20, "Adding tuner configuration data\n");
	platform_add_devices(pvr_devices,  ARRAY_SIZE(pvr_devices));
	dprintk(100, "%s <\n", __func__);
	return;
}
EXPORT_SYMBOL(cx24116_register_frontend);

static struct dvb_frontend_ops cx24116_ops =
{
	.info =
	{
		.name                = "Conexant CX24116 DVB-S2",
		.type                = FE_QPSK,
		.frequency_min       =  950000,
		.frequency_max       = 2150000,
		.frequency_stepsize  = 1011,  /* kHz for QPSK frontends */
		.frequency_tolerance = 5000,
		.symbol_rate_min     =  1000000,
		.symbol_rate_max     = 45000000,
		.caps                = /* FE_CAN_INVERSION_AUTO | */// (ab)use inversion for pilot tones
		                       FE_CAN_FEC_1_2
		                     | FE_CAN_FEC_2_3
		                     | FE_CAN_FEC_3_4
		                     | FE_CAN_FEC_4_5
		                     | FE_CAN_FEC_5_6
		                     | FE_CAN_FEC_6_7
		                     | FE_CAN_FEC_7_8
		                     | FE_CAN_FEC_AUTO
		                     | FE_CAN_QPSK
		                     | FE_CAN_RECOVER
	},
	.release                 = cx24116_release,
	.init                    = cx24116_fe_init,
	.sleep                   = cx24116_sleep,
	.read_status             = cx24116_read_status,
	.read_ber                = cx24116_read_ber,
	.read_signal_strength    = cx24116_read_signal_strength,
	.read_snr                = cx24116_read_snr,
	.read_ucblocks           = cx24116_read_ucblocks,
	.set_tone                = cx24116_set_tone,
	.set_voltage             = set_voltage,
	.diseqc_send_master_cmd  = cx24116_send_diseqc_msg,
	.diseqc_send_burst       = cx24116_diseqc_send_burst,
	.get_frontend_algo       = cx24116_get_algo,
	.set_frontend            = cx24116_set_frontend,

#if 0 //DVB_API_VERSION < 5
	.get_tune_settings       = cx24116_get_tune_settings,
	.get_frontend            = cx24116_get_frontend,
	.get_info                = cx24116_get_info,
	.get_delsys              = cx24116_get_delsys,
	.set_params              = cx24116_set_params,
	.get_params              = cx24116_get_params,
#else
	.tune                    = cx24116_tune,
	.get_property            = cx24116_get_property,
#endif
};

/* ******************************* */
/* module functions                */
/* ******************************* */

int __init cx24116_init(void)
{
	dprintk(20, "Module cx24116 loaded\n");
	return 0;
}

void __exit cx24116_exit(void)
{
	dprintk(20, "Module cx24116 unloaded\n");
}

module_init(cx24116_init);
module_exit(cx24116_exit);

module_param(useUnknown, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(useUnknown, "Enable an unknown tuner command 0=disabled 1=enabled");

module_param(paramDebug, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(paramDebug, "Activates frontend debugging (default: 0)");

MODULE_DESCRIPTION("DVB Frontend module for Conexant CX24116/CX24118 hardware");
MODULE_AUTHOR("Open Vision developers");
MODULE_LICENSE("GPL");
// vim:ts=4
