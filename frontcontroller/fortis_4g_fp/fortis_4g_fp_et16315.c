/******************************************************************************
 *
 * fortis_4g_fp_et16315.c
 *
 * Frontpanel driver for Fortis 4th generation receivers, pio core part
 *
 * Supports: GPIO connected Etek ET16315 / Princeton PT6315 VFD/keyboard driver
 *
 * Based on code written by:
 * (c) 2009 Dagobert@teamducktales
 * (c) 2010 Schischu & konfetti: Add nuvoton irq handling
 *
 * (c) 2014-2020 Audioniek: rewritten for Fortis 4th generation receivers
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
 * Justification for this (separate) Fortis front panel module:
 * By tradition, the first 3 generations of Fortis HD receivers used the
 * nuvoton module for their front panel support. This module handles the front
 * panel display, the front panel keyboard, clock for the timer(s) and the
 * remote control.
 *
 * The 4th generation of Fortis receivers no longer have a separate front-
 * processor. The display and front panel keyboard are handled by non-
 * intelligent ICs, the remote control is handled by lirc and the timers by
 * the RTC in the SoC.
 * Deep standby mode used to be power down everything except the front
 * processor, on the 4G models the main board is put in "hibernation".
 * The hardware architecture is quite different, so that is reason one.
 *
 * The second reason is that the nuvoton code has become quite cluttered
 * because of the number and variants of receivers supported. Adding these
 * new receivers having a different hardware approach to the nuvoton code
 * would probably clutter it up even more.
 *
 *****************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * ----------------------------------------------------------------------------
 * 20141218 Audioniek      Start of work, based on nuvoton_file.c and the
 *                         original driver for Shenzen Titan Micro Electronics
 *                         TM1668 written by Pawel Moll <pawel.moll@st.com>
 *                         of STM. Thank you Pawel and nuvoton authors.
 * 20141224 Audioniek      Text display and LED control working.
 * 20141225 Audioniek      Icons working.
 * 20141229 Audioniek      Characterset now full ASCII.
 * 20141229 Audioniek      RTC code moved to fortis_4g_file.c.
 * 20141230 Audioniek      Bug fixed with displaying string longer than display
 *                         size.
 * 20141231 Audioniek      Display string longer than display size is scrolled
 *                         once.
 * 20150205 Audioniek      GPV8000 and EP8000 models added.
 * 20191208 Audioniek      FOREVER_3434HD model added.
 * 20191209 Audioniek      Support for blue led added.
 * 20191216 Audioniek      Streamline LED code.
 * 20191221 Audioniek      Fix inverted logo LED drive.
 * 20191225 Audioniek      Fix non-working channel up front panel key.
 * 20200122 Audioniek      Simplify brightness control.
 * 20200201 Audioniek      FOREVER_9898HD: handle red LED differently.
 * 20200313 Audioniek      Add UTF8 support (untested yet).
 *
 *****************************************************************************/

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/termbits.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/gpio.h>
#include <linux/stm/pio.h>
//#include <linux/stm/gpio.h>
#include <linux/workqueue.h>
//#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/poll.h>
#include <linux/input.h>
#include <linux/platform_device.h>

#include "fortis_4g_fp.h"
#include "fortis_4g_fp_et16315.h"
#include "fortis_4g_fp_utf.h"

#if defined(FOREVER_3434HD) \
 || defined(FOREVER_9898HD) \
 || defined(GPV8000) \
 || defined(EP8000) \
 || defined(EPP8000)
#define DISP_SIZE 8
#else
#warning Box model not supported!
#endif

#if defined TAGDEBUG
#undef TAGDEBUG
#endif
#define TAGDEBUG "[fortis_4g_et16315] "

static struct et16315_chip *et16315_data;  // global values for IOCTL routines

/****************************************************************************************
 *
 * Character table for 8 digit 15-segment VFD display
 *
 ****************************************************************************************
 *
 * In the Fortis FOREVER_3434HD/FOREVER_9898HD/GPV8000/EP8000/EPP8000 front panel the ET16315 driver IC
 * is wired as follows to the display (TODO/assumption: GPV/EP8000 are the same):
 *
 *     aaaaaaa
 *    fh  j  kb
 *    f h j k b  q
 *    f  hjk  b
 * Q   gggimmm
 *    e  rpn  c
 *    e r p n c  q
 *    er  p  nc
 *     ddddddd
 *
 * SG1 -> segment a (0x01, value0) 
 * SG2 -> segment k (0x02, value0)
 * SG3 -> segment j (0x04, value0)
 * SG4 -> segment h (0x08, value0)
 * SG5 -> segment b (0x10, value0)
 * SG6 -> segment f (0x20, value0)
 * SG7 -> segment m (0x40, value0)
 * SG8 -> segment i (0x80, value0)
 *
 * SG9 -> segment g (0x01, value1) 
 * SG10-> segment c (0x02, value1)
 * SG11-> segment e (0x04, value1)
 * SG12-> segment n (0x08, value1)
 * SG13-> segment p (0x10, value1)
 * SG14-> segment r (0x20, value1)
 * SG15-> segment d (0x40, value1)
 * SG16-> segment Q/q (0x80, value1) red dot on digit 1, colon on digits 2, 4 & 6
 *
 * SG17-SG24 not connected -> all value2 are zero
 *
 * GR1 is connected to the rightmost digit 8, GR8 to the leftmost digit 1
 * (reverse of what you would expect)
 *
 * The characters below must be in ASCII order, starting at space (0x20).
 */
