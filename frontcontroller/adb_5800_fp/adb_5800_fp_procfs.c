/*
 * adb_5800_fp_procfs.c
 *
 * (c) 20?? Gustav Gans
 * (c) 2019 Audioniek
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * procfs for ADB ITI-5800S(X) front panel driver.
 *
 *
 ****************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------
 * 20190215 Audioniek       Initial version based on cuberevo_micom_procfs.c.
 * 20190805 Audioniek       led_pattern2 and led_pattern3 added.
 * 20190807 Audioniek       /proc/stb/info/adb_variant added; replaces
 *                          /proc/boxtype.
 * 20190809 Audioniek       Split in nuvoton style between main, file and
 *                          procfs parts.
 * 20190813 Audioniek       /proc/stb/lcd/symbol_circle added.
 * 20190813 Audioniek       /proc/stb/fan/fan_ctrl added.
 * 20200504 Audioniek       Removed dual display capability.
 * 20200504 Audioniek       /proc/stb/fan/fan_ctrl always reports 0 on BSKA/BXZB.
 * 20200504 Audioniek       /proc/stb/lcd/symbol_circle always reports 0 on
 *                          BSKA/BXZB.
 * 20200505 Audioniek       Fix handling of BZZB with box_variant; boxvariant
 *                          values changed so that bit 1 signals dual tuner.
 * 20210810 Audioniek       Fix: BSLA and BZZB could not control fan.
 * 20210810 Audioniek       Add stb/fp/fan and stb/fp/fan_pwm entries for E2
 *                          compatiblity.
 * 
 ****************************************************************************/

#include <asm/io.h>
#include <linux/proc_fs.h>      /* proc fs */
#include <asm/uaccess.h>        /* copy_from_user */
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else
#include <linux/stpio.h>
#endif

#include "adb_5800_fp.h"

/*
 *  /proc/------
 *             |
 *             +--- progress (rw)           Progress of E2 startup in %
 *
 *  /proc/stb/fan/
 *             |
 *             +--- fan_ctrl (rw)           Control of fan speed (deprecated, use /proc/stb/fp/fan_pwm)
 *
 *  /proc/stb/fp/
 *             |
 *             +--- fan (rw)                Control of fan (on/off)
 *             +--- fan_pwm (rw)            Control of fan speed
 *             +--- led0_pattern (rw)       Blink pattern for LED 1 (currently limited to on (0xffffffff) or off (0))
 *             +--- led1_pattern (rw)       Blink pattern for LED 2 (currently limited to on (0xffffffff) or off (0))
 *             +--- led2_pattern (rw)       Blink pattern for LED 3 (currently limited to on (0xffffffff) or off (0))
 *             +--- led3_pattern (rw)       Blink pattern for LED 4 (currently limited to on (0xffffffff) or off (0))
 *             +--- led_patternspeed (rw)   Blink speed for pattern (not implemented)
 *             +--- oled_brightness (w)     Direct control of display brightness
 *             +--- rtc (rw)                RTC time (UTC, seconds since Unix epoch)) (not yet implemented)
 *             +--- rtc_offset (rw)         RTC offset in seconds from UTC (not yet implemented)
 *             +--- text (w)                Direct writing of display text
 *
 *  /proc/stb/info/
 *             +--- adb_variant (r)         Model variant: bska, bsla, bxzb, bzzb, cable or unknown
 *
 *  /proc/stb/lcd/
 *             |
 *             +--- symbol_circle (rw)      Control of spinner (VFD only)
 */

/* from e2procfs */
extern int install_e2_procs(char *name, read_proc_t *read_proc, write_proc_t *write_proc, void *data);
extern int remove_e2_procs(char *name, read_proc_t *read_proc, write_proc_t *write_proc);

/* from other front_bsla modules */
extern int pt6302_write_dcram(unsigned char addr, unsigned char *data, unsigned char len);
extern int pt6958_ShowBuf(unsigned char *data, unsigned char len, int utf8_flag);
extern void pt6302_set_brightness(int level);
extern void pt6958_set_brightness(int level);
extern void clear_display(void);
extern void pt6958_set_led(int led_nr, int level);
extern int adb_setFan(int speed);
//extern int micomSetWakeUpTime(char *time);
//extern int micomGetWakeUpMode(int *wakeup_mode);
//extern int micomGetVersion(unsigned int *data);
//extern static int rtc_time2tm(char *uTime, struct rtc_time *tm);

