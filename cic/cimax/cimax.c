/*
 * Atmel CiMAX driver for ufs910, HL101 and similar
 * The driver also supports the StartCI2WIN chip (dual channel).
 *
 * Copyright (C) 2007 konfetti <konfetti@ufs910.de>
 * many thanx to jolt for good support
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
 *   TODO:
 *	- add dual channel support for StarCI2WIN
 *	- implement the platform driver concept
 *
 ****************************************************
 *
 * Note: used only by UFS910, VIP1_V1, VIP1_V2, Octagon1008, HL101
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/slab.h>

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#ifdef STLINUX20
#include <linux/platform.h>
#else
#include <linux/platform_device.h>
#endif
#include <linux/interrupt.h>
#include <linux/i2c.h> 
#include <linux/i2c-algo-bit.h>
#include <linux/firmware.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else
#include <linux/stpio.h>
#endif

#include <asm/system.h>
#include <asm/io.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif
#include <linux/dvb/dmx.h>

#include "dvb_frontend.h"
#include "dvbdev.h"
#include "demux.h"
#include "dvb_demux.h"
#include "dmxdev.h"
#include "dvb_filter.h"
#include "dvb_net.h"
#include "dvb_ca_en50221.h"

#include "cimax.h"

short paramDebug = 0;  // debug print level is zero as default (0=nothing, 1= errors, 10=some detail, 20=more detail, 100=open/close functions, 150=all)
#define TAGDEBUG "[CiMAX] "
#define dprintk(level, x...) \
	do \
	{ \
		if ((paramDebug) && (paramDebug >= level) || level == 0) \
		{ \
			printk(TAGDEBUG x); \
		} \
	} while (0)

#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2) \
 || defined(OCTAGON1008)
struct stpio_pin *cic_enable_pin;
struct stpio_pin *module_pin[2];
#endif

/* konfetti: EMI ************************* */
unsigned long reg_config = 0;
unsigned long reg_buffer = 0;
#if defined(OCTAGON1008)
static unsigned char *slot_membase[2];
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
static unsigned long reg_ci_base;
#else
unsigned long reg_bank3 = 0;
unsigned long reg_bank4 = 0;
#endif

#define EMI_DATA0_WE_USE_OE(a)      (a<<26)
#define EMI_DATA0_WAIT_POL(a)       (a<<25)
#define EMI_DATA0_LATCH_POINT(a)    (a<<20)
#define EMI_DATA0_DATA_DRIVE(a)     (a<<15)
#define EMI_DATA0_BUS_RELEASE(a)    (a<<11)
#define EMI_DATA0_CS_ACTIVE(a)      (a<<9)
#define EMI_DATA0_OE_ACTIVE(a)      (a<<7)
#define EMI_DATA0_BE_ACTIVE(a)      (a<<5)
#define EMI_DATA0_PORT_SIZE(a)      (a<<3)
#define EMI_DATA0_DEVICE_TYPE(a)    (a<<0)

#define EMI_DATA1_CYCLE(a)          (a<<31)
#define EMI_DATA1_ACCESS_READ(a)    (a<<24)
#define EMI_DATA1_CSE1_READ(a)      (a<<20)
#define EMI_DATA1_CSE2_READ(a)      (a<<16)
#define EMI_DATA1_OEE1_READ(a)      (a<<12)
#define EMI_DATA1_OEE2_READ(a)      (a<<8)
#define EMI_DATA1_BEE1_READ(a)      (a<<4)
#define EMI_DATA1_BEE2_READ(a)      (a<<0)

#define EMI_DATA2_CYCLE(a)          (a<<31)
#define EMI_DATA2_ACCESS_WRITE(a)   (a<<24)
#define EMI_DATA2_CSE1_WRITE(a)     (a<<20)
#define EMI_DATA2_CSE2_WRITE(a)     (a<<16)
#define EMI_DATA2_OEE1_WRITE(a)     (a<<12)
#define EMI_DATA2_OEE2_WRITE(a)     (a<<8)
#define EMI_DATA2_BEE1_WRITE(a)     (a<<4)
#define EMI_DATA2_BEE2_WRITE(a)     (a<<0)

#define EMIConfigBaseAddress        0x1A100000

#define EMIBufferBaseAddress        0x1A100800

#define EMIBank0                    0x100
#define EMIBank1                    0x140
#define EMIBank2                    0x180
#define EMIBank3                    0x1C0
#define EMIBank4                    0x200
#define EMIBank5                    0x240 /* virtual */
 
#define EMIBank0BaseAddress EMIConfigBaseAddress + EMIBank0
#define EMIBank1BaseAddress EMIConfigBaseAddress + EMIBank1
#define EMIBank2BaseAddress EMIConfigBaseAddress + EMIBank2
#define EMIBank3BaseAddress EMIConfigBaseAddress + EMIBank3
#define EMIBank4BaseAddress EMIConfigBaseAddress + EMIBank4
#define EMIBank5BaseAddress EMIConfigBaseAddress + EMIBank5