static struct et16315_char et16315_fp_chars[] =
{
	/*    value0      value1      value2                                   */
	/*  imfbhjka    qdrpnecg             <-display segment                 */
	/*  76543210    76543210    76543210      ASCII  character/description */
	{ 0b00000000, 0b00000000, 0b00000000 },  // 020, space 
	{ 0b10000100, 0b00010000, 0b00000000 },  // 021, '!'
	{ 0b00110000, 0b00000000, 0b00000000 },  // 022, double quote
	{ 0b11000000, 0b01000111, 0b00000000 },  // 023, '#'
	{ 0b11100101, 0b01010011, 0b00000000 },  // 024, '$'
	{ 0b10100010, 0b00100010, 0b00000000 },  // 025, '%'
	{ 0b10101000, 0b01001101, 0b00000000 },  // 026, '&'
	{ 0b00000010, 0b00000000, 0b00000000 },  // 027, quote
	{ 0b10000010, 0b00001000, 0b00000000 },  // 028, '('
	{ 0b10001000, 0b00100000, 0b00000000 },  // 029, ')'
	{ 0b11001010, 0b00101001, 0b00000000 },  // 02a, '*'
	{ 0b11000100, 0b00010001, 0b00000000 },  // 02b, '+'
	{ 0b10000000, 0b00100000, 0b00000000 },  // 02c, ','
	{ 0b11000000, 0b00000001, 0b00000000 },  // 02d, '-'
	{ 0b00000000, 0b00100000, 0b00000000 },  // 02e, '.'
	{ 0b10000010, 0b00100000, 0b00000000 },  // 02f, '/'
	{ 0b10110001, 0b01000110, 0b00000000 },  // 030, '0'
	{ 0b00010010, 0b00000010, 0b00000000 },  // 031, '1'
	{ 0b11010001, 0b01100000, 0b00000000 },  // 032, '2'
	{ 0b11010001, 0b01000010, 0b00000000 },  // 033, '3'
	{ 0b11110000, 0b00000011, 0b00000000 },  // 034, '4'
	{ 0b11100001, 0b01000011, 0b00000000 },  // 035, '5'
	{ 0b11100001, 0b01000111, 0b00000000 },  // 036, '6'
	{ 0b10000011, 0b00100000, 0b00000000 },  // 037, '7'
	{ 0b11110001, 0b01000111, 0b00000000 },  // 038, '8'
	{ 0b11110001, 0b01000011, 0b00000000 },  // 039, '9'
	{ 0b00000000, 0b00000000, 0b00000000 },  // 03a, ':'
	{ 0b00000000, 0b00000000, 0b00000000 },  // 03b, ';'
	{ 0b10000010, 0b00001000, 0b00000000 },  // 03c, '<'
	{ 0b11000000, 0b01000001, 0b00000000 },  // 03d, '='
	{ 0b10001000, 0b00100000, 0b00000000 },  // 03e, '>'
	{ 0b00000000, 0b00000000, 0b00000000 },  // 03f, '?'
	{ 0b11110001, 0b01000101, 0b00000000 },  // 040, '@'
	{ 0b11110001, 0b00000111, 0b00000000 },  // 041, 'A'
	{ 0b11010101, 0b01010010, 0b00000000 },  // 042, 'B'
	{ 0b00100001, 0b01000100, 0b00000000 },  // 043, 'C'
	{ 0b10010101, 0b01010010, 0b00000000 },  // 044, 'D'
	{ 0b11100001, 0b01000101, 0b00000000 },  // 045, 'E'
	{ 0b11100001, 0b00000101, 0b00000000 },  // 046, 'F'
	{ 0b01100001, 0b01000110, 0b00000000 },  // 047, 'G'
	{ 0b11110000, 0b00000111, 0b00000000 },  // 048, 'H'
	{ 0b10000101, 0b01010000, 0b00000000 },  // 049, 'I'
	{ 0b00010000, 0b01000010, 0b00000000 },  // 04a, 'J'
	{ 0b10100010, 0b00001101, 0b00000000 },  // 04b, 'K'
	{ 0b00100000, 0b01000100, 0b00000000 },  // 04c, 'L'
	{ 0b10111010, 0b00000110, 0b00000000 },  // 04d, 'M'
	{ 0b10111000, 0b00001110, 0b00000000 },  // 04e, 'N'
	{ 0b00110001, 0b01000110, 0b00000000 },  // 04f, 'O'
	{ 0b11110001, 0b00000101, 0b00000000 },  // 050, 'P'
	{ 0b00110001, 0b01001110, 0b00000000 },  // 051, 'Q'
	{ 0b11110001, 0b00001101, 0b00000000 },  // 052, 'R'
	{ 0b11100001, 0b01000011, 0b00000000 },  // 053, 'S'
	{ 0b10000101, 0b00010000, 0b00000000 },  // 054, 'T'
	{ 0b00110000, 0b01000110, 0b00000000 },  // 055, 'U'
	{ 0b10100010, 0b00100100, 0b00000000 },  // 056, 'V'
	{ 0b10110000, 0b00101110, 0b00000000 },  // 057, 'W'
	{ 0b10001010, 0b00101000, 0b00000000 },  // 058, 'X'
	{ 0b11110000, 0b00010001, 0b00000000 },  // 059, 'Y'
	{ 0b10000011, 0b01100000, 0b00000000 },  // 05a, 'Z'
	{ 0b00100001, 0b01000100, 0b00000000 },  // 05b, '['
	{ 0b10001000, 0b00001000, 0b00000000 },  // 05c, back slash
	{ 0b00010001, 0b01000010, 0b00000000 },  // 05d, ']'
	{ 0b00110001, 0b00000000, 0b00000000 },  // 05e, '^'
	{ 0b00000000, 0b01000000, 0b00000000 },  // 05f, '_'
	{ 0b00001000, 0b00000000, 0b00000000 },  // 060, back quote
	{ 0b11010001, 0b01000111, 0b00000000 },  // 061, 'a'
	{ 0b11100000, 0b01000111, 0b00000000 },  // 062, 'b'
	{ 0b11000000, 0b01000101, 0b00000000 },  // 063, 'c'
	{ 0b11010000, 0b01000111, 0b00000000 },  // 064, 'd'
	{ 0b11110001, 0b01000101, 0b00000000 },  // 065, 'e'
	{ 0b10100001, 0b00000101, 0b00000000 },  // 066, 'f'
	{ 0b11110001, 0b01000011, 0b00000000 },  // 067, 'g'
	{ 0b11100000, 0b00000111, 0b00000000 },  // 068, 'h'
	{ 0b10000000, 0b00010000, 0b00000000 },  // 069, 'i'
	{ 0b00010000, 0b01000010, 0b00000000 },  // 06a, 'j'
	{ 0b11100000, 0b00001101, 0b00000000 },  // 06b, 'k'
	{ 0b10000100, 0b00010000, 0b00000000 },  // 06c, 'l'
	{ 0b11000000, 0b00010111, 0b00000000 },  // 06d, 'm'
	{ 0b11000000, 0b00000111, 0b00000000 },  // 06e, 'n'
	{ 0b11000000, 0b01000111, 0b00000000 },  // 06f, 'o'
	{ 0b11110001, 0b00000101, 0b00000000 },  // 070, 'p'
	{ 0b11110001, 0b00000011, 0b00000000 },  // 071, 'q'
	{ 0b11000000, 0b00000101, 0b00000000 },  // 072, 'r'
	{ 0b11100001, 0b01000011, 0b00000000 },  // 073, 's'
	{ 0b10100000, 0b01000101, 0b00000000 },  // 074, 't'
	{ 0b00000000, 0b01000110, 0b00000000 },  // 075, 'u'
	{ 0b10100010, 0b00100100, 0b00000000 },  // 076, 'v'
	{ 0b10110000, 0b00101110, 0b00000000 },  // 077, 'w'
	{ 0b10001010, 0b00101000, 0b00000000 },  // 078, 'x'
	{ 0b10001010, 0b00100000, 0b00000000 },  // 079, 'y'
	{ 0b10000011, 0b01100000, 0b00000000 },  // 07a, 'z'
	{ 0b10000010, 0b00001000, 0b00000000 },  // 07b, '{'
	{ 0b00000100, 0b00010000, 0b00000000 },  // 07c, '|'
	{ 0b10001000, 0b00100000, 0b00000000 },  // 07d, '}'
	{ 0b01000000, 0b00000001, 0b00000000 },  // 07e, '~'
	{ 0b11000000, 0b01000111, 0b00000000 },  // 07f, DEL
// Note: following values are cyrillic patterns (offset 0x60 - 0df)  // TODO: determine segment patterns
	{ 0b00000000, 0b00000000, 0b00000000 },  // 080, Ѐ cyrillic capital letter ie with grave
	{ 0b00000000, 0b00000000, 0b00000000 },  // 081, Ё cyrillic capital letter io
	{ 0b00000000, 0b00000000, 0b00000000 },  // 082, Ђ cyrillic capital letter dje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 083, Ѓ cyrillic capital letter gje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 084, Є cyrillic capital letter ukrainian ie
	{ 0b00000000, 0b00000000, 0b00000000 },  // 085, Ѕ cyrillic capital letter dze
	{ 0b00000000, 0b00000000, 0b00000000 },  // 086, І cyrillic capital letter byelorussian-ukrainian i
	{ 0b00000000, 0b00000000, 0b00000000 },  // 087, Ї cyrillic capital letter yi
	{ 0b00000000, 0b00000000, 0b00000000 },  // 088, Ј cyrillic capital letter je
	{ 0b00000000, 0b00000000, 0b00000000 },  // 089, Љ cyrillic capital letter lje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 08a, Њ cyrillic capital letter nje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 08b, Ћ cyrillic capital letter tshe
	{ 0b00000000, 0b00000000, 0b00000000 },  // 08c, Ќ cyrillic capital letter kje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 08d, Ѝ cyrillic capital letter i with grave
	{ 0b00000000, 0b00000000, 0b00000000 },  // 08e, Ў cyrillic capital letter short u
	{ 0b00000000, 0b00000000, 0b00000000 },  // 08f, Џ cyrillic capital letter dzhe

