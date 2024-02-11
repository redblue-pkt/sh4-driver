/************************************************************************
 *
 * Frontend core driver
 *
 * Customized for adb_box, bzzb model.
 *
 * Dual tuner frontend with STM STV0900, 2x STM STB6100 and ISL6422.
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
 ************************************************************************/
#ifndef __CORE_H__
#define __CORE_H__

extern short paramDebug;
#ifndef dprintk
#define dprintk(level, x...) do \
{ \
	if ((paramDebug) && (paramDebug >= level)) printk(TAGDEBUG x); \
} while (0)
#endif

#include "dvb_frontend.h"
#include "dvbdev.h"
#include "demux.h"
#include "dvb_demux.h"
#include "dmxdev.h"
#include "dvb_filter.h"
#include "dvb_net.h"
#include "dvb_ca_en50221.h"

#include <linux/dvb/frontend.h>
#include "dvb_frontend.h"

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#include <linux/stpio.h>
#else
#include <linux/stm/pio.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
#include "compat.h"
#include <linux/mutex.h>
#endif

#define MAX_DVB_ADAPTERS       4
#define MAX_TUNERS_PER_ADAPTER 4

struct core_config
{
	struct i2c_adapter *i2c_adap;            /* I2C bus of the tuners               */
	u8                 i2c_addr;             /* I2C address of the tuner(s)         */
//	u8                 i2c_addr_lnb_supply;  /* I2C address of the power controller */
//	u8                 vertical;             /* I2C value */
//	u8                 horizontal;           /* I2C value */
//	struct stpio_pin   *lnb_enable;
//	struct stpio_pin   *lnb_vsel;            /* 13/18V select pin */
//	struct stpio_pin   *tuner_reset_pin;
//	u8                 tuner_reset_act;      /* active state of the pin */
};

struct fe_core_state
{
	struct dvb_frontend_ops  ops;
	struct dvb_frontend      frontend;
	const struct core_config *config;
	int                      thread_id;
	int                      not_responding;
};

struct core_info
{
	char *name;
	int  type;
};

/* place to store all the necessary device information */
struct core
{
	/* devices */
	struct dvb_device     dvb_dev;
	struct dvb_net        dvb_net;

	struct core_info      *card;

	unsigned char         *grabbing;

	struct tasklet_struct fidb_tasklet;
	struct tasklet_struct vpe_tasklet;

	struct dmxdev         dmxdev;
	struct dvb_demux      demux;

	struct dmx_frontend   hw_frontend;
	struct dmx_frontend   mem_frontend;

	int                   ci_present;
	int                   video_port;

	u32                   buffer_width;
	u32                   buffer_height;
	u32                   buffer_size;
	u32                   buffer_warning_threshold;
	u32                   buffer_warnings;
	unsigned long         buffer_warning_time;

	u32                   ttbp;
	int                   feeding;

	spinlock_t            feedlock;

	spinlock_t            debilock;

	struct dvb_adapter    *dvb_adapter;
	struct dvb_frontend   *frontend[MAX_TUNERS_PER_ADAPTER];
	int                   (*read_fe_status)(struct dvb_frontend *fe, fe_status_t *status);
	int                   fe_synced;

	void                  *priv;
};

extern void fe_core_register_frontend(struct dvb_adapter *dvb_adap);

#endif  // __CORE_H__
// vim:ts=4