/* konfetti: first base address of EMI. the specification
 * says that all other can be calculated from the top address
 * but this doesnt work for me. so I set the address fix later on
 * and do not waste time on that. 
 */
#define EMI_BANK0_BASE_ADDRESS      0x40000000 

/* ConfigBase */
#define EMI_STA_CFG	                0x0010
#define EMI_STA_LCK 	            0x0018
#define EMI_LCK 	                0x0020
/* general purpose config register
 * 32Bit, R/W, reset=0x00
 * Bit 31-5 reserved
 * Bit 4 = PCCB4_EN: Enable PC card bank 4
 * Bit 3 = PCCB3_EN: Enable PC card bank 3 
 * Bit 2 = EWAIT_RETIME
 * Bit 1/0 reserved
 */
#define EMI_GEN_CFG                 0x0028

#define EMI_FLASH_CLK_SEL           0x0050 /* WO: 00, 10, 01 */
#define EMI_CLK_EN                  0x0068  /* WO: must only be set once !!*/

/* BankBase */
#define EMI_CFG_DATA0	            0x0000
#define EMI_CFG_DATA1	            0x0008
#define EMI_CFG_DATA2	            0x0010
#define EMI_CFG_DATA3            	0x0018


/* ***************************** */
/* EMIBufferBaseAddress + Offset */
/* ***************************** */
#define EMIB_BANK0_TOP_ADDR         0x000
#define EMIB_BANK1_TOP_ADDR         0x010
#define EMIB_BANK2_TOP_ADDR         0x020
 /* 32Bit, R/W, reset=0xFB
	* Bits 31- 8: reserved
	* Bits 7 - 0: Bits 27-22 off Bufferadresse
	*/
#define EMIB_BANK3_TOP_ADDR         0x030
 /* 32Bit, R/W, reset=0xFB
  * Bits 31- 8: reserved
  * Bits 7 - 0: Bits 27-22 off Bufferadresse
  */
#define EMIB_BANK4_TOP_ADDR         0x040
#define EMIB_BANK5_TOP_ADDR         0x050

/* 32 Bit, R/W, reset=0x110
 * Enable/Disable the banks
 */
#define EMIB_BANK_EN 	            0x0060

static struct cimax_core ci_core;
static struct cimax_state ci_state;

/* EMI Banks End ************************************ */

static int cimax_read_attribute_mem(struct dvb_ca_en50221 *ca, int slot, int address);

/* ************************** */
/* konfetti ->cimax control   */
/* ************************** */

#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2) \
 || defined(OCTAGON1008)
static int cimax_writeregN (struct cimax_state *state, u8 * data, u16 len)
{
	int ret = -EREMOTEIO;
	struct i2c_msg msg;
	u8 *buf;

	buf = kmalloc (len, GFP_KERNEL);

	if (buf == NULL)
	{
		dprintk (1, "Unable to kmalloc\n");
		ret = -ENOMEM;
		goto error;
	}
	memcpy (buf, data, len);

	msg.addr = state->i2c_addr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = len;

	if ((ret = i2c_transfer (state->i2c, &msg, 1)) != 1)
	{
		dprintk (1, "%s: Error(err == %i, reg == 0x%02x\n", __func__, ret, buf[0]);
		ret = -EREMOTEIO;
	}

error:
	kfree (buf);
	return ret;
}
#endif

static int cimax_writereg(struct cimax_state* state, int reg, int data)
{
	u8 buf[] = { reg, data };
	struct i2c_msg msg = { .addr = state->i2c_addr, .flags = 0, .buf = buf, .len = 2 };
	int err;

	dprintk(150, "%s: > reg 0x%02x, value 0x%02x\n", __func__,reg, data);

	if ((err = i2c_transfer(state->i2c, &msg, 1)) != 1)
	{
		dprintk(1, "%s: Error (err == %i, reg == 0x%02x, data == 0x%02x)\n", __func__, err, reg, data);
		return -EREMOTEIO;
	}
	return 0;
}

static int cimax_readreg(struct cimax_state* state, u8 reg)
{
	int ret;
	u8 b0[] = { reg };
	u8 b1[] = { 0 };
	struct i2c_msg msg[] =
	{
		{ .addr = state->i2c_addr, .flags = 0, .buf = b0, .len = 1 },
		{ .addr = state->i2c_addr, .flags = I2C_M_RD, .buf = b1, .len = 1 }
	};

	ret = i2c_transfer(state->i2c, msg, 2);

	if (ret != 2)
	{
		dprintk(1, "%s: reg=0x%x (error=%d)\n", __func__, reg, ret);
		return ret;
	}
	dprintk(150, "%s < reg 0x%02x, value 0x%02x\n",__func__, reg, b1[0]);
	return b1[0];
}