	{ 0b00000000, 0b00000000, 0b00000000 },  // 090, А cyrillic capital letter a
	{ 0b00000000, 0b00000000, 0b00000000 },  // 091, Б cyrillic capital letter be
	{ 0b00000000, 0b00000000, 0b00000000 },  // 092, В cyrillic capital letter ve
	{ 0b00000000, 0b00000000, 0b00000000 },  // 093, Г cyrillic capital letter ghe
	{ 0b00000000, 0b00000000, 0b00000000 },  // 094, Д cyrillic capital letter de
	{ 0b00000000, 0b00000000, 0b00000000 },  // 095, Е cyrillic capital letter ie
	{ 0b00000000, 0b00000000, 0b00000000 },  // 096, Ж cyrillic capital letter zhe
	{ 0b00000000, 0b00000000, 0b00000000 },  // 097, З cyrillic capital letter ze
	{ 0b00000000, 0b00000000, 0b00000000 },  // 098, И cyrillic capital letter i
	{ 0b00000000, 0b00000000, 0b00000000 },  // 099, Й cyrillic capital letter short i
	{ 0b00000000, 0b00000000, 0b00000000 },  // 09a, К cyrillic capital letter ka
	{ 0b00000000, 0b00000000, 0b00000000 },  // 09b, Л cyrillic capital letter el
	{ 0b00000000, 0b00000000, 0b00000000 },  // 09c, М cyrillic capital letter em
	{ 0b00000000, 0b00000000, 0b00000000 },  // 09d, Н cyrillic capital letter en
	{ 0b00000000, 0b00000000, 0b00000000 },  // 09e, О cyrillic capital letter o
	{ 0b00000000, 0b00000000, 0b00000000 },  // 09f, П cyrillic capital letter pe

	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a0, Р Cyrillic capital letter er
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a1, С Cyrillic capital letter es
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a2, Т Cyrillic capital letter te
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a3, У Cyrillic capital letter u
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a4, Ф Cyrillic capital letter ef
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a5, Х Cyrillic capital letter ha
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a6, Ц Cyrillic capital letter tse
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a7, Ч Cyrillic capital letter che
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a8, Ш Cyrillic capital letter sha
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0a9, Щ Cyrillic capital letter shcha
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0aa, Ъ Cyrillic capital letter hard sign
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ab, Ы Cyrillic capital letter yeru
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ac, Ь Cyrillic capital letter soft sign
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ad, Э Cyrillic capital letter e
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ae, Ю Cyrillic capital letter yu
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0af, Я Cyrillic capital letter ya

	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b0, а Cyrillic small letter a
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b1, б Cyrillic small letter be
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b2, в Cyrillic small letter ve
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b3, г Cyrillic small letter ghe
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b4, д Cyrillic small letter de
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b5, е Cyrillic small letter ie
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b6, ж Cyrillic small letter zhe
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b7, з Cyrillic small letter ze
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b8, и Cyrillic small letter i
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0b9, й Cyrillic small letter short i
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ba, к Cyrillic small letter ka
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0bb, л Cyrillic small letter el
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0bc, м Cyrillic small letter em
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0bd, н Cyrillic small letter en
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0be, о Cyrillic small letter o
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0bf, п Cyrillic small letter pe

	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c0, р Cyrillic small letter er
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c1, с Cyrillic small letter es
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c2, т Cyrillic small letter te
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c3, у Cyrillic small letter u
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c4, ф Cyrillic small letter ef
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c5, х Cyrillic small letter ha
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c6, ц Cyrillic small letter tse
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c7, ч Cyrillic small letter che
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c8, ш Cyrillic small letter sha
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0c9, щ Cyrillic small letter shcha
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ca, ъ Cyrillic small letter hard sign
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0cb, ы Cyrillic small letter yeru
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0cc, ь Cyrillic small letter soft sign
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0cd, э Cyrillic small letter e
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ce, ю Cyrillic small letter yu
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0cf, я Cyrillic small letter ya

	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d0, ѐ Cyrillic small letter ie with grave
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d1, ё Cyrillic small letter io
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d2, ђ Cyrillic small letter dje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d3, ѓ Cyrillic small letter gje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d4, є Cyrillic small letter ukrainian ie
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d5, ѕ Cyrillic small letter dze
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d6, і Cyrillic small letter byelorussian-ukrainian i
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d7, ї Cyrillic small letter yi
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d8, ј Cyrillic small letter je
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0d9, љ Cyrillic small letter lje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0da, њ Cyrillic small letter nje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0db, ћ Cyrillic small letter tshe
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0dc, ќ Cyrillic small letter kje
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0dd, ѝ Cyrillic small letter i with grave
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0de, ў Cyrillic small letter short u
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0df, џ Cyrillic small letter dzhe

	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e0, Ѡ Cyrillic capital letter omega
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e1, ѡ Cyrillic small letter omega
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e2, Ѣ Cyrillic capital letter yat
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e3, ѣ Cyrillic small letter yat
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e4, Ѥ Cyrillic capital letter iotified e
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e5, ѥ Cyrillic small letter iotified e
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e6, Ѧ Cyrillic capital letter little yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e7, ѧ Cyrillic small letter little yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e8, Ѩ Cyrillic capital letter iotified little yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0e9, ѩ Cyrillic small letter iotified little yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ea, Ѫ Cyrillic capital letter big yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0eb, ѫ Cyrillic small letter big yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ec, Ѭ Cyrillic capital letter iotified big yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ed, ѭ Cyrillic small letter iotified big yus
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ee, Ѯ Cyrillic capital letter ksi
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ef, ѯ Cyrillic small letter ksi

	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f0, Ѱ Cyrillic capital letter psi
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f1, ѱ Cyrillic small letter psi
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f2, Ѳ Cyrillic capital letter fita
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f3, ѳ Cyrillic small letter fita
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f4, Ѵ Cyrillic capital letter izhitsa
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f5, ѵ Cyrillic small letter izhitsa
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f6, Ѷ Cyrillic capital letter izhitsa with double grave accent
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f7, ѷ Cyrillic small letter izhitsa with double grave accent
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f8, Ѹ Cyrillic capital letter uk
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0f9, ѹ Cyrillic small letter uk
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0fa, Ѻ Cyrillic capital letter round omega
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0fb, ѻ Cyrillic small letter round omega
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0fc, Ѽ Cyrillic capital letter omega with titlo
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0fd, ѽ Cyrillic small letter omega with titlo
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0fe, Ѿ Cyrillic capital letter ot
	{ 0b00000000, 0b00000000, 0b00000000 },  // 0ff, ѿ Cyrillic small letter ot
};