/* Globals */
extern int box_variant;   // 0=BSKA, 1=BSLA, 2=BXZB, 3=BZZB, 4=cable, -1=unknown
extern int display_type;  // active display: 0=LED, 1=VFD, 2=both
static int progress = 0;
static int symbol_circle = 0;
static int old_icon_state;
static u32 led0_pattern = 0;
static u32 led1_pattern = 0;
static u32 led2_pattern = 0;
static u32 led3_pattern = 0;
static int led_pattern_speed = 20;
extern unsigned long fan_registers;
static int fan_onoff = 1;
extern int fan_pwm;
extern struct stpio_pin* fan_pin;
extern tIconState spinner_state;
extern tIconState icon_state;
// for fan (BSLA/BZZB only)
//extern unsigned long fan_registers;
//extern struct stpio_pin* fan_pin;

static int progress_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char* page;
	char* myString;
	ssize_t ret = -ENOMEM;

	page = (char*)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
		{
			goto out;
		}
		myString = (char*) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count - 1] = '\0';

		sscanf(myString, "%d", &progress);
		kfree(myString);

		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	return ret;
}

static int progress_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page,"%d", progress);
	}
	return len;
}	

static int symbol_circle_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char* page;
	ssize_t ret = -ENOMEM;
	char* myString;
	int i = 0;

	page = (char*)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
		{
			goto out;
		}
		myString = (char*) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count - 1] = '\0';

		sscanf(myString, "%d", &symbol_circle);
		kfree(myString);

		if (display_type)
		{
			if (symbol_circle != 0)
			{  // spinner on
				if (symbol_circle > 255)
				{
					symbol_circle = 255;  // set maximum value
				}
				if (spinner_state.state == 0)  // if spinner not active
				{
					if (icon_state.state != 0)
					{  // stop icon thread if active
						old_icon_state = icon_state.state;
//						dprintk(50, "%s Stop icon thread\n", __func__);
						i = 0;
						icon_state.state = 0;
						do
						{
							msleep(250);
							i++;
						}
						while (icon_state.status != THREAD_STATUS_HALTED && i < 20);  //time out of 5 seconds
//						dprintk(50, "%s Icon thread stopped\n", __func__);
					}
					if (symbol_circle == 1)  // handle special value 1
					{
						spinner_state.period = 250;  //set standard speed
					}
					else
					{
						spinner_state.period = symbol_circle * 10;  //set user specified speed
					}
					spinner_state.state = 1;
					lastdata.icon_state[ICON_SPINNER] = 1;
					up(&spinner_state.sem);
				}
			}
			else
			{  // spinner off
				if (spinner_state.state != 0)
				{
					spinner_state.state = 0;
					lastdata.icon_state[ICON_SPINNER] = 0;
//					dprintk(50, "%s Stop spinner thread\n", __func__);
					i = 0;
					do
					{
						msleep(250);
						i++;
					}
					while (spinner_state.status != THREAD_STATUS_HALTED && i < 20);  //time out of 20 seconds
//					dprintk(50, "%s Spinner thread stopped\n", __func__);
				}
				if (old_icon_state != 0)  // restart icon thread when it was active
				{
					icon_state.state = old_icon_state;
					up(&icon_state.sem);
				}
			}
		}
		else
		{
			symbol_circle = 0;
		}
		/* always return count to avoid endless loop */
		ret = count;
		goto out;
	}
out:
	free_page((unsigned long)page);
	return ret;
}

static int symbol_circle_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page,"%d", symbol_circle);
	}
	return len;
}

static int text_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	int ret = -ENOMEM;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count) == 0)
		{
			if (display_type)
			{
				ret = pt6302_write_dcram(0, page, count);
			}
			else
			{
				ret = pt6958_ShowBuf(page, count, 0);
			}
			if (ret >= 0)
			{
				ret = count;
			}
		}
		free_page((unsigned long)page);
	}
	return ret;
}

#if 0
/* Time subroutines */
/*
struct tm {
int	tm_sec      //seconds after the minute 0-61*
int	tm_min      //minutes after the hour   0-59
int	tm_hour     //hours since midnight     0-23
int	tm_mday     //day of the month         1-31
int	tm_mon      //months since January     0-11
int	tm_year     //years since 1900
int	tm_wday     //days since Sunday        0-6
int	tm_yday     //days since January 1     0-365
}

time_t long seconds //UTC since epoch  
*/

int date2days(int year, int mon, int day, int *yday)
{
	int days;

	// process the days in the current month
	day--; // do not count today
	days = day;

	// process the days in the remaining months of this year
	mon--; // do not count the current month, as it is done already
	while (mon > 0)
	{
		days += _ytab[LEAPYEAR(year)][mon - 1];
		mon--;
	}
	*yday = days;
	// process the remaining years
	year--; // do not count current year, as it is done already
	while (year >= 0)
	{
		days += YEARSIZE(year);
		year--;
	}
	days += 10957; // add days since linux epoch
	return days;
}