/* This function gets the CI source
   Params:
     slot - CI slot number (0|1)
     source - tuner number (0|1)
*/
void getCiSource(int slot, int* source)
{
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
	int val;

	val = cimax_readreg(&ci_state, 0x11);
	val &= 0x20;

	if(slot == 0)
	{
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		if (val != 0)
		{
			*source = 1;
		}
		else
		{
			*source = 0;
		}
#else
		if (val != 0) 
		{
			*source = 0;
		}
		else
		{
			*source = 1;
		}
#endif
	}

	if(slot == 1)
	{
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		if (val != 0)
		{
			*source = 0;
		}
		else
		{
			*source = 1;
		}
#else
		if (val != 0)
		{
			*source = 1;
		}
		else
		{
			*source = 0;
		}
#endif
	}
#endif
}

/* This function sets the CI source
   Params:
     slot - CI slot number (0|1)
     source - tuner number (0|1)
*/
int setCiSource(int slot, int source)
{
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
	int val;

	if ((slot < 0)
	||  (slot > 1)
	||  (source < 0)
	||  (source > 1))
	{
		return -1;
	}
	val = cimax_readreg(&ci_state, 0x11);
	if(slot != source)
	{
		/* send stream A through module B and stream B through module A */
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		val |= 0x20;
#else
		val &= ~0x20;
#endif
	}
	else
	{
		/* enforce direct mapping */
		/* send stream A through module A and stream B through module B */
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		val &= ~0x20;
#else
		val |= 0x20;
#endif
	}
	return cimax_writereg(&ci_state, 0x11, val);
#else
	return 0;
#endif
}

void setDestination(struct cimax_state *state, int slot)
{
	int result = 0;
	int loop = 0;

	dprintk(150, "%s > slot = %d\n", __func__, slot);

#if !defined(OCTAGON1008)
	if (slot == 0)
	{
		/* read destination register */
		result = cimax_readreg(state, 0x17);

		while ((result & 0x2) == 0)
		{
			/* disable module b if this was selected before */
			result = result & ~0x4; 
			cimax_writereg(state, 0x17, result | 0x2);

			/* try it first time without sleep */
			if (loop > 0)
			{
				msleep(10);
			}
			/* destination: module a */
			result = cimax_readreg(state, 0x17);
			
			dprintk(20, "%s slot = %d, loop = %d: result = 0x%02x\n", __func__, slot,loop, result);
		
			loop++;
			if (loop == 10)
			{
				dprintk(1, "%s Abort setting slot destination\n", __func__);
				break;
			}
		} 
	}
	else
	{
		/* read destination register */
		result = cimax_readreg(state, 0x17);

		while ((result & 0x4) == 0)
		{
			/* disable module a if this was selected before */
			result = result & ~0x2; 
			cimax_writereg(state, 0x17, result | 0x4);
		
			/* try it first time without sleep */
			if (loop > 0) msleep(10);
		
			/* destination: modul b */
			result = cimax_readreg(state, 0x17);

			dprintk(20, "%s slot = %d, loop = %d: result = 0x%02x\n", __func__, slot, loop, result);
			loop++;
			if (loop == 10)
			{
				dprintk(1, "%s Abort setting slot destination\n", __func__);
				break;
			}
		}
	}
#endif
	dprintk(150, "%s < slot = %d\n", __func__, slot);
}

static int cimax_poll_slot_status(struct dvb_ca_en50221 *ca, int slot, int open)
{
	struct cimax_state *state = ca->data;
	int slot_status = 0;
	int ctrlReg[2] = {0x00, 0x09};

	dprintk(150, "%s > slot = %d, open = %d) >\n", __func__, slot, open);

	if ((slot < 0) || (slot > 1))
	{
		return 0;
	}
	slot_status = cimax_readreg(state, ctrlReg[slot]) & 0x01;
	if (slot_status)
	{
		if (state->module_status[slot] & SLOTSTATUS_RESET)
		{
			unsigned int result = cimax_read_attribute_mem(ca, slot, 0); 

			dprintk(20, "%s result = 0x%02x\n", __func__, result);
			if (result == 0x1d)
			{
				state->module_status[slot] = SLOTSTATUS_READY;
			}
		}
		else if (state->module_status[slot] & SLOTSTATUS_NONE)
		{
#if defined(OCTAGON1008)
			stpio_set_pin(module_pin[slot], 1);
#endif
			dprintk(20, "CI Module present now\n");
			state->module_status[slot] = SLOTSTATUS_PRESENT;
		}
	}
	else
	{
		if (!(state->module_status[slot] & SLOTSTATUS_NONE))
		{
#if defined(OCTAGON1008)
			stpio_set_pin (module_pin[slot], 0);
#endif
			dprintk(20, "CI Module not present now\n");
			state->module_status[slot] = SLOTSTATUS_NONE;
		}
	}
	if (state->module_status[slot] != SLOTSTATUS_NONE)
	{
		slot_status = DVB_CA_EN50221_POLL_CAM_PRESENT;
	}
	else
	{
		slot_status = 0;
	}
	if (state->module_status[slot] & SLOTSTATUS_READY)
	{
		slot_status |= DVB_CA_EN50221_POLL_CAM_READY;
	}
	dprintk(150, "%s < CI Module %c, slot:%d, result = %d, status = %d\n", slot ? 'B' : 'A', slot, slot_status, state->module_status[slot]);
	return slot_status;
}