/****************************************************************************************
 *
 * Front panel keyboard definitions
 *
 ****************************************************************************************
 * Keyboard polling state
 * 0 = stopped
 * 1 = normal polling
 * 2 = stop requested
 */
static int polling = 1;
static struct workqueue_struct *keyboard_workqueue;
#if defined(FOREVER_3434HD) \
 || defined(FOREVER_9898HD) \
 || defined(GPV8000) \
 || defined(EP8000) \
 || defined(EPP8000)
/* Front panel keys are connected as follows:
  
  Channel Up  : between KS1 & K1 -> bit 7 of byte 0
  Channel Down: between KS1 & K2 -> bit 6 of byte 0
  Volume Up   : between KS2 & K1 -> bit 5 of byte 0
  Volume Down : between KS2 & K2 -> bit 4 of byte 0
  PWR key is connected to GPIO pin 3,1 (not to the ET16315)
*/
static struct et16315_key fp_keys[] =
{
	{ 0x00000080, KEY_UP,    "Channel Up" },
	{ 0x00000040, KEY_DOWN,  "Channel Down" },
	{ 0x00000020, KEY_RIGHT, "Volume Up" },
	{ 0x00000010, KEY_LEFT,  "Volume Down" },
};
#else
#warning No front panel keys defined!
#endif

/****************************************************************************************
 *
 * Initial values and settings for ET16315
 *
 ****************************************************************************************/
static struct et16315_platform_data et16315_init_values =
{
	/* Wiring info */
	.gpio_din    = ET16315_DIN,
	.gpio_dout   = ET16315_DOUT,
	.gpio_clk    = ET16315_CLK,
	.gpio_stb    = ET16315_STB,

	/* VFD display */
	.digits      = et16315_config_8_digits_20_segments,  // 8 character display
	.brightness  = MAX_BRIGHT,
	.on          = 1,  // display enable
	.char_tbl    = et16315_fp_chars,
#if defined(FOREVER_3434HD)
	.text        = "FOREVER_3434HD",  /* initial display text */
#elif defined(FOREVER_9898HD)
	.text        = "FOREVER_9898HD",
#elif defined(EP8000)
	.text        = "EP8000",
#elif defined(EPP8000)
	.text        = "EPP8000",
#elif defined(GPV8000)
	.text        = "GPV8000",
#else
	.text        = "--------",
#endif
	/* LEDs */
#if defined(FOREVER_3434HD)
	.led         = LED_BLUE + LED_LOGO,  // blue & logo on
#elif defined(FOREVER_9898HD)
	.led         = LED_RED,  // all LEDs off
#elif defined(GPV8000) \
 || defined(EP8000) \
 || defined(EPP8000)
	.led         = LED_LOGO,  // logo on, rest off
#else
	.led         = 0,  // all LEDs off
#endif

	/* Keyboard */
	.keys_num    = ARRAY_SIZE(fp_keys),
	.keys        = fp_keys,
	.keys_poll   = 80,  /* ms, 80 is worst case value for ET16315 */
	.driver_name = "ET16315 front panel keyboard",
};

/****************************************************************************************
 *
 * GPIO serial routines for ET16315
 *
 ****************************************************************************************/
static void et16315_writeb(struct et16315_chip *chip, u8 byte)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		gpio_set_value(chip->gpio_din, byte & 0b00000001);
		gpio_set_value(chip->gpio_clk, 0);
		udelay(1);
		gpio_set_value(chip->gpio_clk, 1);
		udelay(1);

		byte >>= 1;
	}
}

static void et16315_readb(struct et16315_chip *chip, u8 *byte)
{  // Read one byte (key data)
	int i;

	*byte = 0;
	for (i = 0; i < 8; i++)
	{
		gpio_set_value(chip->gpio_clk, 0);
		udelay(2);
		gpio_set_value(chip->gpio_clk, 1);
		udelay(1);
		*byte <<= 1;  // note: MSbit is received first
		*byte |= (gpio_get_value(chip->gpio_dout) & 0b00000001);
	}
}

static void et16315_send(struct et16315_chip *chip, u8 command, const void *buf, int len)
{  // Send command and [len] parameter bytes
	const u8 *data = buf;
	int i;

	BUG_ON(len > ET16315_DISPLAY_BUFFER_SIZE);

#if 0
	if (paramDebug >= 20)
	{
		dprintk(20, "%s: command: 0x%02X (len = %d)", __func__, command, len);
		for (i = 0; i < len; i++)
		{
			printk(" [0x%02X]", data[i]);
		}
		printk("\n");
	}
#endif
	gpio_set_value(chip->gpio_stb, 0);
	udelay(1);
	et16315_writeb(chip, command);
	udelay(1);

	for (i = 0; i < len; i++)
	{
		et16315_writeb(chip, *data++);
		udelay(1);
	}
	gpio_set_value(chip->gpio_stb, 1);
	udelay(2);
}