time_t calcGetMicomTime(char *time)
{ //  6 byte string YMDhms -> seconds since epoch
	time_t time_local;
	int year, mon, day, days;
	int hour, min, sec;

	year = ((time[5] >> 4) * 10) + (time[5] & 0x0f);
	mon  = ((time[4] >> 4) * 10) + (time[4] & 0x0f);
	day  = ((time[3] >> 4) * 10) + (time[3] & 0x0f);
	hour = ((time[2] >> 4) * 10) + (time[2] & 0x0f);
	min  = ((time[1] >> 4) * 10) + (time[1] & 0x0f);
	sec  = ((time[0] >> 4) * 10) + (time[0] & 0x0f);

	// calculate the number of days since linux epoch
	days = date2days(year, mon, day, 0);
	time_local += days * 86400;
	// add the time
	time_local += (hour * 3600);
	time_local += (min * 60);
	time_local += sec;
	return time_local;
}

void calcSetMicomTime(time_t theTime, char *destString)
{ // seconds since epoch -> 12 byte string YYMMDDhhmmss
	struct tm calc_tm;
	register unsigned long dayclock, dayno;
	int year = 1970; // linux epoch

	dayclock = (unsigned long)theTime % 86400;
	dayno = (unsigned long)theTime / 86400;
	//dprintk(1, "%s dayclock: %u, dayno: %u\n", __func__, (int)dayclock, (int)dayno);

	calc_tm.tm_sec = dayclock % 60;
	calc_tm.tm_min = (dayclock % 3600) / 60;
	calc_tm.tm_hour = dayclock / 3600;
	calc_tm.tm_wday = (dayno + 4) % 7;  // day 0 was a thursday

	// calculate the year
	while (dayno >= YEARSIZE(year))
	{
		dayno -= YEARSIZE(year);
		year++;
	}
	calc_tm.tm_year = year - 2000; // RTC starts at 01-01-2000
	calc_tm.tm_yday = dayno;

	// calculate the month
	calc_tm.tm_mon = 0;
	while (dayno >= _ytab[LEAPYEAR(year)][calc_tm.tm_mon])
	{
		dayno -= _ytab[LEAPYEAR(year)][calc_tm.tm_mon];
		calc_tm.tm_mon++;
	}
	calc_tm.tm_mon++;
	calc_tm.tm_mday++;
	destString[0] = (calc_tm.tm_year >> 4) + '0';   // tens
	destString[1] = (calc_tm.tm_year & 0x0f) + '0'; // units
	destString[2] = (calc_tm.tm_mon >> 4) + '0';
	destString[3] = (calc_tm.tm_mon & 0x0f) + '0';
	destString[4] = (calc_tm.tm_mday >> 4) + '0';
	destString[5] = (calc_tm.tm_mday & 0x0f) + '0';
	destString[6] = (calc_tm.tm_hour >> 4) + '0';
	destString[7] = (calc_tm.tm_hour & 0x0f) + '0';
	destString[8] = (calc_tm.tm_min >> 4) + '0';
	destString[9] = (calc_tm.tm_min & 0x0f) + '0';
	destString[10] = (calc_tm.tm_sec >> 4) + '0';
	destString[11] = (calc_tm.tm_sec & 0x0f) + '0';
}

static int read_rtc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int res;
	long rtc_time;
	char fptime[6];

	memset(fptime, 0, sizeof(fptime));
	res = micomGetTime(fptime);  // get front panel clock time (local)
	rtc_time = calcGetMicomTime(fptime) - rtc_offset;  // return UTC

	if (NULL != page)
	{
		len = sprintf(page,"%u\n", (int)(rtc_time));
	}
	return len;
}

static int write_rtc(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *page = NULL;
	ssize_t ret = -ENOMEM;
	time_t argument = 0;
	int test = -1;
	char *myString = kmalloc(count + 1, GFP_KERNEL);
	char time[12];

	dprintk(50, "%s >\n", __func__);
	memset(time, 0, sizeof(time));
	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buffer, count))
		{
			goto out;
		}
		strncpy(myString, page, count);
		myString[count] = '\0';

		test = sscanf (myString, "%u", (unsigned int *)&argument);

		if (test > 0)
		{
			calcSetMicomTime((argument + rtc_offset), time);
			ret = micomSetTime(time); // update front panel clock in local time
		}
		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	kfree(myString);
	return ret;
}

