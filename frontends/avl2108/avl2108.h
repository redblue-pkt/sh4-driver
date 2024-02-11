/*
 *  Availink avl2108 - DVB-S(2) Satellite demod driver
 *
 * Copyright (C) 2009-2010 Duolabs Spa
 *
 * Version for:
 * Kathrein UFS-922: Tuner STV6110A (Erit SP2237 frontend)
 * Octagon 1008: Tuner STV6110A (Sharp BS2S7HZ6360 frontend)
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _AVL2108_H
#define _AVL2108_H

#include <linux/dvb/frontend.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else /* STLinux 2.2 kernel */
#include <linux/stpio.h>
#endif

#include <linux/mutex.h>

#include "dvb_frontend.h"

#include "avl2108_platform.h"

#define cTUNER_INT_STV6306  1
#define cTUNER_EXT_STV6306  2
#define cTUNER_EXT_STV6110A 3

#define cLNB_LNBH221 1
#define cLNB_LNBH23  2
#define cLNB_PIO     3

/* Error codes */
#define AVL2108_OK            0  /*< No error */
#define AVL2108_ERROR_GENERIC 1  /*< Generic error */
#define AVL2108_ERROR_I2C     2  /*< i2c bus failed */
#define AVL2108_ERROR_TIMEOUT 4  /*< Operation failed in a given time period */
#define AVL2108_ERROR_PREV    8  /*< Still working on a previous command */
#define AVL2108_ERROR_MEM    32  /*< Not enough memory for finishing the current job */

/*****************************
 * Data type handling
 *****************************/

static inline u16 extract_16(const u8 *buf)
{
	u16 data;
	data = buf[0];
	data = (u16)(data << 8) + buf[1];
	return data;
}

static inline u32 extract_32(const u8 *buf)
{
	unsigned int data;
	data = buf[0];
	data = (data << 8) + buf[1];
	data = (data << 8) + buf[2];
	data = (data << 8) + buf[3];
	return data;
}

#if 0
struct Signal_Level
{
	u16 SignalLevel;
	short SignalDBM;
};

struct Signal_Level SignalLevel[47] =
{
	{ 8285,  -922 }, { 10224, -902 }, { 12538, -882 }, { 14890, -862 },
	{ 17343, -842 }, { 19767, -822 }, { 22178, -802 }, { 24618, -782 },
	{ 27006, -762 }, { 29106, -742 }, { 30853, -722 }, { 32289, -702 },
	{ 33577, -682 }, { 34625, -662 }, { 35632, -642 }, { 36552, -622 },
	{ 37467, -602 }, { 38520, -582 }, { 39643, -562 }, { 40972, -542 },
	{ 42351, -522 }, { 43659, -502 }, { 44812, -482 }, { 45811, -462 },
	{ 46703, -442 }, { 47501, -422 }, { 48331, -402 }, { 49116, -382 },
	{ 49894, -362 }, { 50684, -342 }, { 51543, -322 }, { 52442, -302 },
	{ 53407, -282 }, { 54314, -262 }, { 55208, -242 }, { 56000, -222 },
	{ 56789, -202 }, { 57544, -182 }, { 58253, -162 }, { 58959, -142 },
	{ 59657, -122 }, { 60404, -102 }, { 61181,  -82 }, { 62008,  -62 },
	{ 63032,  -42 }, { 65483,  -22 }, { 65535,  -12 }
};
#endif

struct avl2108_config
{
	int tuner_no;
	struct stpio_pin *tuner_enable_pin;

	u8 demod_address;  /*< the demodulator's i2c address */
	u8 tuner_address;  /*< the tuner's i2c address */

	u16 ref_freq;    /*< Reference clock in kHz units */
	u16 demod_freq;  /*< Demod clock in 10kHz units */
	u16 fec_freq;    /*< FEC clock in 10kHz units */
	u16 mpeg_freq;   /*< MPEG clock in 10kHz units */

	u16 i2c_speed_khz;
	u32 agc_polarization;
	u16 max_lpf;

	u32 mpeg_mode;
	u16 mpeg_serial;
	u16 mpeg_clk_mode;

	u32 pll_config;

	u32 tuner_active_lh;

	u32 lpf;
	u8  lock_mode;
	u8  iq_swap;
	u8  auto_iq_swap;

	u16 agc_ref;

	u32 usedTuner;
	u32 usedLNB;

	u32 lnb[6];
};

struct avl2108_equipment_s
{
	/* must be set by demod */
	u16(*demod_i2c_repeater_send)(void *state, u8 *buf, u16 size);
	u16(*demod_i2c_repeater_recv)(void *state, u8 *buf, u16 size);
	u16(*demod_i2c_write)(void *state, u8 *buf, u16 buf_size);
	u16(*demod_i2c_write16)(void *state, u32 addr, u16 data);
	u16(*demod_send_op)(u8 ucOpCmd, void *state);
	u16(*demod_get_op_status)(void *state);
	u16(*demod_i2c_read16)(void *state, u32 addr, u16 *data);

	/* must be set by tuner */
	u16(*tuner_load_fw)(struct dvb_frontend *fe);
	u16(*tuner_init)(struct dvb_frontend *fe);
	u16(*tuner_lock)(struct dvb_frontend *fe, u32 frequency, u32 srate, u32 _lfp);
	u16(*tuner_lock_status)(struct dvb_frontend *fe);

	/* must be set by lnb */
	u16(*lnb_set_voltage)(void *lnb_priv, struct dvb_frontend *fe, fe_sec_voltage_t voltage);
};

struct avl2108_state
{
	struct i2c_adapter         *i2c;
	struct avl2108_config      *config;

	struct dvb_frontend        frontend;
	u8                         boot_done;
	u8                         diseqc_status;

	void                       *lnb_priv;

	struct avl2108_equipment_s equipment;
	struct mutex lock;
};

struct dvb_frontend *avl2108_attach(struct avl2108_config *config, struct i2c_adapter *i2c);
int avl2108_set_tone(struct dvb_frontend *fe, fe_sec_tone_mode_t tone);
int avl2108_set_voltage(struct dvb_frontend *fe, fe_sec_voltage_t voltage);

extern int stv6306_attach(struct dvb_frontend *fe, void *demod_priv, struct avl2108_equipment_s *equipment, u8 internal, struct i2c_adapter *i2c);
extern int stv6110a_attach(struct dvb_frontend *fe, void *demod_priv, struct avl2108_equipment_s *equipment, u32 mclk, u32 max_lfp);

extern void *lnb_pio_attach(u32 *lnb, struct avl2108_equipment_s *equipment);
//extern void *lnbh221_attach(u32 *lnb, struct avl2108_equipment_s *equipment);
extern void *lnbh23_attach(u32 *lnb, struct avl2108_equipment_s *equipment);

#endif /* _AVL2108_H */
// vim:ts=4