static void et16315_recv(struct et16315_chip *chip, u8 command, void *buf, int len)
{
	u8 *data = buf;
	int i;

	gpio_set_value(chip->gpio_stb, 0);
	udelay(1);
//	dprintk(10, "%s Command = 0x%02x, # args = %d\n", __func__, command, len);
	et16315_writeb(chip, command);
	udelay(3);

	for (i = 0; i < len; i++)
	{
		et16315_readb(chip, data++);
//		dprintk(10, "%s read byte #%02d = 0x%02x\n", __func__, i, *data);
		udelay(1);
	}
	gpio_set_value(chip->gpio_stb, 1);
	udelay(2);
}

/* LED routine; control LED port pins */
static void et16315_set_led(struct et16315_chip *chip, int leds)
{
	u8 data[1] = { (leds & 0b00001111) ^ 0b00001111 };

	dprintk(20, "%s > leds = 0x%02x\n", __func__, leds);
	chip->led = leds;

	dprintk(10, "Send cmd=0x%02x + data 0x%02x\n", (int)ET16315_CMD2_DATA_SET(0, ET16315_FIXED_ADDR, ET16315_CMD_SET_LED), (int)data[0]);
	et16315_send(chip, ET16315_CMD2_DATA_SET(0, ET16315_FIXED_ADDR, ET16315_CMD_SET_LED), data, 1);
}

/* Display routines */
static void et16315_clear(struct et16315_chip *chip)
{
	u8 data[ET16315_DISPLAY_BUFFER_SIZE];

	memset(data, 0, sizeof(data));  // clear display RAM

	et16315_send(chip, ET16315_CMD2_DATA_SET(0, ET16315_AUTO_ADDR_INC, ET16315_CMD_WRITE_DATA), NULL, 0);
	et16315_send(chip, ET16315_CMD3_SET_ADDRESS(0), data, sizeof(data));
	memcpy(chip->last_display, data, sizeof(data));
}

static void et16315_set_brightness(struct et16315_chip *chip, int brightness)
{
	if (brightness < 0)
	{
		brightness = 0;
	}
	else if (brightness > MAX_BRIGHT)
	{
		brightness = MAX_BRIGHT;
	}
	et16315_send(chip, ET16315_CMD4_DISPLAY_CONTROL(et16315_data->on, brightness), NULL, 0);
	et16315_data->brightness = brightness;
}

static void et16315_set_light(struct et16315_chip *chip, int on)
{
	if (on < 0)
	{
		on = 0;
	}
	else if (on > 1)
	{
		on = 1;
	}
	et16315_send(chip, ET16315_CMD4_DISPLAY_CONTROL(on, et16315_data->brightness), NULL, 0);
	et16315_data->on = on;
}

static void et16315_set_text(struct et16315_chip *chip, const char *text)
{
	u8 data[ET16315_DISPLAY_BUFFER_SIZE] = { 0, };
	int i, len;

	len = strlen(text);
	if (len > chip->digits)
	{
		len = chip->digits;
	}
//	dprintk(10, "%s Display [%s] (length = %02d)\n", __func__, text, len);

	/* On FOREVER_3434HD, FOREVER_9898HD, EP8000, EPP8000 & GPV8000, display digits are connected right to left */
	for (i = 3 * (chip->digits - 1); i > 3 * (chip->digits - 1 - len); i -= 3)
	{
		chip->last_display[i] = data[i] = chip->char_tbl[*text - 0x20].value0;
		chip->last_display[i + 1] = data[i + 1] = chip->char_tbl[*text - 0x20].value1 | (chip->last_display[i + 1] & 0b10000000);
//		chip->last_display[i + 2] = data[i + 2] = chip->char_tbl[*text - 0x20].value2;
		chip->last_display[i + 2] = data[i + 2] = 0;
		text++;
 
	}
	// update last display data
	for (i = 0; i < 3 * (chip->digits - len); i += 3)
	{
		chip->last_display[i] = data[i] = 0;
		chip->last_display[i + 1] = data[i + 1] = (chip->last_display[i + 1] & 0b10000000);
		chip->last_display[i + 2] = data[i + 2] = 0;
	}
	et16315_send(chip, ET16315_CMD3_SET_ADDRESS(0), data, (chip->digits * 3));
}

/****************************************************************************************
 *
 * LED routines
 *
 ****************************************************************************************/
static void init_leds(struct et16315_chip *chip)
{
	struct et16315_platform_data *et16315_init = &et16315_init_values;

	dprintk(100, "%s >\n", __func__);
#if defined(FOREVER_3434HD) \
 || defined(GPV8000) \
 || defined(EP8000) \
 || defined(EPP8000)
	// blink red led
	et16315_set_led(chip, LED_RED);
	msleep(200);
	et16315_set_led(chip, 0);  // all LEDs off
	msleep(200);
	gpio_free(GPIO_LED_LOGO);
	gpio_request(GPIO_LED_LOGO, "LED_LOGO");
	gpio_direction_output(GPIO_LED_LOGO, 1);  // drive of LED_LOGO is inverted
#if defined(FOREVER_3434HD)
	// blink blue led
	et16315_set_led(chip, LED_BLUE);
	msleep(200);
	et16315_set_led(chip, 0);  // all LEDs off
	msleep(200);
#endif
	/* Set initial LED state */
	chip->led = et16315_init->led;
	fortis_4gSetLED(chip->led, 1);
#elif defined(FOREVER_9898HD)
	dprintk(1, "%s FOREVER_9898HD detected: Red LED is GPIO driven?\n", __func__);
//	gpio_free(GPIO_LED_RED);
//	gpio_request(GPIO_LED_RED, "LED_RED");
//	gpio_direction_output(GPIO_LED_RED, 1);
//	msleep(200);
//	gpio_direction_output(GPIO_LED_RED, 0);
#else
	dprintk(1, "%s Unsupported receiver model detected!\n", __func__);
#endif
	dprintk(100, "%s <\n", __func__);
}

static void cleanup_leds(void)
{
#if defined(FOREVER_3434HD) \
 || defined(GPV8000) \
 || defined(EP8000) \
 || defined(EPP8000)
	gpio_free(GPIO_LED_LOGO);
#elif defined(FOREVER_9898HD)
//	gpio_free(GPIO_LED_RED);
#endif
}

/****************************************************************************************
 *
 * Front panel keyboard device routines
 *
 ****************************************************************************************/