static int read_rtc_offset(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%d\n", rtc_offset);
	}
	return len;
}

static int write_rtc_offset(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *page = NULL;
	ssize_t ret = -ENOMEM;
	int test = -1;
	char *myString = kmalloc(count + 1, GFP_KERNEL);

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buffer, count))
		{
			goto out;
		}
		strncpy(myString, page, count);
		myString[count] = '\0';

		test = sscanf (myString, "%d", &rtc_offset);

		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	kfree(myString);
	return ret;
}

static int wakeup_time_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *page = NULL;
	ssize_t ret = -ENOMEM;
	time_t wakeup_time = 0;
	int test = -1;
	char *myString = kmalloc(count + 1, GFP_KERNEL);
	char wtime[12];

	memset(wtime, 0, sizeof(wtime));
	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buffer, count))
		{
			goto out;
		}
		strncpy(myString, page, count);
		myString[count] = '\0';

		test = sscanf(myString, "%u", (unsigned int *)&wakeup_time);

		if (0 < test)
		{
			calcSetMicomTime((wakeup_time + rtc_offset), wtime);  //set FP wake up time (local)
			ret = micomSetWakeUpTime(wtime);
		}
		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	kfree(myString);
//	dprintk(10, "%s <\n", __func__);
	return ret;
}

static int wakeup_time_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char wtime[5];
	int	w_time;

	if (NULL != page)
	{
		
		micomGetWakeUpTime(wtime);  // get wakeup time from FP clock (local)
		w_time = calcGetMicomTime(wtime) - rtc_offset;  // UTC

		len = sprintf(page, "%u\n", w_time);
	}
	return len;
}

static int was_timer_wakeup_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int res = 0;
	int wakeup_mode = -1;

	dprintk(50, "%s >\n", __func__);
	if (NULL != page)
	{
		res = micomGetWakeUpMode(&wakeup_mode);

		if (res == 0)
		{
			if (wakeup_mode == 2) // if timer wakeup
			{
				wakeup_mode = 1;
			}
			else
			{
				wakeup_mode = 0;
			}
		}
		else
		{
				wakeup_mode = -1;
		}
		len = sprintf(page,"%d\n", wakeup_mode);
	}
	return len;
}
#endif

static int led_pattern_write(struct file *file, const char __user *buf, unsigned long count, void *data, int which)
{
	char *page;
	long pattern;
	int val;
	int ret = -ENOMEM;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';
			pattern = simple_strtol(page, NULL, 16);
//TODO: Not implemented completely; only the cases 0 (off)
// and 0xffffffff (on) are handled.
// Other patterns are blink patterned in time, so handling those should be done in a thread
			val = (pattern == 0xffffffff ? 1 : 0);

			switch (which)
			{
				case 0:
				{
					led0_pattern = pattern;
					pt6958_set_led(1, val);
					break;
				}
				case 1:
				{
					led1_pattern = pattern;
					pt6958_set_led(2, val);
					break;
				}
				case 2:
				{
					led2_pattern = pattern;
					pt6958_set_led(3, val);
					break;
				}
				case 3:
				{
					led3_pattern = pattern;
					pt6958_set_led(4, val);
					break;
				}
			}
		}
		ret = count;
	}
	free_page((unsigned long)page);
	return ret;
}

static int led0_pattern_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%08x\n", led0_pattern);
	}
	return len;
}

static int led0_pattern_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	return led_pattern_write(file, buf, count, data, 0);
}

static int led1_pattern_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%08x\n", led1_pattern);
	}
	return len;
}

static int led1_pattern_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	return led_pattern_write(file, buf, count, data, 1);
}

static int led2_pattern_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%08x\n", led2_pattern);
	}
	return len;
}

static int led2_pattern_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	return led_pattern_write(file, buf, count, data, 2);
}

static int led3_pattern_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%08x\n", led3_pattern);
	}
	return len;
}

static int led3_pattern_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	return led_pattern_write(file, buf, count, data, 3);
}

static int led_pattern_speed_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	int ret = -ENOMEM;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';
			led_pattern_speed = (int)simple_strtol(page, NULL, 10);

			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

static int led_pattern_speed_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%d\n", led_pattern_speed);
	}
	return len;
}

static int oled_brightness_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	long level;
	int  ret = -ENOMEM;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';

			level = simple_strtol(page, NULL, 10);
			if (display_type)
			{
				pt6302_set_brightness((int)level);
			}
			else
			{
				pt6958_set_brightness((int)level);
			}
			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