static int cimax_slot_reset(struct dvb_ca_en50221 *ca, int slot)
{
	struct cimax_state *state = ca->data;

	dprintk(150, "%s >\n", __func__);

	state->module_status[slot] = SLOTSTATUS_RESET;

	if (slot == 0)
	{
		int result = cimax_readreg(state, 0x00);

		/* only reset if module is present */
		if (result & 0x01)
		{
			result = cimax_readreg(state, 0x00);
			dprintk(20, "%s: result = 0x%02x\n", __func__, result);
			cimax_writereg(state, 0x00, result | 0x80);

#if !defined(OCTAGON1008)
			cimax_writereg(state, 0x17, 0x0);
#endif
			msleep(60);
			
			/* reset "rst" bit */
			result = cimax_readreg(state, 0x00);
			cimax_writereg(state, 0x00, result & ~0x80);

			dprintk(1, "%s Reset Module A\n", __func__);
		}
	}
	else
	{
		int result = cimax_readreg(state, 0x09);

		/* only reset if module is present */
		if (result & 0x01)
		{
			result = cimax_readreg(state, 0x09);
			dprintk(20, "%s: result = 0x%02x\n", __func__, result);
			cimax_writereg(state, 0x09, result | 0x80);
			
#if !defined(OCTAGON1008)
			cimax_writereg(state, 0x17, 0x0);
#endif
			msleep(60);

			/* reset "rst" bit */
			result = cimax_readreg(state, 0x09);
			cimax_writereg(state, 0x09, result & ~0x80);

			dprintk(1, "%s Reset Module B\n", __func__);
		}
	}
	dprintk(150, "%s <\n", __func__);
	return 0;
}

static int cimax_read_attribute_mem(struct dvb_ca_en50221 *ca, int slot, int address)
{
	struct cimax_state *state = ca->data;
	int res = 0;
	int result;

	dprintk(150, "%s > slot = %d, address = %d\n", __func__, slot, address);

	if (slot == 0)
	{
		result = cimax_readreg(state, 0x00);
		if (result & 0xC) 
		{
			cimax_writereg(state, 0x00, (result & ~0xC));
		}
		setDestination(state, slot);
#if defined(OCTAGON1008)
		res = slot_membase[slot][address];
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		res = ctrl_inb(reg_ci_base + (address));
#else
		res = ctrl_inb(reg_bank4 + (address << 1));
#endif
		res &= 0x00FF;
	}
	else
	{
		result = cimax_readreg(state, 0x09);

		/* delete bit 3/4 ->access to attribute mem */
		if (result & 0xC) 
		{
			cimax_writereg(state, 0x09, (result & ~0xC));
		}
		setDestination(state, slot);
#if defined(OCTAGON1008)
		res = slot_membase[slot][address];
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		res = ctrl_inb(reg_ci_base + (address));
#else
		res = ctrl_inb(reg_bank4 + (address << 1));
#endif
		res &= 0x00FF;
	}
	if (address <= 2)
	{
		dprintk (20, "%s: Address = %d: res = 0x%02x\n", __func__, address, res);
	}
	else
	{
		if ((res > 0x1f) && (res < 0x7f))
		{
			printk("%c", res);
		}
		else
		{
			printk(".");
		}
	}
	return res;
}