static void keyboard_poll(struct work_struct *work)
{
	int i;
	u32 key_data, diff;

	while (polling == 1)
	{
//		spin_lock(&et16315_data->lock);
//		et16315_recv(et16315_data, ET16315_CMD2_DATA_SET(0, ET16315_FIXED_ADDR, ET16315_CMD_READ_KEY), &key_data, 4);
		et16315_recv(et16315_data, ET16315_CMD2_DATA_SET(0, ET16315_AUTO_ADDR_INC, ET16315_CMD_READ_KEY), &key_data, 4);
//		spin_unlock(&et16315_data->lock);

		/* If key data is different from last time, process it */
		diff = key_data ^ et16315_data->key_prev;

		if (diff)
		{
			dprintk(20, "Key data from ET16315: 0x%04x%04x\n", ((key_data & 0xffff0000) >> 4), (key_data & 0xffff));
		}
		for (i = 0; i < et16315_data->keys_num; i++)
		{
			struct et16315_key *key = &et16315_data->keys[i];

			if (diff & key->mask)
			{
				dprintk(20, "Key received: %s (code 0x%02x, 0x%04x%04x)\n", key->desc, key->code, ((key_data & key->mask & 0xffff0000) >> 4),(key_data & key->mask & 0xffff));
				
//				input_event(et16315_data->keyboard, EV_KEY, key->code, !!(key_data & key->mask));
//				input_sync(et16315_data->keyboard);
				fortis_4gSetIcon(ICON_DOT, (key_data ? 1 : 0));  // key feedback
			}
		}
		et16315_data->key_prev = key_data;
		msleep(et16315_data->keys_poll);
	}
	polling = 2;
}

static DECLARE_WORK(keyboard_obj, keyboard_poll);

static int keyboard_input_open(struct input_dev *dev)
{
	keyboard_workqueue = create_workqueue("keyboard");
	if (queue_work(keyboard_workqueue, &keyboard_obj))
	{
		dprintk(5, "[KBD] Work queue created\n");
		return 0;
	}
	dprintk(1, "[KBD] Create work queue failed\n");
	return 1;
}

static void keyboard_input_close(struct input_dev *dev)
{
	polling = 0;
	while (polling != 2)
	{
		msleep(1);
	}
	polling = 1;

	if (keyboard_workqueue)
	{
		destroy_workqueue(keyboard_workqueue);
		dprintk(5, "[KBD] Workqueue destroyed.\n");
	}
}

int keyboard_dev_init(struct et16315_chip *chip)
{
	int err = -1;
	u32 key_data;
	struct et16315_platform_data *et16315_init = &et16315_init_values;

	dprintk(5, "[KBD] Allocating and registering ET16315 keyboard device\n");

	chip->keyboard = input_allocate_device();

	if (!chip->keyboard)
	{
		dprintk(1, "Could not allocate memory (ET16315 keyboard device)\n");
		return -ENOMEM;
	}

	chip->keyboard->name  = et16315_init->driver_name;
	chip->keyboard->open  = keyboard_input_open;
	chip->keyboard->close = keyboard_input_close;

	set_bit(EV_KEY,    chip->keyboard->evbit);
	set_bit(KEY_UP,    chip->keyboard->keybit);
	set_bit(KEY_DOWN,  chip->keyboard->keybit);
	set_bit(KEY_LEFT,  chip->keyboard->keybit);
	set_bit(KEY_RIGHT, chip->keyboard->keybit);
//	set_bit(KEY_POWER, chip->keyboard->keybit);  // Power key is connected directly to PIO 3.1

	err = input_register_device(chip->keyboard);
	if (err)
	{
		input_free_device(chip->keyboard);
		dprintk(1, "Could not register ET16315 keyboard device\n");
		return err;
	}
	dprintk(5, "[KBD] ET16315 keyboard device registered\n");
	chip->keys_poll = et16315_init->keys_poll;

	/* Initialize keyboard table */
	chip->keys_num = et16315_init->keys_num;
	chip->keys = et16315_init->keys;
	/* Get 1st key data */
	et16315_recv(chip, ET16315_CMD2_DATA_SET(0, ET16315_FIXED_ADDR, ET16315_CMD_READ_KEY), &key_data, 4);
	chip->key_prev = key_data;
	return 0;
}

void keyboard_dev_exit(void)
{
	dprintk(5, "[KBD] Unregistering keyboard device.\n");
	input_unregister_device(et16315_data->keyboard);
}
/****************************************************************************************
 *
 * Initialize ET16315 front panel interface and LEDs
 *
 ****************************************************************************************/
int __init fortis_4g_if_init(void)
{
	int err;
	struct et16315_platform_data *et16315_init = &et16315_init_values;

	/* Initialize the GPIO interface to the ET16315 */
	et16315_data = kzalloc(sizeof(*et16315_data), GFP_KERNEL);
	if (!et16315_data)
	{
		dprintk(1, "kzalloc failed\n");
		return -ENOMEM;
	}

	spin_lock_init(&et16315_data->lock);
//	platform_set_drvdata(pdev, et16315_data);

	/* Allocate GPIO pins */
	gpio_free(et16315_data->gpio_stb);
	gpio_free(et16315_data->gpio_clk);
	gpio_free(et16315_data->gpio_dout);
	gpio_free(et16315_data->gpio_din);
	et16315_data->gpio_din = et16315_init->gpio_din;
	err = gpio_request(et16315_data->gpio_din, "et16315_Din");
	if (err)
	{
		kfree(et16315_data);
		return err;
	}
	gpio_direction_output(et16315_data->gpio_din, 0);

	et16315_data->gpio_dout = et16315_init->gpio_dout;
	err = gpio_request(et16315_data->gpio_dout, "et16315_Dout");
	if (err)
	{
		gpio_free(et16315_data->gpio_din);
		kfree(et16315_data);
		return err;
	}
	gpio_direction_input(et16315_data->gpio_dout);

	et16315_data->gpio_clk = et16315_init->gpio_clk;
	err = gpio_request(et16315_data->gpio_clk, "et16315_clk");
	if (err)
	{
		gpio_free(et16315_data->gpio_dout);
		gpio_free(et16315_data->gpio_din);
		kfree(et16315_data);
		return err;
	}
	gpio_direction_output(et16315_data->gpio_clk, 1);

	et16315_data->gpio_stb = et16315_init->gpio_stb;
	err = gpio_request(et16315_data->gpio_stb, "et16315_stb");
	if (err)
	{
		gpio_free(et16315_data->gpio_clk);
		gpio_free(et16315_data->gpio_dout);
		gpio_free(et16315_data->gpio_din);
		kfree(et16315_data);
		return err;
	}
	gpio_direction_output(et16315_data->gpio_stb, 1);  // set strobe high
	dprintk(20, "GPIOs for ET16315 allocated\n");

	/* Initialize the ET16315 */
	et16315_clear(et16315_data);  /* Clear display memory */

	/* Initialize character table */
	et16315_data->char_tbl = et16315_init->char_tbl;

	/* Set display mode and initial brightness */
	et16315_data->digits = et16315_init->digits;
	if (et16315_data->digits > ET16315_DISPLAY_MAX_DIGITS)
	{
		et16315_data->digits = ET16315_DISPLAY_MAX_DIGITS;
	}
	et16315_send(et16315_data, ET16315_CMD1_SET_DISPLAY_MODE(et16315_data->digits - et16315_config_4_digits_24_segments), NULL, 0);

	// enable display
	et16315_set_light(et16315_data, et16315_init->on);

	// set initial brightness
	fortis_4gSetBrightness(et16315_init->brightness);
	dprintk(10, "Display initialized, width = %d, brightness = %d\n", et16315_data->digits, et16315_data->brightness);

	/* Show initial text */
	et16315_set_text(et16315_data, et16315_init->text);
//	fortis_4gWriteString((unsigned char*)et16315_init_data->text);
//	dprintk(10, "Show initial text [%s]\n", et16315_init->text);
	msleep(1000);

	/* Initialize and set the LEDs and icons */
	fortis_4gSetIcon(ICON_DOT, 1);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON1, 1);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON2, 1);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON3, 1);
	msleep(100);

