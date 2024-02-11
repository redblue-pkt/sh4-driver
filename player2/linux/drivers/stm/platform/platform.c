/*
 * Player2 Platform registration
 *
 * Copyright (C) 2006 STMicroelectronics Limited
 * Author: Peter Bennett <peter.bennett@st.com>
 *
 * May be copied or modified under the terms of the GNU General Public
 * License. See linux/COPYING for more information.
 */

#include <linux/io.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/autoconf.h>

#include <asm/irq-ilc.h>
#include <asm/irq.h>

#if defined(CONFIG_CPU_SUBTYPE_STB7100)
#if defined(QBOXHD) || defined(QBOXHD_MINI)
#include "qboxhd.h"
#else
#include "mb442.h"
#include "hms1.h"
#endif
#include "platform_710x.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7200)
#include "mb520.h"
#include "cb10x.h"
#include "platform_7200.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7141)
#include "platform_7141.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7111)
#include "platform_7111.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7105)
#include "platform_7105.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7106)
#include "platform_7105.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7108)
#include "platform_7108.h"
#elif defined(CONFIG_CPU_SUBTYPE_STX7H205)
#include "platform_h205.h"
#endif

MODULE_DESCRIPTION("Player2 Platform Driver");
MODULE_AUTHOR("STMicroelectronics Limited");
MODULE_VERSION("0.9");
MODULE_LICENSE("GPL");