static int cimax_write_attribute_mem(struct dvb_ca_en50221 *ca, int slot, int address, u8 value)
{
	struct cimax_state *state = ca->data;
	int result;
	
	dprintk(150, "%s > slot = %d, address = %d, value = %d\n", __func__, slot, address, value);

	if (slot == 0)
	{
		result = cimax_readreg(state, 0x00);
		/* delete bit 3/4 ->access to attribute mem */
		if (result & 0xC) 
		{
			cimax_writereg(state, 0x00, (result & ~0xC));
		}
		setDestination(state, slot);
#if defined(OCTAGON1008)
		slot_membase[slot][address] = value;
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		ctrl_outb(value, reg_ci_base + (address));
#else
		ctrl_outb(value, reg_bank4 + (address << 1));
#endif
	}
	else
	{
		result = cimax_readreg(state, 0x09);
		/* bit 3/4 loeschen ->access to attribute mem */
		if (result & 0xC) 
		{
			cimax_writereg(state, 0x09, (result & ~0xC));
		}
		setDestination(state, slot);
#if defined(OCTAGON1008)
		slot_membase[slot][address] = value;
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		ctrl_outb(value, reg_ci_base + (address));
#else
		ctrl_outb(value, reg_bank4 + (address << 1));
#endif
	}
	return 0;
}

static int cimax_read_cam_control(struct dvb_ca_en50221 *ca, int slot, u8 address)
{
	struct cimax_state *state = ca->data;
	int res = 0;
	int result;
	
	dprintk(150, "%s > slot = %d, address = %d\n", __func__, slot, address);

	if (slot == 0)
	{
		result = cimax_readreg(state, 0x00);
		/* FIXME: handle "result" ->is the module really present */

		/* access i/o mem (bit 3)*/
		if (!(result & 0x4))
		{
			cimax_writereg(state, 0x00, (result & ~0xC) | 0x4);
		}
		setDestination(state, slot);

#if defined(OCTAGON1008)
		res = slot_membase[slot][address];
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		res = ctrl_inb(reg_ci_base + (address /*<< 1*/));
#else
		res = ctrl_inb(reg_bank4 + (address << 1));
#endif
		res &= 0x00FF;
	}
	else
	{
		result = cimax_readreg(state, 0x09);
		/* FIXME: handle "result" ->is the module really present */
		/* access i/o mem (bit 3) */
		if (!(result & 0x4))
		{
			cimax_writereg(state, 0x09, (result & ~0xC) | 0x4);
		}
		setDestination(state, slot);
#if defined(OCTAGON1008)
		res = slot_membase[slot][address];
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		res = ctrl_inb(reg_ci_base + (address /*<< 1*/));
#else
		res = ctrl_inb(reg_bank4 + (address << 1));
#endif
		res &= 0x00FF;
	}
	if (address <= 2)
	{
#if defined(OCTAGON1008)
	// do nothing
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		dprintk (50, "%s Address = 0x%.8lx: res = 0x%02x\n", __func__, reg_ci_base + (address /*<< 1*/), res);
#else
		dprintk (50, "%s Address = 0x%.8lx: res = 0x%02x\n", __func__, reg_bank4 + (address << 1), res);
#endif
	}
	else 
	{
		if ((res > 31) && (res < 127))
		{
			printk("%c", res);
		}
		else
		{
			printk(".");
		}
	}
	return res;
}

static int cimax_write_cam_control(struct dvb_ca_en50221 *ca, int slot, u8 address, u8 value)
{
	struct cimax_state *state = ca->data;
	int result;
	
	dprintk(150, "%s > slot = %d, address = %d, value = %d\n", __func__, slot, address, value);

	if (address <= 2)
	{
#if defined(OCTAGON1008)
		// do nothing
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		dprintk (50, "%s Address = 0x%.8lx: value = 0x%02x\n", __func__, reg_ci_base + (address ), value);
#else
		dprintk (50, "Address = 0x%.8lx: value = 0x%02x\n", __func__, reg_bank4 + (address << 1), value);
#endif
	}
	if (slot == 0)
	{
		result = cimax_readreg(state, 0x00);

		/* FIXME: handle "result" ->is the module really present */
		/* access to i/o mem */
		if (!(result & 0x4))
		{
			cimax_writereg(state, 0x00, (result & ~0xC) | 0x4);
		}
		setDestination(state, slot);
#if defined(OCTAGON1008)
		slot_membase[slot][address] = value;
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		ctrl_outb(value, reg_ci_base + (address ));
#else
		ctrl_outb(value, reg_bank4 + (address << 1));
#endif
	}
	else
	{
		result = cimax_readreg(state, 0x09);

		/* FIXME: handle "result" ->is the module really present */
		/* access to i/o mem */
		if (!(result & 0x4))
		{
			cimax_writereg(state, 0x09, (result & ~0xC) | 0x4);
		}
		setDestination(state, slot);
#if defined(OCTAGON1008)
		slot_membase[slot][address] = value;
#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
		ctrl_outb(value, reg_ci_base + (address ));
#else
		ctrl_outb(value, reg_bank4 + (address << 1));
#endif
	}
	return 0;
}

static int cimax_slot_shutdown(struct dvb_ca_en50221 *ca, int slot)
{
	//struct cimax_state *state = ca->data;
	dprintk(150, "%s > slot = %d\n", __func__, slot);

	/*Power control : (@18h); quatsch slot shutdown ->0x17*/
	return 0;
}