//#if !defined(FOREVER_9898HD)
	init_leds(et16315_data);
	dprintk(10, "LEDs initialized\n");
//#endif
	fortis_4gSetIcon(ICON_DOT, 0);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON1, 0);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON2, 0);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON3, 0);
	msleep(100);
	dprintk(10, "Icons initialized\n");

	/* Initialize front panel keyboard interface */
	err = keyboard_dev_init(et16315_data);
	if (err)
	{
		dprintk(1, "Could not initialize ET16315 keyboard interface\n");
		fortis_4gExit_core();
		return err;
	}
	dprintk(10, "ET16315 keyboard initialized, %d keys, poll period is %d ms\n", et16315_data->keys_num, et16315_data->keys_poll);

	dprintk(100, "%s <\n", __func__);
	return 0;
}

void cleanup_et16315(void)
{
	gpio_free(et16315_data->gpio_stb);
	gpio_free(et16315_data->gpio_clk);
	gpio_free(et16315_data->gpio_dout);
	gpio_free(et16315_data->gpio_din);
	kfree(et16315_data);
}

/****************************************************************************************
 *
 * ET16315 IOCTL routines
 *
 ****************************************************************************************/
/* Function SetIcon */
int fortis_4gSetIcon(int which, int on)
{
	int ret = 0;
	int digit_offset;

	dprintk(100, "%s > which = %d, on = %d\n", __func__, which, on);

	switch (which)
	{
		case ICON_DOT:
		{
			digit_offset = 7 * 3 + 1;
			break;
		}
		case ICON_COLON1:
		{
			digit_offset = 6 * 3 + 1;
			break;
		}
		case ICON_COLON2:
		{
			digit_offset = 4 * 3 + 1;
			break;
		}
		case ICON_COLON3:
		{
			digit_offset = 2 * 3 + 1;
			break;
		}
		default:
		{
			dprintk(1, "Icon number %d out of range (1-%d)\n", which, ICON_MAX - 1);
			return -EINVAL;
		}
	}

	if (on < 1)
	{
		et16315_data->last_display[digit_offset] &= 0b01111111;
	}
	else
	{
		et16315_data->last_display[digit_offset] |= 0b10000000;
	}
	et16315_send(et16315_data, ET16315_CMD3_SET_ADDRESS(0), et16315_data->last_display, (et16315_data->digits * 3));

	dprintk(100, "%s < RTV = %d\n", __func__, ret);
	return ret;
}
/* export for later use in e2_proc */
EXPORT_SYMBOL(fortis_4gSetIcon);

/* Function SetLED (and SetPwrLED) */
int fortis_4gSetLED(int which, int level)
{
	int ret = 0;
	u8 led;

	dprintk(10, "%s > which = %d, level = %d\n", __func__, which, level);

	/* Caution: LED# is a bitmask with Fortis receivers (nuvoton compatibility) */

	if (which < 0 || which > MAX_LED)
	{
		dprintk(1, "LED number %d out of range (1-%d)\n", which, MAX_LED);
		return -EINVAL;
	}

	level = (level != 0 ? 1 : 0);  // normalize level to 0 or 1

#if defined(FOREVER_9898HD)
//	if (which & LED_RED)
//	{
//		level = (level == 0 ? 1 : 0);
//		gpio_set_value(GPIO_LED_RED, level);
//	}
#else
	/* Red and blue LEDs are connected to the ET16315 VFD driver
	 * Logo LEDs are connected to a GPIO pin via a driver transistor,
	 * both the ET16315 and the transistor invert the drive, so 1 = on.
	 */
	// Handle ET16315 connected LEDs
 #if defined(FOREVER_3434HD)
	if ((which & LED_RED) || (which & LED_BLUE))
 #else
	if (which & LED_RED)
 #endif
	{
		led = et16315_data->led;  // get current values

		if (level)
		{
			led |= which;  // add led to bit mask for ET16315
		}
		else
		{
			led &= ~which;  // remove led from bit mask
		}
		et16315_data->led = led & 0x03;
		dprintk(10, "Setting LED pattern 0x%02x\n", led);
		et16315_set_led(et16315_data, led);
	}
//	else
//	{
//	}
#endif // FOREVER_9898HD
	// Handle GPIO connected LEDs
#if defined(FOREVER_3434HD) \
 || defined(GPV8000) \
 || defined(EP8000) \
 || defined(EPP8000)
	if (which & LED_LOGO)
	{
		/* Note: drive of LED_LOGO is inverted on FOREVER_3434HD/GPV8000/EP8000/EPP8000 */
		level = (level == 0 ? 1 : 0);
		gpio_set_value(GPIO_LED_LOGO, level);
	}
#endif
	dprintk(100, "%s < RTV = %d\n", __func__, ret);
	return ret;
}
/* export for later use in e2_proc */
EXPORT_SYMBOL(fortis_4gSetLED);

/* Function SetBrightness */
int fortis_4gSetBrightness(int level)
{
	int ret = 0;

	dprintk(100, "%s > level = %d\n", __func__, level);

	if (level < 0 || level > MAX_BRIGHT)
	{
		dprintk(1,"Display brightness level %d out of range (0-%d)\n", level, MAX_BRIGHT);
		return -EINVAL;
	}
	et16315_data->brightness = level;
	et16315_set_brightness(et16315_data, level);

	dprintk(100, "%s < RTV = %d\n", __func__, ret);
	return ret;
}
/* export for later use in e2_proc */
EXPORT_SYMBOL(fortis_4gSetBrightness);