static int adb_variant_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		switch (box_variant)
		{
			case 0:
			{
				len = sprintf(page, "%s\n", "bska");
				break;
			}
			case 1:
			{
				len = sprintf(page, "%s\n", "bsla");
				break;
			}
			case 2:
			{
				len = sprintf(page, "%s\n", "bxzb");
				break;
			}
			case 3:
			{
				len = sprintf(page, "%s\n", "bzzb");
				break;
			}
			case 4:
			{
				len = sprintf(page, "%s\n", "cable");
				break;
			}
			default:
			{
				len = sprintf(page, "%s\n", "unknown");
				break;
			}
		}
	}
	return len;
}

static int fan_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	int ret = -ENOMEM;
	unsigned int value;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';
			fan_onoff = (unsigned int)simple_strtol(page, NULL, 10);

			if (fan_onoff != 0)
			{
				fan_onoff = 1;
			}
			if (box_variant & 0x01)  // BSLA or BZZB
			{
//				dprintk(50, "%s Fan value: %d\n", __func__, value);
				ctrl_outl((fan_onoff ? fan_pwm : 0), fan_registers + 0x04);
			}
			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

static int fan_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page,"%d", fan_onoff);
	}
	return len;
}

int fan_pwm_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	char *myString;
	int  ret = -ENOMEM;
	unsigned int value;
	
	page = (char *)__get_free_page(GFP_KERNEL);

	if (page) 
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';
			fan_pwm = (unsigned int)simple_strtol(page, NULL, 10);

			if (fan_pwm > 255)
			{
				fan_pwm = 255;
			}
			if (fan_pwm < 0)
			{
				fan_pwm = 0;
			}
			if (box_variant & 0x01)  // BSLA or BZZB
			{
//				dprintk(50, "%s Fan PWM value: %d\n", __func__, value);
				ctrl_outl(fan_pwm, fan_registers + 0x04);
			}
			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

int fan_pwm_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int      len = 0;
	unsigned int value;
	
	dprintk(10, "%s >\n", __func__);

	if (NULL != page)
	{
		if (box_variant & 0x01)  // BSLA or BZZB
		{
			value = fan_pwm;
		}
		else
		{
			value = 0;  // on BSKA and BXZB always return zero
		}
		len = sprintf(page, "%d\n", value);
	}
	return len;
}

/*
static int null_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	return count;
}
*/

struct fp_procs
{
	char *name;
	read_proc_t *read_proc;
	write_proc_t *write_proc;
} fp_procs[] =
{
	{ "progress", progress_read, progress_write },
	{ "stb/fan/fan_ctrl", fan_pwm_read, fan_pwm_write },
	{ "stb/fp/fan", fan_read, fan_write },
	{ "stb/fp/fan_pwm", fan_pwm_read, fan_pwm_write },
//	{ "stb/fp/rtc", read_rtc, write_rtc },
//	{ "stb/fp/rtc_offset", read_rtc_offset, write_rtc_offset },
	{ "stb/fp/led0_pattern", led0_pattern_read, led0_pattern_write },
	{ "stb/fp/led1_pattern", led1_pattern_read, led1_pattern_write },
	{ "stb/fp/led2_pattern", led2_pattern_read, led2_pattern_write },
	{ "stb/fp/led3_pattern", led3_pattern_read, led3_pattern_write },
	{ "stb/fp/led_pattern_speed", led_pattern_speed_read, led_pattern_speed_write },
	{ "stb/fp/oled_brightness", NULL, oled_brightness_write },
	{ "stb/fp/text", NULL, text_write },
	{ "stb/info/adb_variant", adb_variant_read, NULL },
	{ "stb/lcd/symbol_circle", symbol_circle_read, symbol_circle_write },
//	{ "stb/fp/wakeup_time", wakeup_time_read, wakeup_time_write },
//	{ "stb/fp/was_timer_wakeup", was_timer_wakeup_read, NULL },
//	{ "stb/fp/version", fp_version_read, NULL },
};

void create_proc_fp(void)
{
	int i;

	for (i = 0; i < sizeof(fp_procs) / sizeof(fp_procs[0]); i++)
	{
		install_e2_procs(fp_procs[i].name, fp_procs[i].read_proc, fp_procs[i].write_proc, NULL);
	}
}

void remove_proc_fp(void)
{
	int i;

	for (i = sizeof(fp_procs) / sizeof(fp_procs[0]) - 1; i >= 0; i--)
	{
		remove_e2_procs(fp_procs[i].name, fp_procs[i].read_proc, fp_procs[i].write_proc);
	}
}
// vim:ts=4