static int cimax_slot_ts_enable(struct dvb_ca_en50221 *ca, int slot)
{
	struct cimax_state *state = ca->data;

	dprintk(150, "%s > slot = %d\n", __func__, slot);

	if (slot == 0)
	{
		/* das read back der register ist gleichzeitig unser sleep! */	
		int result = cimax_readreg(state, 0x00);

#if !defined(OCTAGON1008)
		cimax_writereg(state, 0x00, 0x23);
		dprintk(70, "%s: writing 0x%x\n", __func__, 0x23);
//#elif !defined(OCTAGON1008)
		cimax_writereg(state, 0x00, result | 0x20);  // fs9000
#else
		cimax_writereg(state, 0x00, result | 0x21);  // octagon
#endif
		result = cimax_readreg(state, 0x00);
		dprintk(70, "%s: writing 0x%02x\n", __func__, result | 0x40);
		cimax_writereg(state, 0x00, result | 0x40);
		result = cimax_readreg(state, 0x00);

		dprintk(20, "%s: result 0x%02x\n", __func__, result);

		if (!(result & 0x40))
		{
			dprintk(1, "%s Error setting ts enable on slot 0\n", __func__);
		}
	} 
	else
	{
		int result = cimax_readreg(state, 0x09);
#if !defined(OCTAGON1008)
		cimax_writereg(state, 0x09, 0x23);
		dprintk(70, "%s: writing 0x%02x\n", __func__, 0x23);
//#elif !defined(OCTAGON1008)
		cimax_writereg(state, 0x09, result | 0x20);
#else
		cimax_writereg(state, 0x09, result | 0x21);  // octagon
#endif
		result = cimax_readreg(state, 0x09);

		dprintk(70, "%s: writing 0x%02x\n", __func__, result | 0x40);
		cimax_writereg(state, 0x09, result | 0x40);
		result = cimax_readreg(state, 0x09);

		dprintk(70, "%s: result 0x%02x\n", __func__, result);

		if (!(result & 0x40))
		{
			dprintk(1, "%s Error setting ts enable on slot 1\n", __func__);
		}
	}
	return 0;
}