/* Function SetDisplayOnOff */
int fortis_4gSetDisplayOnOff(int on)
{
	dprintk(100, "%s > on = %d\n", __func__, on);

	et16315_data->on = on;
	et16315_set_light(et16315_data, et16315_data->on);
	dprintk(100, "%s <\n", __func__);
	return 0;
}

/* Function GetWakeUpMode (to be built) */
int fortis_4gGetWakeUpMode(void)
{
//	char buffer[3];
	int ret = -1;

	dprintk(100, "%s >\n", __func__);

//	memset(buffer, 0, 3);

//	buffer[0] = SOP;
//	buffer[1] = cCommandGetPowerOnSrc;
//	buffer[2] = EOP;

//	errorOccured = 0;
//	ret = fortis_4gWriteCommand(buffer, 3, 1);

//	if (errorOccured == 1)
//	{
//		/* error */
////		memset(ioctl_data, 0, 8);
//		dprintk(1, "GetWakeMode: error\n");
//		ret = -ETIMEDOUT;
//	}
//	else
//	{
//		/* wake up reason mode received ->noop here */
//	}

	dprintk(100, "%s <\n", __func__);
	return ret;
}

/* Function SetTimeFormat (to be built, if needed) */
int fortis_4gSetTimeFormat(char format)
{
	int  ret = -1;

	dprintk(100, "%s >\n", __func__);

	dprintk(1, "SetTimeFormat not implemented yet\n");

	dprintk(100, "%s <\n", __func__);
	return ret;
}

/* Function SetTimeDisplay (to be built, if needed) */
int fortis_4gSetTimeDisplayOnOff(char onoff)
{
	int  ret = -1;

	dprintk(100, "%s >\n", __func__);

	dprintk(1, "SetTimeDisplayOnOff not implemented yet\n");

	dprintk(100, "%s <\n", __func__);
	return ret;
}

/* Function WriteString */
#if defined(FOREVER_3434HD) \
 || defined(GPV8000) \
 || defined(EP8000) \
 || defined(EPP8000) \
 || defined(FOREVER_9898HD)
int fortis_4gWriteString(unsigned char *aBuf, int len)
{
	int i, j;
	int ret = -1;
	unsigned char *UTF_Char_Table;
	unsigned char bBuf[64];  // resulting display string after UTF 8 processing

	dprintk(100, "%s > %d\n", __func__, len);

	/* Handle UTF8 stuff */
	i = 0;  // input string index
	j = 0;  // output string index

#if defined(VFD_SCROLL)  /* Scroll normally handled in display thread */
	while ((i < len) && (j < 63))
#else
	while ((i < len) && (j < DISP_SIZE))
#endif
	{
		if (aBuf[i] < 0x80)  // if normal ASCII
		{
			bBuf[j] = aBuf[i];  // copy to output string
		}
		else if (aBuf[i] < 0xe0)
		{
			switch (aBuf[i])
			{
				case 0xc2:
				{
					UTF_Char_Table = UTF_C2;
					break;
				}
				case 0xc3:
				{
					UTF_Char_Table = UTF_C3;
					break;
				}
				case 0xc4:
				{
					UTF_Char_Table = UTF_C4;
					break;
				}
				case 0xc5:
				{
					UTF_Char_Table = UTF_C5;
					break;
				}
				case 0xd0:
				{
					UTF_Char_Table = UTF_D0;  // cyrillic I
					break;
				}
				case 0xd1:
				{
					UTF_Char_Table = UTF_D1;  // cyrillic II
					break;
				}
				default:
				{
					UTF_Char_Table = NULL;
				}
			}
			i++;  // set index to next byte
			if (UTF_Char_Table)
			{
				if (UTF_Char_Table[aBuf[i] & 0x3f] != 0x00)  // if printable
				{
					bBuf[j] = UTF_Char_Table[aBuf[i] & 0x3f];  // get replacement character from table
				}
				else
				{
					j--;  //else skip character
				}
			}
			else
			{
#if 0
				sprintf(&bBuf[j], "%02x", aBuf[i - 1]);  // show hex value;
				j += 2;  // advance output index
				bBuf[j] = (aBuf[i] & 0x3f) | 0x40;
#else
				bBuf[j] = 0x20;  // display a space
#endif
			}
		}
		else
		{
			if (aBuf[i] < 0xF0)
			{
				i += 2;  // skip 2 input bytes
			}
			else if (aBuf[i] < 0xF8)
			{
				i += 3;
			}
			else if (aBuf[i] < 0xFC)
			{
				i += 4;
			}
			else
			{
				i += 5;
			}
			bBuf[j] = 0x20;  // and put a space
		}
		i++;
		j++;
	}
	bBuf[j] = 0;  // terminate result string
	dprintk(100, "Display [%s] (len = %d)\n", bBuf, j);

#if defined(VFD_SCROLL)  /* Scroll normally handled in display thread */
	if (len > et16315_data->digits)
	{
		if (len > 63)
		{
			len = 63;
		}
		et16315_set_text(et16315_data + 2, bBuf);  // initial display

		msleep(300);
		for (i = 1; i <= len; i++)
		{
			memset(aBuf, ' ', et16315_data->digits);  // fill buffer with spaces
			memcpy(aBuf, bBuf + i, len);
			et16315_set_text(et16315_data, aBuf);
			msleep(300);
		}
	}
#endif
	et16315_set_text(et16315_data, bBuf);
	ret = 0;

	/* save last string written to fp */
	memcpy(&lastdata.data, bBuf, 64);
	lastdata.length = len;

	dprintk(100, "%s <\n", __func__);
	return ret;
}
#else  // not FOREVER_3434HD, GPV8000, EP8000, EPP8000 or FOREVER_9898HD
int fortis_4gWriteString(unsigned char* aBuf, int len)
{
	dprintk(100, "%s >\n", __func__);
	dprintk(100, "%s <\n", __func__);
	return -EFAULT;
}
#endif

int __init fortis_4gInit_core(void)
{
	int res;

	dprintk(100, "%s >\n", __func__);

	res = fortis_4g_if_init();

	dprintk(100, "%s < RTV = %d\n", __func__, res);
	return res;
}

void fortis_4gExit_core(void)
{
	dprintk(100, "%s >\n", __func__);

	dprintk(5, "[KBD] Unloading...\n");
	keyboard_dev_exit();

	/* free logo LED GPIO pin */
	cleanup_leds();	

	cleanup_et16315();

	dprintk(100, "%s <\n", __func__);
}
// vim:ts=4