int init_ci_controller(struct dvb_adapter* dvb_adap)
{
	struct cimax_state *state = &ci_state;
	struct cimax_core *core = &ci_core;
	int result;
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
	u8 sequence[33] =
	{
		0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x44,
		0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x06, 0x00, 0x03,
		0x01
	};

#elif defined(OCTAGON1008)
	u8 sequence[33] =
	{
		0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x44,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x06, 0x00, 0x03,
		0x01 /* ->0x1f = 0x01 = lock ->so after this most registers are not accessible */
	};
#endif

	dprintk(10, "Initialize CiMAX driver\n");

	core->dvb_adap = dvb_adap;
#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
	state->i2c = i2c_get_adapter(2);
#elif defined(OCTAGON1008)
	state->i2c = i2c_get_adapter(1);
#else
	state->i2c = i2c_get_adapter(0);
#endif

#if defined(OCTAGON1008)
	state->i2c_addr = 0x43;
#else
	state->i2c_addr = 0x40;
#endif

	memset(&core->ca, 0, sizeof(struct dvb_ca_en50221));

	/* register CI interface */
	core->ca.owner = THIS_MODULE;

	core->ca.read_attribute_mem  = cimax_read_attribute_mem;
	core->ca.write_attribute_mem = cimax_write_attribute_mem;
	core->ca.read_cam_control 	 = cimax_read_cam_control;
	core->ca.write_cam_control   = cimax_write_cam_control;
	core->ca.slot_shutdown       = cimax_slot_shutdown;
	core->ca.slot_ts_enable      = cimax_slot_ts_enable;

	core->ca.slot_reset          = cimax_slot_reset;
	core->ca.poll_slot_status    = cimax_poll_slot_status;

	state->core                  = core;
	core->ca.data                = state;

	state->module_status[0] = SLOTSTATUS_NONE;
	state->module_status[1] = SLOTSTATUS_NONE;

	reg_config = (unsigned long) ioremap(EMIConfigBaseAddress, 0x7ff);
	reg_buffer = (unsigned long) ioremap(EMIBufferBaseAddress, 0x40);

	dprintk(20, "ioremap 0x%.8x -> 0x%.8lx\n", EMIConfigBaseAddress, reg_config);	
	dprintk(20, "ioremap 0x%.8x -> 0x%.8lx\n", EMIBufferBaseAddress, reg_buffer);
	dprintk(10, "Allocating PIO pins\n");	

#if defined(OCTAGON1008)
	cic_enable_pin = stpio_request_pin (3, 6, "StarCI", STPIO_OUT);
	stpio_set_pin (cic_enable_pin, 1);
	msleep(250);
	stpio_set_pin (cic_enable_pin, 0);
	msleep(250);

	module_pin[0] = stpio_request_pin (1, 2, "StarCI_ModuleA", STPIO_OUT);
	module_pin[1] = stpio_request_pin (2, 7, "StarCI_ModuleB", STPIO_OUT);

	stpio_set_pin (module_pin[0], 0);
	stpio_set_pin (module_pin[1], 0);

#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
	cic_enable_pin = stpio_request_pin (2, 5, "CIMaX", STPIO_OUT);
	if (cic_enable_pin == NULL)
	{
		dprintk(1, "ERROR: Allocating PIO pin 2.5 failed\n");	
	}
	stpio_set_pin (cic_enable_pin, 1);
	msleep(250);
	stpio_set_pin (cic_enable_pin, 0);
	msleep(250);

	module_pin[0] = stpio_request_pin (0, 2, "CIMaX_ModuleA", STPIO_OUT);
	if (module_pin[0] == NULL)
	{
		dprintk(1, "ERROR: Allocating PIO pin 0.2 failed\n");	
	}
	module_pin[1] = stpio_request_pin (0, 6, "CIMaX_ModuleB", STPIO_OUT);
	if (module_pin[1] == NULL)
	{
		dprintk(1, "ERROR: Allocating PIO pin 0.6 failed\n");	
	}
	stpio_set_pin (module_pin[0], 1);
	stpio_set_pin (module_pin[1], 1);
#else
	/* add your box hw infos here... */
#endif
	/* cimax reset */
	cimax_writereg(state, 0x1f, 0x80);

#if defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2) \
 || defined(OCTAGON1008)
	cimax_writeregN(state, sequence, sizeof(sequence));
#endif
 
#if !defined(OCTAGON1008)
	/* Module A auto activation */	
	cimax_writereg(state, 0x00, 0x02);

	/* Module B auto activation */	
	cimax_writereg(state, 0x09, 0x02);
#else
	cimax_writereg(state, 0x00, 0x00);
	cimax_writereg(state, 0x09, 0x00);
#endif

#if defined(OCTAGON1008)
	/* Powercontrol: VCLVL ->VCC pin active-low (LOCK muss 0 sein) */
	cimax_writereg(state, 0x18, 0x21);
#else
	/* Powercontrol: VCLVL ->VCC pin active-high (LOCK muss 0 sein) */
	cimax_writereg(state, 0x18, 0x40);	
#endif

#if !defined(OCTAGON1008)
	/* Destination Select: Select module A */
	cimax_writereg(state, 0x17, 0x02);

	/* memory access cycle for Module A: set all to 100ns */
	cimax_writereg(state, 0x05, 0x00);

	/* memory access cycle for Module B: set all to 100ns */
	cimax_writereg(state, 0x0E, 0x00);

	/* interrupt output register: ITDRV: INT buffer is push-pull (LOCK 0) */
	cimax_writereg(state, 0x1c, 0x04);

	/* interrupt mask register: set all to mask */
	cimax_writereg(state, 0x1b, 0x00);

	/* UCSG2 Register: */
	cimax_writereg(state, 0x1e, 0x00);

	/* UCSG1 Register: */
	cimax_writereg(state, 0x1d, 0x00);

	/* Auto select mask high external  */
	cimax_writereg(state, 0x12, 0x00);

	/* Auto select mask high module a */
	cimax_writereg(state, 0x01, 0x00);

	/* Auto select mask low module a */
	cimax_writereg(state, 0x02, 0x1c);

	/* Auto select pattern high module a */
	cimax_writereg(state, 0x03, 0x00);

	/* Auto select pattern low module a */
	cimax_writereg(state, 0x04, 0x04);

	/* Auto select mask high module b */
	cimax_writereg(state, 0x0a, 0x00);

	/* Auto select mask low module b */
	cimax_writereg(state, 0x0b, 0x1c);

	/* Auto select pattern high module b */
	cimax_writereg(state, 0x0c, 0x00);

	/* Auto select pattern low module b */
	cimax_writereg(state, 0x0d, 0x08);

	/* LOCK */
	cimax_writereg(state, 0x1f, 0x01);
#endif /* !octagon  */

#if defined(OCTAGON1008)
	/* POWER ON */
	cimax_writereg(state, 0x18, 0x21);
#else
	/* POWER ON */
	cimax_writereg(state, 0x18, 0x01);
#endif
	ctrl_outl(0x0, reg_config + EMI_LCK);
	ctrl_outl(0x0, reg_config + EMI_GEN_CFG);
#if defined(OCTAGON1008)
	ctrl_outl(0x8486d9, reg_config + EMIBank1 + EMI_CFG_DATA0);
	ctrl_outl(0x9d220000, reg_config + EMIBank1 + EMI_CFG_DATA1);
	ctrl_outl(0x9d220000, reg_config + EMIBank1 + EMI_CFG_DATA2);
	ctrl_outl(0x8, reg_config + EMIBank1 + EMI_CFG_DATA3);

	ctrl_outl(0x001016d9, reg_config + EMIBank3 + EMI_CFG_DATA0);
	ctrl_outl(0x9d200000, reg_config + EMIBank3 + EMI_CFG_DATA1);
	ctrl_outl(0x9d220000, reg_config + EMIBank3 + EMI_CFG_DATA2);
	ctrl_outl(0x00000008, reg_config + EMIBank3 + EMI_CFG_DATA3);

#elif defined(HL101) \
 || defined(VIP1_V1) \
 || defined(VIP1_V2)
	/*
	ctrl_outl(0x002045f9, reg_config + EMIBank3 + EMI_CFG_DATA0);
	ctrl_outl(0xa5888888, reg_config + EMIBank3 + EMI_CFG_DATA1);
	ctrl_outl(0xa5888888, reg_config + EMIBank3 + EMI_CFG_DATA2);
	ctrl_outl(0x04000000, reg_config + EMIBank3 + EMI_CFG_DATA3);
	*/
#else
	ctrl_outl(0xa306d9, reg_config + EMIBank4 + EMI_CFG_DATA0);
	ctrl_outl(0x7d336633, reg_config + EMIBank4 + EMI_CFG_DATA1);
	ctrl_outl(0x7d336633, reg_config + EMIBank4 + EMI_CFG_DATA2);
	ctrl_outl(0x0, reg_config + EMIBank4 + EMI_CFG_DATA3);

	ctrl_outl(0x2, reg_config + EMI_FLASH_CLK_SEL);
#endif
	ctrl_outl(0x1, reg_config + EMI_CLK_EN);

#if defined(OCTAGON1008)
// #if defined(OCTAGON1008)
	slot_membase[0] = (unsigned long) 0xa2000000;
// #else
//	slot_membase[0] = (unsigned long) ioremap(0xa3000000, 0x1000);  // TF7700
// #endif
	if (slot_membase[0] == NULL)
	{
		result = 1;
		goto error;
	}
#if defined(OCTAGON1008)
	slot_membase[1] = (unsigned long) 0xa2010000;
#else
	slot_membase[1] = (unsigned long) ioremap(0xa3010000, 0x1000);
#endif
	if (slot_membase[1] == NULL)
	{
		iounmap(slot_membase[0]);
		result = 1;
		goto error;
	}
#else  // !octagon

 #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
/* Dagobert: we have a 29Bit MMU so it is not possible
 * to ioremap addresses greater than 0x20000000, nevertheless
 * on stlinux22 it works to remap this. 
 * Not sure if we can get the "mapped" address via the
 * emi bank register or how this should work ;)
 * For now I know the mapped address and use it directly.
 */
   #if defined(HL101) \
    || defined(VIP1_V1) \
    || defined(VIP1_V2)
	reg_ci_base = (unsigned long) 0xa2c00000;
   #else
	reg_bank4 = (unsigned long) 0xa3000000;
   #endif
 #else  // kernel version
	reg_bank4 = (unsigned long) ioremap(0x43000000, 0x400000);
#endif  // kernel version
#endif  // !octagon
	ctrl_outl(0x1F,reg_config + EMI_LCK);
	dprintk(20, "%s: Call dvb_ca_en50221_init\n", __func__);
	if ((result = dvb_ca_en50221_init(core->dvb_adap, &core->ca, 0, 2)) != 0)
	{
		dprintk(1, "%s ca0 initialisation failed.\n", __func__);
		goto error;
	}
	dprintk(10, "CiMAX: ca0 interface initialised.\n");
	dprintk(150, "%s <\n", __func__);
	return 0;

error:
	dprintk(1, "%s < (result = %d)\n", __func__, result);
	return result;
}

EXPORT_SYMBOL(init_ci_controller);
EXPORT_SYMBOL(setCiSource);
EXPORT_SYMBOL(getCiSource);

/* ******************** */
/* konfetti: End cimax */
/* ******************** */

int __init cimax_init(void)
{
	dprintk(10, "CiMAX driver loaded\n");
	return 0;
}

void __exit cimax_exit(void)
{  
	dprintk(10, "CiMAX driver unloaded\n");
}

module_init (cimax_init);
module_exit (cimax_exit);

module_param(paramDebug, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(paramDebug, "Debug Output 0=disabled >0=enabled(debuglevel)");

MODULE_DESCRIPTION ("CI Controller");
MODULE_AUTHOR      ("Open Vision developers");
MODULE_LICENSE     ("GPL");
// vim:ts=4
