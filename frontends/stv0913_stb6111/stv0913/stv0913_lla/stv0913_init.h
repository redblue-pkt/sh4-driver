/*****************************************************************************/
/* COPYRIGHT (C) 2009 STMicroelectronics - All Rights Reserved               */
/* ST makes no warranty express or implied including but not limited to,     */
/* any warranty of                                                           */
/*                                                                           */
/*   (i)  merchantability or fitness for a particular purpose and/or         */
/*   (ii) requirements, for a particular purpose in relation to the LICENSED */
/*        MATERIALS, which is provided 'AS IS', WITH ALL FAULTS. ST does not */
/*        represent or warrant that the LICENSED MATERIALS provided here     */
/*        under is free of infringement of any third party patents,          */
/*        copyrights,trade secrets or other intellectual property rights.    */
/*        ALL WARRANTIES, CONDITIONS OR OTHER TERMS IMPLIED BY LAW ARE       */
/*        EXCLUDED TO THE FULLEST EXTENT PERMITTED BY LAW                    */
/*                                                                           */
/*****************************************************************************/
/**
@File   STV0913_init.h                                             
@brief                                                                         



*/
#ifndef H_STV0913INIT
#define H_STV0913INIT
	
#include "chip.h"
#include "gen_types.h"

	/* =======================================================================
	-- Registers Declaration
	-- -------------------------
	-- Each register (RSTV0913_XXXXX) is defined by its address (2 bytes).	 						    
	--
	-- Each field (FSTV0913_XXXXX)is defined as follow: 
	-- [register address -- 2bytes][field sign -- 1byte][field mask -- 1byte]
	======================================================================= */

/*MID*/
#define RSTV0913_MID                                        0xf100
#define FSTV0913_MCHIP_IDENT                                        0xf10000f0
#define FSTV0913_MRELEASE                                        0xf100000f

/*DID*/
#define RSTV0913_DID                                        0xf101
#define FSTV0913_DEVICE_ID                                        0xf10100ff

/*DACR1*/
#define RSTV0913_DACR1                                        0xf113
#define FSTV0913_DAC_MODE                                        0xf11300e0
#define FSTV0913_DAC_VALUE1                                        0xf113000f

/*DACR2*/
#define RSTV0913_DACR2                                        0xf114
#define FSTV0913_DAC_VALUE0                                        0xf11400ff

/*PADCFG*/
#define RSTV0913_PADCFG                                        0xf11a
#define FSTV0913_AGCRF1_OPD                                        0xf11a0002
#define FSTV0913_AGCRF1_XOR                                        0xf11a0001

/*OUTCFG2*/
#define RSTV0913_OUTCFG2                                        0xf11b
#define FSTV0913_TS1_ERROR_XOR                                        0xf11b0008
#define FSTV0913_TS1_DPN_XOR                                        0xf11b0004
#define FSTV0913_TS1_STROUT_XOR                                        0xf11b0002
#define FSTV0913_TS1_CLOCKOUT_XOR                                        0xf11b0001

/*OUTCFG*/
#define RSTV0913_OUTCFG                                        0xf11c
#define FSTV0913_TS1_OUTSER_HZ                                        0xf11c0010
#define FSTV0913_TS1_OUTPAR_HZ                                        0xf11c0004
#define FSTV0913_TS_SERDATA0                                        0xf11c0002

/*IRQSTATUS3*/
#define RSTV0913_IRQSTATUS3                                        0xf120
#define FSTV0913_SPLL_LOCK                                        0xf1200020
#define FSTV0913_SSTREAM_LCK_1                                        0xf1200010
#define FSTV0913_SDVBS1_PRF_1                                        0xf1200001

/*IRQSTATUS2*/
#define RSTV0913_IRQSTATUS2                                        0xf121
#define FSTV0913_SSPY_ENDSIM_1                                        0xf1210080
#define FSTV0913_SPKTDEL_ERROR_1                                        0xf1210002
#define FSTV0913_SPKTDEL_LOCKB_1                                        0xf1210001

/*IRQSTATUS1*/
#define RSTV0913_IRQSTATUS1                                        0xf122
#define FSTV0913_SPKTDEL_LOCK_1                                        0xf1220080
#define FSTV0913_SFEC_LOCKB_1                                        0xf1220010
#define FSTV0913_SFEC_LOCK_1                                        0xf1220008

/*IRQSTATUS0*/
#define RSTV0913_IRQSTATUS0                                        0xf123
#define FSTV0913_SDEMOD_LOCKB_1                                        0xf1230080
#define FSTV0913_SDEMOD_LOCK_1                                        0xf1230040
#define FSTV0913_SDEMOD_IRQ_1                                        0xf1230020
#define FSTV0913_SBCH_ERRFLAG                                        0xf1230010
#define FSTV0913_SDISEQC1_IRQ                                        0xf1230001

/*IRQMASK3*/
#define RSTV0913_IRQMASK3                                        0xf124
#define FSTV0913_MPLL_LOCK                                        0xf1240020
#define FSTV0913_MSTREAM_LCK_1                                        0xf1240010
#define FSTV0913_MDVBS1_PRF_1                                        0xf1240001

/*IRQMASK2*/
#define RSTV0913_IRQMASK2                                        0xf125
#define FSTV0913_MSPY_ENDSIM_1                                        0xf1250080
#define FSTV0913_MPKTDEL_ERROR_1                                        0xf1250002
#define FSTV0913_MPKTDEL_LOCKB_1                                        0xf1250001

/*IRQMASK1*/
#define RSTV0913_IRQMASK1                                        0xf126
#define FSTV0913_MPKTDEL_LOCK_1                                        0xf1260080
#define FSTV0913_MFEC_LOCKB_1                                        0xf1260010
#define FSTV0913_MFEC_LOCK_1                                        0xf1260008

/*IRQMASK0*/
#define RSTV0913_IRQMASK0                                        0xf127
#define FSTV0913_MDEMOD_LOCKB_1                                        0xf1270080
#define FSTV0913_MDEMOD_LOCK_1                                        0xf1270040
#define FSTV0913_MDEMOD_IRQ_1                                        0xf1270020
#define FSTV0913_MBCH_ERRFLAG                                        0xf1270010
#define FSTV0913_MDISEQC1_IRQ                                        0xf1270001

/*I2CCFG*/
#define RSTV0913_I2CCFG                                        0xf129
#define FSTV0913_I2C_FASTMODE                                        0xf1290008
#define FSTV0913_I2CADDR_INC                                        0xf1290003

/*I2CRPT*/
#define RSTV0913_I2CRPT                                        0xf12a
#define FSTV0913_I2CT_ON                                        0xf12a0080
#define FSTV0913_ENARPT_LEVEL                                        0xf12a0070
#define FSTV0913_SCLT_DELAY                                        0xf12a0008
#define FSTV0913_STOP_ENABLE                                        0xf12a0004
#define FSTV0913_STOP_SDAT2SDA                                        0xf12a0002

/*GPIO0CFG*/
#define RSTV0913_GPIO0CFG                                        0xf140
#define FSTV0913_GPIO0_OPD                                        0xf1400080
#define FSTV0913_GPIO0_CONFIG                                        0xf140007e
#define FSTV0913_GPIO0_XOR                                        0xf1400001

/*GPIO1CFG*/
#define RSTV0913_GPIO1CFG                                        0xf141
#define FSTV0913_GPIO1_OPD                                        0xf1410080
#define FSTV0913_GPIO1_CONFIG                                        0xf141007e
#define FSTV0913_GPIO1_XOR                                        0xf1410001

/*GPIO2CFG*/
#define RSTV0913_GPIO2CFG                                        0xf142
#define FSTV0913_GPIO2_OPD                                        0xf1420080
#define FSTV0913_GPIO2_CONFIG                                        0xf142007e
#define FSTV0913_GPIO2_XOR                                        0xf1420001

/*GPIO3CFG*/
#define RSTV0913_GPIO3CFG                                        0xf143
#define FSTV0913_GPIO3_OPD                                        0xf1430080
#define FSTV0913_GPIO3_CONFIG                                        0xf143007e
#define FSTV0913_GPIO3_XOR                                        0xf1430001

/*GPIO4CFG*/
#define RSTV0913_GPIO4CFG                                        0xf144
#define FSTV0913_GPIO4_OPD                                        0xf1440080
#define FSTV0913_GPIO4_CONFIG                                        0xf144007e
#define FSTV0913_GPIO4_XOR                                        0xf1440001

/*GPIO5CFG*/
#define RSTV0913_GPIO5CFG                                        0xf145
#define FSTV0913_GPIO5_OPD                                        0xf1450080
#define FSTV0913_GPIO5_CONFIG                                        0xf145007e
#define FSTV0913_GPIO5_XOR                                        0xf1450001

/*GPIO6CFG*/
#define RSTV0913_GPIO6CFG                                        0xf146
#define FSTV0913_GPIO6_OPD                                        0xf1460080
#define FSTV0913_GPIO6_CONFIG                                        0xf146007e
#define FSTV0913_GPIO6_XOR                                        0xf1460001

/*GPIO7CFG*/
#define RSTV0913_GPIO7CFG                                        0xf147
#define FSTV0913_GPIO7_OPD                                        0xf1470080
#define FSTV0913_GPIO7_CONFIG                                        0xf147007e
#define FSTV0913_GPIO7_XOR                                        0xf1470001

/*GPIO8CFG*/
#define RSTV0913_GPIO8CFG                                        0xf148
#define FSTV0913_GPIO8_OPD                                        0xf1480080
#define FSTV0913_GPIO8_CONFIG                                        0xf148007e
#define FSTV0913_GPIO8_XOR                                        0xf1480001

/*GPIO9CFG*/
#define RSTV0913_GPIO9CFG                                        0xf149
#define FSTV0913_GPIO9_OPD                                        0xf1490080
#define FSTV0913_GPIO9_CONFIG                                        0xf149007e
#define FSTV0913_GPIO9_XOR                                        0xf1490001

/*GPIO10CFG*/
#define RSTV0913_GPIO10CFG                                        0xf14a
#define FSTV0913_GPIO10_OPD                                        0xf14a0080
#define FSTV0913_GPIO10_CONFIG                                        0xf14a007e
#define FSTV0913_GPIO10_XOR                                        0xf14a0001

/*GPIO11CFG*/
#define RSTV0913_GPIO11CFG                                        0xf14b
#define FSTV0913_GPIO11_OPD                                        0xf14b0080
#define FSTV0913_GPIO11_CONFIG                                        0xf14b007e
#define FSTV0913_GPIO11_XOR                                        0xf14b0001

/*GPIO12CFG*/
#define RSTV0913_GPIO12CFG                                        0xf14c
#define FSTV0913_GPIO12_OPD                                        0xf14c0080
#define FSTV0913_GPIO12_CONFIG                                        0xf14c007e
#define FSTV0913_GPIO12_XOR                                        0xf14c0001

/*STRSTATUS1*/
#define RSTV0913_STRSTATUS1                                        0xf16a
#define FSTV0913_STRSTATUS_SEL2                                        0xf16a00f0
#define FSTV0913_STRSTATUS_SEL1                                        0xf16a000f

/*STRSTATUS2*/
#define RSTV0913_STRSTATUS2                                        0xf16b
#define FSTV0913_STRSTATUS_SEL4                                        0xf16b00f0
#define FSTV0913_STRSTATUS_SEL3                                        0xf16b000f

/*STRSTATUS3*/
#define RSTV0913_STRSTATUS3                                        0xf16c
#define FSTV0913_STRSTATUS_SEL6                                        0xf16c00f0
#define FSTV0913_STRSTATUS_SEL5                                        0xf16c000f

/*FSKTFC2*/
#define RSTV0913_FSKTFC2                                        0xf170
#define FSTV0913_FSKT_KMOD                                        0xf17000fc
#define FSTV0913_FSKT_CAR2                                        0xf1700003

/*FSKTFC1*/
#define RSTV0913_FSKTFC1                                        0xf171
#define FSTV0913_FSKT_CAR1                                        0xf17100ff

/*FSKTFC0*/
#define RSTV0913_FSKTFC0                                        0xf172
#define FSTV0913_FSKT_CAR0                                        0xf17200ff

/*FSKTDELTAF1*/
#define RSTV0913_FSKTDELTAF1                                        0xf173
#define FSTV0913_FSKT_DELTAF1                                        0xf173000f

/*FSKTDELTAF0*/
#define RSTV0913_FSKTDELTAF0                                        0xf174
#define FSTV0913_FSKT_DELTAF0                                        0xf17400ff

/*FSKTCTRL*/
#define RSTV0913_FSKTCTRL                                        0xf175
#define FSTV0913_FSKT_PINSEL                                        0xf1750080
#define FSTV0913_FSKT_EN_SGN                                        0xf1750040
#define FSTV0913_FSKT_MOD_SGN                                        0xf1750020
#define FSTV0913_FSKT_MOD_EN                                        0xf175001c
#define FSTV0913_FSKT_DACMODE                                        0xf1750003

/*FSKRFC2*/
#define RSTV0913_FSKRFC2                                        0xf176
#define FSTV0913_FSKR_DETSGN                                        0xf1760040
#define FSTV0913_FSKR_OUTSGN                                        0xf1760020
#define FSTV0913_FSKR_KAGC                                        0xf176001c
#define FSTV0913_FSKR_CAR2                                        0xf1760003

/*FSKRFC1*/
#define RSTV0913_FSKRFC1                                        0xf177
#define FSTV0913_FSKR_CAR1                                        0xf17700ff

/*FSKRFC0*/
#define RSTV0913_FSKRFC0                                        0xf178
#define FSTV0913_FSKR_CAR0                                        0xf17800ff

/*FSKRK1*/
#define RSTV0913_FSKRK1                                        0xf179
#define FSTV0913_FSKR_K1_EXP                                        0xf17900e0
#define FSTV0913_FSKR_K1_MANT                                        0xf179001f

/*FSKRK2*/
#define RSTV0913_FSKRK2                                        0xf17a
#define FSTV0913_FSKR_K2_EXP                                        0xf17a00e0
#define FSTV0913_FSKR_K2_MANT                                        0xf17a001f

/*FSKRAGCR*/
#define RSTV0913_FSKRAGCR                                        0xf17b
#define FSTV0913_FSKR_OUTCTL                                        0xf17b00c0
#define FSTV0913_FSKR_AGC_REF                                        0xf17b003f

/*FSKRAGC*/
#define RSTV0913_FSKRAGC                                        0xf17c
#define FSTV0913_FSKR_AGC_ACCU                                        0xf17c00ff

/*FSKRALPHA*/
#define RSTV0913_FSKRALPHA                                        0xf17d
#define FSTV0913_FSKR_ALPHA_EXP                                        0xf17d001c
#define FSTV0913_FSKR_ALPHA_M                                        0xf17d0003

/*FSKRPLTH1*/
#define RSTV0913_FSKRPLTH1                                        0xf17e
#define FSTV0913_FSKR_BETA                                        0xf17e00f0
#define FSTV0913_FSKR_PLL_TRESH1                                        0xf17e000f

/*FSKRPLTH0*/
#define RSTV0913_FSKRPLTH0                                        0xf17f
#define FSTV0913_FSKR_PLL_TRESH0                                        0xf17f00ff

/*FSKRDF1*/
#define RSTV0913_FSKRDF1                                        0xf180
#define FSTV0913_FSKR_OUT                                        0xf1800080
#define FSTV0913_FSKR_STATE                                        0xf1800060
#define FSTV0913_FSKR_DELTAF1                                        0xf180001f

/*FSKRDF0*/
#define RSTV0913_FSKRDF0                                        0xf181
#define FSTV0913_FSKR_DELTAF0                                        0xf18100ff

/*FSKRSTEPP*/
#define RSTV0913_FSKRSTEPP                                        0xf182
#define FSTV0913_FSKR_STEP_PLUS                                        0xf18200ff

/*FSKRSTEPM*/
#define RSTV0913_FSKRSTEPM                                        0xf183
#define FSTV0913_FSKR_STEP_MINUS                                        0xf18300ff

/*FSKRDET1*/
#define RSTV0913_FSKRDET1                                        0xf184
#define FSTV0913_FSKR_DETECT                                        0xf1840080
#define FSTV0913_FSKR_CARDET_ACCU1                                        0xf184000f

/*FSKRDET0*/
#define RSTV0913_FSKRDET0                                        0xf185
#define FSTV0913_FSKR_CARDET_ACCU0                                        0xf18500ff

/*FSKRDTH1*/
#define RSTV0913_FSKRDTH1                                        0xf186
#define FSTV0913_FSKR_CARLOSS_THRESH1                                        0xf18600f0
#define FSTV0913_FSKR_CARDET_THRESH1                                        0xf186000f

/*FSKRDTH0*/
#define RSTV0913_FSKRDTH0                                        0xf187
#define FSTV0913_FSKR_CARDET_THRESH0                                        0xf18700ff

/*FSKRLOSS*/
#define RSTV0913_FSKRLOSS                                        0xf188
#define FSTV0913_FSKR_CARLOSS_THRESH0                                        0xf18800ff

/*NCOARSE*/
#define RSTV0913_NCOARSE                                        0xf1b3
#define FSTV0913_CP                                        0xf1b300f8
#define FSTV0913_IDF                                        0xf1b30007

/*NCOARSE1*/
#define RSTV0913_NCOARSE1                                        0xf1b4
#define FSTV0913_N_DIV                                        0xf1b400ff

/*NCOARSE2*/
#define RSTV0913_NCOARSE2                                        0xf1b5
#define FSTV0913_ODF                                        0xf1b5003f

/*SYNTCTRL*/
#define RSTV0913_SYNTCTRL                                        0xf1b6
#define FSTV0913_STANDBY                                        0xf1b60080
#define FSTV0913_BYPASSPLLCORE                                        0xf1b60040
#define FSTV0913_STOP_PLL                                        0xf1b60008
#define FSTV0913_OSCI_E                                        0xf1b60002

/*FILTCTRL*/
#define RSTV0913_FILTCTRL                                        0xf1b7
#define FSTV0913_INV_CLKFSK                                        0xf1b70002
#define FSTV0913_BYPASS_APPLI                                        0xf1b70001

/*PLLSTAT*/
#define RSTV0913_PLLSTAT                                        0xf1b8
#define FSTV0913_PLLLOCK                                        0xf1b80001

/*STOPCLK1*/
#define RSTV0913_STOPCLK1                                        0xf1c2
#define FSTV0913_INV_CLKADCI1                                        0xf1c20001

/*STOPCLK2*/
#define RSTV0913_STOPCLK2                                        0xf1c3
#define FSTV0913_STOP_DVBS2FEC                                        0xf1c30010
#define FSTV0913_STOP_DVBS1FEC                                        0xf1c30004
#define FSTV0913_STOP_DEMOD                                        0xf1c30001

/*PREGCTL*/
#define RSTV0913_PREGCTL                                        0xf1c8
#define FSTV0913_REG3V3TO2V5_POFF                                        0xf1c80080

/*TSTTNR0*/
#define RSTV0913_TSTTNR0                                        0xf1df
#define FSTV0913_FSK_PON                                        0xf1df0004

/*TSTTNR1*/
#define RSTV0913_TSTTNR1                                        0xf1e0
#define FSTV0913_ADC1_PON                                        0xf1e00002

/*TSTTNR2*/
#define RSTV0913_TSTTNR2                                        0xf1e1
#define FSTV0913_I2C_DISEQC_PON                                        0xf1e10020
#define FSTV0913_DISEQC_CLKDIV                                        0xf1e1000f

/*IQCONST*/
#define RSTV0913_IQCONST                                        0xf400
#define FSTV0913_CONSTEL_SELECT                                        0xf4000060
#define FSTV0913_IQSYMB_SEL                                        0xf400001f

/*NOSCFG*/
#define RSTV0913_NOSCFG                                        0xf401
#define FSTV0913_DUMMYPL_NOSDATA                                        0xf4010020
#define FSTV0913_NOSPLH_BETA                                        0xf4010018
#define FSTV0913_NOSDATA_BETA                                        0xf4010007

/*ISYMB*/
#define RSTV0913_ISYMB                                        0xf402
#define FSTV0913_I_SYMBOL                                        0xf40201ff

/*QSYMB*/
#define RSTV0913_QSYMB                                        0xf403
#define FSTV0913_Q_SYMBOL                                        0xf40301ff

/*AGC1CFG*/
#define RSTV0913_AGC1CFG                                        0xf404
#define FSTV0913_DC_FROZEN                                        0xf4040080
#define FSTV0913_DC_CORRECT                                        0xf4040040
#define FSTV0913_AMM_FROZEN                                        0xf4040020
#define FSTV0913_AMM_CORRECT                                        0xf4040010
#define FSTV0913_QUAD_FROZEN                                        0xf4040008
#define FSTV0913_QUAD_CORRECT                                        0xf4040004

/*AGC1CN*/
#define RSTV0913_AGC1CN                                        0xf406
#define FSTV0913_AGC1_LOCKED                                        0xf4060080
#define FSTV0913_AGC1_MINPOWER                                        0xf4060010
#define FSTV0913_AGCOUT_FAST                                        0xf4060008
#define FSTV0913_AGCIQ_BETA                                        0xf4060007

/*AGC1REF*/
#define RSTV0913_AGC1REF                                        0xf407
#define FSTV0913_AGCIQ_REF                                        0xf40700ff

/*IDCCOMP*/
#define RSTV0913_IDCCOMP                                        0xf408
#define FSTV0913_IAVERAGE_ADJ                                        0xf40801ff

/*QDCCOMP*/
#define RSTV0913_QDCCOMP                                        0xf409
#define FSTV0913_QAVERAGE_ADJ                                        0xf40901ff

/*POWERI*/
#define RSTV0913_POWERI                                        0xf40a
#define FSTV0913_POWER_I                                        0xf40a00ff

/*POWERQ*/
#define RSTV0913_POWERQ                                        0xf40b
#define FSTV0913_POWER_Q                                        0xf40b00ff

/*AGC1AMM*/
#define RSTV0913_AGC1AMM                                        0xf40c
#define FSTV0913_AMM_VALUE                                        0xf40c00ff

/*AGC1QUAD*/
#define RSTV0913_AGC1QUAD                                        0xf40d
#define FSTV0913_QUAD_VALUE                                        0xf40d01ff

/*AGCIQIN1*/
#define RSTV0913_AGCIQIN1                                        0xf40e
#define FSTV0913_AGCIQ_VALUE1                                        0xf40e00ff

/*AGCIQIN0*/
#define RSTV0913_AGCIQIN0                                        0xf40f
#define FSTV0913_AGCIQ_VALUE0                                        0xf40f00ff

/*DEMOD*/
#define RSTV0913_DEMOD                                        0xf410
#define FSTV0913_MANUALS2_ROLLOFF                                        0xf4100080
#define FSTV0913_SPECINV_CONTROL                                        0xf4100030
#define FSTV0913_MANUALSX_ROLLOFF                                        0xf4100004
#define FSTV0913_ROLLOFF_CONTROL                                        0xf4100003

/*DMDMODCOD*/
#define RSTV0913_DMDMODCOD                                        0xf411
#define FSTV0913_MANUAL_MODCOD                                        0xf4110080
#define FSTV0913_DEMOD_MODCOD                                        0xf411007c
#define FSTV0913_DEMOD_TYPE                                        0xf4110003

/*DSTATUS*/
#define RSTV0913_DSTATUS                                        0xf412
#define FSTV0913_CAR_LOCK                                        0xf4120080
#define FSTV0913_TMGLOCK_QUALITY                                        0xf4120060
#define FSTV0913_LOCK_DEFINITIF                                        0xf4120008
#define FSTV0913_OVADC_DETECT                                        0xf4120001

/*DSTATUS2*/
#define RSTV0913_DSTATUS2                                        0xf413
#define FSTV0913_DEMOD_DELOCK                                        0xf4130080
#define FSTV0913_AGC1_NOSIGNALACK                                        0xf4130008
#define FSTV0913_AGC2_OVERFLOW                                        0xf4130004
#define FSTV0913_CFR_OVERFLOW                                        0xf4130002
#define FSTV0913_GAMMA_OVERUNDER                                        0xf4130001

/*DMDCFGMD*/
#define RSTV0913_DMDCFGMD                                        0xf414
#define FSTV0913_DVBS2_ENABLE                                        0xf4140080
#define FSTV0913_DVBS1_ENABLE                                        0xf4140040
#define FSTV0913_SCAN_ENABLE                                        0xf4140010
#define FSTV0913_CFR_AUTOSCAN                                        0xf4140008
#define FSTV0913_TUN_RNG                                        0xf4140003

/*DMDCFG2*/
#define RSTV0913_DMDCFG2                                        0xf415
#define FSTV0913_S1S2_SEQUENTIAL                                        0xf4150040
#define FSTV0913_INFINITE_RELOCK                                        0xf4150010

/*DMDISTATE*/
#define RSTV0913_DMDISTATE                                        0xf416
#define FSTV0913_I2C_DEMOD_MODE                                        0xf416001f

/*DMDT0M*/
#define RSTV0913_DMDT0M                                        0xf417
#define FSTV0913_DMDT0_MIN                                        0xf41700ff

/*DMDSTATE*/
#define RSTV0913_DMDSTATE                                        0xf41b
#define FSTV0913_HEADER_MODE                                        0xf41b0060

/*DMDFLYW*/
#define RSTV0913_DMDFLYW                                        0xf41c
#define FSTV0913_I2C_IRQVAL                                        0xf41c00f0
#define FSTV0913_FLYWHEEL_CPT                                        0xf41c000f

/*DSTATUS3*/
#define RSTV0913_DSTATUS3                                        0xf41d
#define FSTV0913_DEMOD_CFGMODE                                        0xf41d0060

/*DMDCFG3*/
#define RSTV0913_DMDCFG3                                        0xf41e
#define FSTV0913_NOSTOP_FIFOFULL                                        0xf41e0008

/*DMDCFG4*/
#define RSTV0913_DMDCFG4                                        0xf41f
#define FSTV0913_TUNER_NRELAUNCH                                        0xf41f0008

/*CORRELMANT*/
#define RSTV0913_CORRELMANT                                        0xf420
#define FSTV0913_CORREL_MANT                                        0xf42000ff

/*CORRELABS*/
#define RSTV0913_CORRELABS                                        0xf421
#define FSTV0913_CORREL_ABS                                        0xf42100ff

/*CORRELEXP*/
#define RSTV0913_CORRELEXP                                        0xf422
#define FSTV0913_CORREL_ABSEXP                                        0xf42200f0
#define FSTV0913_CORREL_EXP                                        0xf422000f

/*PLHMODCOD*/
#define RSTV0913_PLHMODCOD                                        0xf424
#define FSTV0913_SPECINV_DEMOD                                        0xf4240080
#define FSTV0913_PLH_MODCOD                                        0xf424007c
#define FSTV0913_PLH_TYPE                                        0xf4240003

/*DMDREG*/
#define RSTV0913_DMDREG                                        0xf425
#define FSTV0913_HIER_SHORTFRAME                                        0xf4250002

/*AGC2O*/
#define RSTV0913_AGC2O                                        0xf42c
#define FSTV0913_AGC2_COEF                                        0xf42c0007

/*AGC2REF*/
#define RSTV0913_AGC2REF                                        0xf42d
#define FSTV0913_AGC2_REF                                        0xf42d00ff

/*AGC1ADJ*/
#define RSTV0913_AGC1ADJ                                        0xf42e
#define FSTV0913_AGC1_ADJUSTED                                        0xf42e007f

/*AGC2I1*/
#define RSTV0913_AGC2I1                                        0xf436
#define FSTV0913_AGC2_INTEGRATOR1                                        0xf43600ff

/*AGC2I0*/
#define RSTV0913_AGC2I0                                        0xf437
#define FSTV0913_AGC2_INTEGRATOR0                                        0xf43700ff

/*CARCFG*/
#define RSTV0913_CARCFG                                        0xf438
#define FSTV0913_ROTAON                                        0xf4380004
#define FSTV0913_PH_DET_ALGO                                        0xf4380003

/*ACLC*/
#define RSTV0913_ACLC                                        0xf439
#define FSTV0913_CAR_ALPHA_MANT                                        0xf4390030
#define FSTV0913_CAR_ALPHA_EXP                                        0xf439000f

/*BCLC*/
#define RSTV0913_BCLC                                        0xf43a
#define FSTV0913_CAR_BETA_MANT                                        0xf43a0030
#define FSTV0913_CAR_BETA_EXP                                        0xf43a000f

/*CARFREQ*/
#define RSTV0913_CARFREQ                                        0xf43d
#define FSTV0913_KC_COARSE_EXP                                        0xf43d00f0
#define FSTV0913_BETA_FREQ                                        0xf43d000f

/*CARHDR*/
#define RSTV0913_CARHDR                                        0xf43e
#define FSTV0913_K_FREQ_HDR                                        0xf43e00ff

/*LDT*/
#define RSTV0913_LDT                                        0xf43f
#define FSTV0913_CARLOCK_THRES                                        0xf43f01ff

/*LDT2*/
#define RSTV0913_LDT2                                        0xf440
#define FSTV0913_CARLOCK_THRES2                                        0xf44001ff

/*CFRICFG*/
#define RSTV0913_CFRICFG                                        0xf441
#define FSTV0913_NEG_CFRSTEP                                        0xf4410001

/*CFRUP1*/
#define RSTV0913_CFRUP1                                        0xf442
#define FSTV0913_CFR_UP1                                        0xf44201ff

/*CFRUP0*/
#define RSTV0913_CFRUP0                                        0xf443
#define FSTV0913_CFR_UP0                                        0xf44300ff

/*CFRLOW1*/
#define RSTV0913_CFRLOW1                                        0xf446
#define FSTV0913_CFR_LOW1                                        0xf44601ff

/*CFRLOW0*/
#define RSTV0913_CFRLOW0                                        0xf447
#define FSTV0913_CFR_LOW0                                        0xf44700ff

/*CFRINIT1*/
#define RSTV0913_CFRINIT1                                        0xf448
#define FSTV0913_CFR_INIT1                                        0xf44801ff

/*CFRINIT0*/
#define RSTV0913_CFRINIT0                                        0xf449
#define FSTV0913_CFR_INIT0                                        0xf44900ff

/*CFRINC1*/
#define RSTV0913_CFRINC1                                        0xf44a
#define FSTV0913_MANUAL_CFRINC                                        0xf44a0080
#define FSTV0913_CFR_INC1                                        0xf44a003f

/*CFRINC0*/
#define RSTV0913_CFRINC0                                        0xf44b
#define FSTV0913_CFR_INC0                                        0xf44b00ff

/*CFR2*/
#define RSTV0913_CFR2                                        0xf44c
#define FSTV0913_CAR_FREQ2                                        0xf44c01ff

/*CFR1*/
#define RSTV0913_CFR1                                        0xf44d
#define FSTV0913_CAR_FREQ1                                        0xf44d00ff

/*CFR0*/
#define RSTV0913_CFR0                                        0xf44e
#define FSTV0913_CAR_FREQ0                                        0xf44e00ff

/*LDI*/
#define RSTV0913_LDI                                        0xf44f
#define FSTV0913_LOCK_DET_INTEGR                                        0xf44f01ff

/*TMGCFG*/
#define RSTV0913_TMGCFG                                        0xf450
#define FSTV0913_TMGLOCK_BETA                                        0xf45000c0
#define FSTV0913_DO_TIMING_CORR                                        0xf4500010
#define FSTV0913_TMG_MINFREQ                                        0xf4500003

/*RTC*/
#define RSTV0913_RTC                                        0xf451
#define FSTV0913_TMGALPHA_EXP                                        0xf45100f0
#define FSTV0913_TMGBETA_EXP                                        0xf451000f

/*RTCS2*/
#define RSTV0913_RTCS2                                        0xf452
#define FSTV0913_TMGALPHAS2_EXP                                        0xf45200f0
#define FSTV0913_TMGBETAS2_EXP                                        0xf452000f

/*TMGTHRISE*/
#define RSTV0913_TMGTHRISE                                        0xf453
#define FSTV0913_TMGLOCK_THRISE                                        0xf45300ff

/*TMGTHFALL*/
#define RSTV0913_TMGTHFALL                                        0xf454
#define FSTV0913_TMGLOCK_THFALL                                        0xf45400ff

/*SFRUPRATIO*/
#define RSTV0913_SFRUPRATIO                                        0xf455
#define FSTV0913_SFR_UPRATIO                                        0xf45500ff

/*SFRLOWRATIO*/
#define RSTV0913_SFRLOWRATIO                                        0xf456
#define FSTV0913_SFR_LOWRATIO                                        0xf45600ff

/*KREFTMG*/
#define RSTV0913_KREFTMG                                        0xf458
#define FSTV0913_KREF_TMG                                        0xf45800ff

/*SFRSTEP*/
#define RSTV0913_SFRSTEP                                        0xf459
#define FSTV0913_SFR_SCANSTEP                                        0xf45900f0
#define FSTV0913_SFR_CENTERSTEP                                        0xf459000f

/*TMGCFG2*/
#define RSTV0913_TMGCFG2                                        0xf45a
#define FSTV0913_SFRRATIO_FINE                                        0xf45a0001

/*KREFTMG2*/
#define RSTV0913_KREFTMG2                                        0xf45b
#define FSTV0913_KREF_TMG2                                        0xf45b00ff

/*TMGCFG3*/
#define RSTV0913_TMGCFG3                                        0xf45d
#define FSTV0913_AUTO_GUP                                        0xf45d0004
#define FSTV0913_AUTO_GLOW                                        0xf45d0002

/*SFRINIT1*/
#define RSTV0913_SFRINIT1                                        0xf45e
#define FSTV0913_SFR_INIT1                                        0xf45e00ff

/*SFRINIT0*/
#define RSTV0913_SFRINIT0                                        0xf45f
#define FSTV0913_SFR_INIT0                                        0xf45f00ff

/*SFRUP1*/
#define RSTV0913_SFRUP1                                        0xf460
#define FSTV0913_SYMB_FREQ_UP1                                        0xf46000ff

/*SFRUP0*/
#define RSTV0913_SFRUP0                                        0xf461
#define FSTV0913_SYMB_FREQ_UP0                                        0xf46100ff

/*SFRLOW1*/
#define RSTV0913_SFRLOW1                                        0xf462
#define FSTV0913_SYMB_FREQ_LOW1                                        0xf46200ff

/*SFRLOW0*/
#define RSTV0913_SFRLOW0                                        0xf463
#define FSTV0913_SYMB_FREQ_LOW0                                        0xf46300ff

/*SFR3*/
#define RSTV0913_SFR3                                        0xf464
#define FSTV0913_SYMB_FREQ3                                        0xf46400ff

/*SFR2*/
#define RSTV0913_SFR2                                        0xf465
#define FSTV0913_SYMB_FREQ2                                        0xf46500ff

/*SFR1*/
#define RSTV0913_SFR1                                        0xf466
#define FSTV0913_SYMB_FREQ1                                        0xf46600ff

/*SFR0*/
#define RSTV0913_SFR0                                        0xf467
#define FSTV0913_SYMB_FREQ0                                        0xf46700ff

/*TMGREG2*/
#define RSTV0913_TMGREG2                                        0xf468
#define FSTV0913_TMGREG2                                        0xf46800ff

/*TMGREG1*/
#define RSTV0913_TMGREG1                                        0xf469
#define FSTV0913_TMGREG1                                        0xf46900ff

/*TMGREG0*/
#define RSTV0913_TMGREG0                                        0xf46a
#define FSTV0913_TMGREG0                                        0xf46a00ff

/*TMGLOCK1*/
#define RSTV0913_TMGLOCK1                                        0xf46b
#define FSTV0913_TMGLOCK_LEVEL1                                        0xf46b01ff

/*TMGLOCK0*/
#define RSTV0913_TMGLOCK0                                        0xf46c
#define FSTV0913_TMGLOCK_LEVEL0                                        0xf46c00ff

/*TMGOBS*/
#define RSTV0913_TMGOBS                                        0xf46d
#define FSTV0913_ROLLOFF_STATUS                                        0xf46d00c0

/*EQUALCFG*/
#define RSTV0913_EQUALCFG                                        0xf46f
#define FSTV0913_EQUAL_ON                                        0xf46f0040
#define FSTV0913_MU_EQUALDFE                                        0xf46f0007

/*EQUAI1*/
#define RSTV0913_EQUAI1                                        0xf470
#define FSTV0913_EQUA_ACCI1                                        0xf47001ff

/*EQUAQ1*/
#define RSTV0913_EQUAQ1                                        0xf471
#define FSTV0913_EQUA_ACCQ1                                        0xf47101ff

/*EQUAI2*/
#define RSTV0913_EQUAI2                                        0xf472
#define FSTV0913_EQUA_ACCI2                                        0xf47201ff

/*EQUAQ2*/
#define RSTV0913_EQUAQ2                                        0xf473
#define FSTV0913_EQUA_ACCQ2                                        0xf47301ff

/*EQUAI3*/
#define RSTV0913_EQUAI3                                        0xf474
#define FSTV0913_EQUA_ACCI3                                        0xf47401ff

/*EQUAQ3*/
#define RSTV0913_EQUAQ3                                        0xf475
#define FSTV0913_EQUA_ACCQ3                                        0xf47501ff

/*EQUAI4*/
#define RSTV0913_EQUAI4                                        0xf476
#define FSTV0913_EQUA_ACCI4                                        0xf47601ff

/*EQUAQ4*/
#define RSTV0913_EQUAQ4                                        0xf477
#define FSTV0913_EQUA_ACCQ4                                        0xf47701ff

/*EQUAI5*/
#define RSTV0913_EQUAI5                                        0xf478
#define FSTV0913_EQUA_ACCI5                                        0xf47801ff

/*EQUAQ5*/
#define RSTV0913_EQUAQ5                                        0xf479
#define FSTV0913_EQUA_ACCQ5                                        0xf47901ff

/*EQUAI6*/
#define RSTV0913_EQUAI6                                        0xf47a
#define FSTV0913_EQUA_ACCI6                                        0xf47a01ff

/*EQUAQ6*/
#define RSTV0913_EQUAQ6                                        0xf47b
#define FSTV0913_EQUA_ACCQ6                                        0xf47b01ff

/*EQUAI7*/
#define RSTV0913_EQUAI7                                        0xf47c
#define FSTV0913_EQUA_ACCI7                                        0xf47c01ff

/*EQUAQ7*/
#define RSTV0913_EQUAQ7                                        0xf47d
#define FSTV0913_EQUA_ACCQ7                                        0xf47d01ff

/*EQUAI8*/
#define RSTV0913_EQUAI8                                        0xf47e
#define FSTV0913_EQUA_ACCI8                                        0xf47e01ff

/*EQUAQ8*/
#define RSTV0913_EQUAQ8                                        0xf47f
#define FSTV0913_EQUA_ACCQ8                                        0xf47f01ff

/*NNOSDATAT1*/
#define RSTV0913_NNOSDATAT1                                        0xf480
#define FSTV0913_NOSDATAT_NORMED1                                        0xf48000ff

/*NNOSDATAT0*/
#define RSTV0913_NNOSDATAT0                                        0xf481
#define FSTV0913_NOSDATAT_NORMED0                                        0xf48100ff

/*NNOSDATA1*/
#define RSTV0913_NNOSDATA1                                        0xf482
#define FSTV0913_NOSDATA_NORMED1                                        0xf48200ff

/*NNOSDATA0*/
#define RSTV0913_NNOSDATA0                                        0xf483
#define FSTV0913_NOSDATA_NORMED0                                        0xf48300ff

/*NNOSPLHT1*/
#define RSTV0913_NNOSPLHT1                                        0xf484
#define FSTV0913_NOSPLHT_NORMED1                                        0xf48400ff

/*NNOSPLHT0*/
#define RSTV0913_NNOSPLHT0                                        0xf485
#define FSTV0913_NOSPLHT_NORMED0                                        0xf48500ff

/*NNOSPLH1*/
#define RSTV0913_NNOSPLH1                                        0xf486
#define FSTV0913_NOSPLH_NORMED1                                        0xf48600ff

/*NNOSPLH0*/
#define RSTV0913_NNOSPLH0                                        0xf487
#define FSTV0913_NOSPLH_NORMED0                                        0xf48700ff

/*NOSDATAT1*/
#define RSTV0913_NOSDATAT1                                        0xf488
#define FSTV0913_NOSDATAT_UNNORMED1                                        0xf48800ff

/*NOSDATAT0*/
#define RSTV0913_NOSDATAT0                                        0xf489
#define FSTV0913_NOSDATAT_UNNORMED0                                        0xf48900ff

/*NNOSFRAME1*/
#define RSTV0913_NNOSFRAME1                                        0xf48a
#define FSTV0913_NOSFRAME_NORMED1                                        0xf48a00ff

/*NNOSFRAME0*/
#define RSTV0913_NNOSFRAME0                                        0xf48b
#define FSTV0913_NOSFRAME_NORMED0                                        0xf48b00ff

/*NNOSRAD1*/
#define RSTV0913_NNOSRAD1                                        0xf48c
#define FSTV0913_NOSRADIAL_NORMED1                                        0xf48c00ff

/*NNOSRAD0*/
#define RSTV0913_NNOSRAD0                                        0xf48d
#define FSTV0913_NOSRADIAL_NORMED0                                        0xf48d00ff

/*NOSCFGF1*/
#define RSTV0913_NOSCFGF1                                        0xf48e
#define FSTV0913_LOWNOISE_MESURE                                        0xf48e0080

/*CAR2CFG*/
#define RSTV0913_CAR2CFG                                        0xf490
#define FSTV0913_ROTA2ON                                        0xf4900004
#define FSTV0913_PH_DET_ALGO2                                        0xf4900003

/*CFR2CFR1*/
#define RSTV0913_CFR2CFR1                                        0xf491
#define FSTV0913_EN_S2CAR2CENTER                                        0xf4910020
#define FSTV0913_CFR2TOCFR1_BETA                                        0xf4910007

/*CAR3CFG*/
#define RSTV0913_CAR3CFG                                        0xf492
#define FSTV0913_CARRIER23_MODE                                        0xf49200c0
#define FSTV0913_CAR3INTERM_DVBS1                                        0xf4920020
#define FSTV0913_ABAMPLIF_MODE                                        0xf4920018
#define FSTV0913_CARRIER3_ALPHA3DL                                        0xf4920007

/*CFR22*/
#define RSTV0913_CFR22                                        0xf493
#define FSTV0913_CAR2_FREQ2                                        0xf49301ff

/*CFR21*/
#define RSTV0913_CFR21                                        0xf494
#define FSTV0913_CAR2_FREQ1                                        0xf49400ff

/*CFR20*/
#define RSTV0913_CFR20                                        0xf495
#define FSTV0913_CAR2_FREQ0                                        0xf49500ff

/*ACLC2S2Q*/
#define RSTV0913_ACLC2S2Q                                        0xf497
#define FSTV0913_ENAB_SPSKSYMB                                        0xf4970080
#define FSTV0913_CAR2S2_Q_ALPH_M                                        0xf4970030
#define FSTV0913_CAR2S2_Q_ALPH_E                                        0xf497000f

/*ACLC2S28*/
#define RSTV0913_ACLC2S28                                        0xf498
#define FSTV0913_OLDI3Q_MODE                                        0xf4980080
#define FSTV0913_CAR2S2_8_ALPH_M                                        0xf4980030
#define FSTV0913_CAR2S2_8_ALPH_E                                        0xf498000f

/*ACLC2S216A*/
#define RSTV0913_ACLC2S216A                                        0xf499
#define FSTV0913_CAR2S2_16A_ALPH_M                                        0xf4990030
#define FSTV0913_CAR2S2_16A_ALPH_E                                        0xf499000f

/*ACLC2S232A*/
#define RSTV0913_ACLC2S232A                                        0xf49a
#define FSTV0913_CAR2S2_32A_ALPH_M                                        0xf49a0030
#define FSTV0913_CAR2S2_32A_ALPH_E                                        0xf49a000f

/*BCLC2S2Q*/
#define RSTV0913_BCLC2S2Q                                        0xf49c
#define FSTV0913_CAR2S2_Q_BETA_M                                        0xf49c0030
#define FSTV0913_CAR2S2_Q_BETA_E                                        0xf49c000f

/*BCLC2S28*/
#define RSTV0913_BCLC2S28                                        0xf49d
#define FSTV0913_CAR2S2_8_BETA_M                                        0xf49d0030
#define FSTV0913_CAR2S2_8_BETA_E                                        0xf49d000f

/*PLROOT2*/
#define RSTV0913_PLROOT2                                        0xf4ac
#define FSTV0913_PLSCRAMB_ROOT2                                        0xf4ac0003

/*PLROOT1*/
#define RSTV0913_PLROOT1                                        0xf4ad
#define FSTV0913_PLSCRAMB_ROOT1                                        0xf4ad00ff

/*PLROOT0*/
#define RSTV0913_PLROOT0                                        0xf4ae
#define FSTV0913_PLSCRAMB_ROOT0                                        0xf4ae00ff

/*MODCODLST1*/
#define RSTV0913_MODCODLST1                                        0xf4b1
#define FSTV0913_DIS_32PSK_9_10                                        0xf4b10003

/*MODCODLST2*/
#define RSTV0913_MODCODLST2                                        0xf4b2
#define FSTV0913_DIS_32PSK_8_9                                        0xf4b200f0
#define FSTV0913_DIS_32PSK_5_6                                        0xf4b2000f

/*MODCODLST3*/
#define RSTV0913_MODCODLST3                                        0xf4b3
#define FSTV0913_DIS_32PSK_4_5                                        0xf4b300f0
#define FSTV0913_DIS_32PSK_3_4                                        0xf4b3000f

/*MODCODLST4*/
#define RSTV0913_MODCODLST4                                        0xf4b4
#define FSTV0913_DIS_16PSK_9_10                                        0xf4b40030
#define FSTV0913_DIS_16PSK_8_9                                        0xf4b4000f

/*MODCODLST5*/
#define RSTV0913_MODCODLST5                                        0xf4b5
#define FSTV0913_DIS_16PSK_5_6                                        0xf4b500f0
#define FSTV0913_DIS_16PSK_4_5                                        0xf4b5000f

/*MODCODLST6*/
#define RSTV0913_MODCODLST6                                        0xf4b6
#define FSTV0913_DIS_16PSK_3_4                                        0xf4b600f0
#define FSTV0913_DIS_16PSK_2_3                                        0xf4b6000f

/*MODCODLST7*/
#define RSTV0913_MODCODLST7                                        0xf4b7
#define FSTV0913_DIS_8PSK_9_10                                        0xf4b70030
#define FSTV0913_DIS_8P_8_9                                        0xf4b7000f

/*MODCODLST8*/
#define RSTV0913_MODCODLST8                                        0xf4b8
#define FSTV0913_DIS_8P_5_6                                        0xf4b800f0
#define FSTV0913_DIS_8P_3_4                                        0xf4b8000f

/*MODCODLST9*/
#define RSTV0913_MODCODLST9                                        0xf4b9
#define FSTV0913_DIS_8P_2_3                                        0xf4b900f0
#define FSTV0913_DIS_8P_3_5                                        0xf4b9000f

/*MODCODLSTA*/
#define RSTV0913_MODCODLSTA                                        0xf4ba
#define FSTV0913_DIS_QPSK_9_10                                        0xf4ba0030
#define FSTV0913_DIS_QP_8_9                                        0xf4ba000f

/*MODCODLSTB*/
#define RSTV0913_MODCODLSTB                                        0xf4bb
#define FSTV0913_DIS_QP_5_6                                        0xf4bb00f0
#define FSTV0913_DIS_QP_4_5                                        0xf4bb000f

/*MODCODLSTC*/
#define RSTV0913_MODCODLSTC                                        0xf4bc
#define FSTV0913_DIS_QP_3_4                                        0xf4bc00f0
#define FSTV0913_DIS_QP_2_3                                        0xf4bc000f

/*MODCODLSTD*/
#define RSTV0913_MODCODLSTD                                        0xf4bd
#define FSTV0913_DIS_QPSK_3_5                                        0xf4bd00f0
#define FSTV0913_DIS_QPSK_1_2                                        0xf4bd000f

/*MODCODLSTE*/
#define RSTV0913_MODCODLSTE                                        0xf4be
#define FSTV0913_DIS_QPSK_2_5                                        0xf4be00f0
#define FSTV0913_DIS_QPSK_1_3                                        0xf4be000f

/*MODCODLSTF*/
#define RSTV0913_MODCODLSTF                                        0xf4bf
#define FSTV0913_DIS_QPSK_1_4                                        0xf4bf00f0

/*GAUSSR0*/
#define RSTV0913_GAUSSR0                                        0xf4c0
#define FSTV0913_EN_CCIMODE                                        0xf4c00080
#define FSTV0913_R0_GAUSSIEN                                        0xf4c0007f

/*CCIR0*/
#define RSTV0913_CCIR0                                        0xf4c1
#define FSTV0913_CCIDETECT_PLHONLY                                        0xf4c10080
#define FSTV0913_R0_CCI                                        0xf4c1007f

/*CCIQUANT*/
#define RSTV0913_CCIQUANT                                        0xf4c2
#define FSTV0913_CCI_BETA                                        0xf4c200e0
#define FSTV0913_CCI_QUANT                                        0xf4c2001f

/*CCITHRES*/
#define RSTV0913_CCITHRES                                        0xf4c3
#define FSTV0913_CCI_THRESHOLD                                        0xf4c300ff

/*CCIACC*/
#define RSTV0913_CCIACC                                        0xf4c4
#define FSTV0913_CCI_VALUE                                        0xf4c400ff

/*DSTATUS4*/
#define RSTV0913_DSTATUS4                                        0xf4c5
#define FSTV0913_RAINFADE_DETECT                                        0xf4c50080
#define FSTV0913_NOTHRES2_FAIL                                        0xf4c50040
#define FSTV0913_NOTHRES1_FAIL                                        0xf4c50020

/*DMDRESCFG*/
#define RSTV0913_DMDRESCFG                                        0xf4c6
#define FSTV0913_DMDRES_RESET                                        0xf4c60080
#define FSTV0913_DMDRES_STRALL                                        0xf4c60008
#define FSTV0913_DMDRES_NEWONLY                                        0xf4c60004
#define FSTV0913_DMDRES_NOSTORE                                        0xf4c60002

/*DMDRESADR*/
#define RSTV0913_DMDRESADR                                        0xf4c7
#define FSTV0913_DMDRES_VALIDCFR                                        0xf4c70040
#define FSTV0913_DMDRES_MEMFULL                                        0xf4c70030
#define FSTV0913_DMDRES_RESNBR                                        0xf4c7000f

/*DMDRESDATA7*/
#define RSTV0913_DMDRESDATA7                                        0xf4c8
#define FSTV0913_DMDRES_DATA7                                        0xf4c800ff

/*DMDRESDATA6*/
#define RSTV0913_DMDRESDATA6                                        0xf4c9
#define FSTV0913_DMDRES_DATA6                                        0xf4c900ff

/*DMDRESDATA5*/
#define RSTV0913_DMDRESDATA5                                        0xf4ca
#define FSTV0913_DMDRES_DATA5                                        0xf4ca00ff

/*DMDRESDATA4*/
#define RSTV0913_DMDRESDATA4                                        0xf4cb
#define FSTV0913_DMDRES_DATA4                                        0xf4cb00ff

/*DMDRESDATA3*/
#define RSTV0913_DMDRESDATA3                                        0xf4cc
#define FSTV0913_DMDRES_DATA3                                        0xf4cc00ff

/*DMDRESDATA2*/
#define RSTV0913_DMDRESDATA2                                        0xf4cd
#define FSTV0913_DMDRES_DATA2                                        0xf4cd00ff

/*DMDRESDATA1*/
#define RSTV0913_DMDRESDATA1                                        0xf4ce
#define FSTV0913_DMDRES_DATA1                                        0xf4ce00ff

/*DMDRESDATA0*/
#define RSTV0913_DMDRESDATA0                                        0xf4cf
#define FSTV0913_DMDRES_DATA0                                        0xf4cf00ff

/*FFEI1*/
#define RSTV0913_FFEI1                                        0xf4d0
#define FSTV0913_FFE_ACCI1                                        0xf4d001ff

/*FFEQ1*/
#define RSTV0913_FFEQ1                                        0xf4d1
#define FSTV0913_FFE_ACCQ1                                        0xf4d101ff

/*FFEI2*/
#define RSTV0913_FFEI2                                        0xf4d2
#define FSTV0913_FFE_ACCI2                                        0xf4d201ff

/*FFEQ2*/
#define RSTV0913_FFEQ2                                        0xf4d3
#define FSTV0913_FFE_ACCQ2                                        0xf4d301ff

/*FFEI3*/
#define RSTV0913_FFEI3                                        0xf4d4
#define FSTV0913_FFE_ACCI3                                        0xf4d401ff

/*FFEQ3*/
#define RSTV0913_FFEQ3                                        0xf4d5
#define FSTV0913_FFE_ACCQ3                                        0xf4d501ff

/*FFEI4*/
#define RSTV0913_FFEI4                                        0xf4d6
#define FSTV0913_FFE_ACCI4                                        0xf4d601ff

/*FFEQ4*/
#define RSTV0913_FFEQ4                                        0xf4d7
#define FSTV0913_FFE_ACCQ4                                        0xf4d701ff

/*FFECFG*/
#define RSTV0913_FFECFG                                        0xf4d8
#define FSTV0913_EQUALFFE_ON                                        0xf4d80040
#define FSTV0913_MU_EQUALFFE                                        0xf4d80007

/*TNRCFG2*/
#define RSTV0913_TNRCFG2                                        0xf4e1
#define FSTV0913_TUN_IQSWAP                                        0xf4e10080

/*SMAPCOEF7*/
#define RSTV0913_SMAPCOEF7                                        0xf500
#define FSTV0913_SMAPCOEF_Q_LLR12                                        0xf500017f

/*SMAPCOEF6*/
#define RSTV0913_SMAPCOEF6                                        0xf501
#define FSTV0913_ADJ_8PSKLLR1                                        0xf5010004
#define FSTV0913_OLD_8PSKLLR1                                        0xf5010002
#define FSTV0913_DIS_AB8PSK                                        0xf5010001

/*SMAPCOEF5*/
#define RSTV0913_SMAPCOEF5                                        0xf502
#define FSTV0913_DIS_8SCALE                                        0xf5020080
#define FSTV0913_SMAPCOEF_8P_LLR23                                        0xf502017f

/*NOSTHRES1*/
#define RSTV0913_NOSTHRES1                                        0xf509
#define FSTV0913_NOS_THRESHOLD1                                        0xf50900ff

/*NOSTHRES2*/
#define RSTV0913_NOSTHRES2                                        0xf50a
#define FSTV0913_NOS_THRESHOLD2                                        0xf50a00ff

/*NOSDIFF1*/
#define RSTV0913_NOSDIFF1                                        0xf50b
#define FSTV0913_NOSTHRES1_DIFF                                        0xf50b00ff

/*RAINFADE*/
#define RSTV0913_RAINFADE                                        0xf50c
#define FSTV0913_NOSTHRES_DATAT                                        0xf50c0080
#define FSTV0913_RAINFADE_CNLIMIT                                        0xf50c0070
#define FSTV0913_RAINFADE_TIMEOUT                                        0xf50c0007

/*NOSRAMCFG*/
#define RSTV0913_NOSRAMCFG                                        0xf50d
#define FSTV0913_NOSRAM_ACTIVATION                                        0xf50d0030
#define FSTV0913_NOSRAM_CNRONLY                                        0xf50d0008
#define FSTV0913_NOSRAM_LGNCNR1                                        0xf50d0007

/*NOSRAMPOS*/
#define RSTV0913_NOSRAMPOS                                        0xf50e
#define FSTV0913_NOSRAM_LGNCNR0                                        0xf50e00f0
#define FSTV0913_NOSRAM_VALIDE                                        0xf50e0004
#define FSTV0913_NOSRAM_CNRVAL1                                        0xf50e0003

/*NOSRAMVAL*/
#define RSTV0913_NOSRAMVAL                                        0xf50f
#define FSTV0913_NOSRAM_CNRVAL0                                        0xf50f00ff

/*DMDPLHSTAT*/
#define RSTV0913_DMDPLHSTAT                                        0xf520
#define FSTV0913_PLH_STATISTIC                                        0xf52000ff

/*LOCKTIME3*/
#define RSTV0913_LOCKTIME3                                        0xf522
#define FSTV0913_DEMOD_LOCKTIME3                                        0xf52200ff

/*LOCKTIME2*/
#define RSTV0913_LOCKTIME2                                        0xf523
#define FSTV0913_DEMOD_LOCKTIME2                                        0xf52300ff

/*LOCKTIME1*/
#define RSTV0913_LOCKTIME1                                        0xf524
#define FSTV0913_DEMOD_LOCKTIME1                                        0xf52400ff

/*LOCKTIME0*/
#define RSTV0913_LOCKTIME0                                        0xf525
#define FSTV0913_DEMOD_LOCKTIME0                                        0xf52500ff

/*VITSCALE*/
#define RSTV0913_VITSCALE                                        0xf532
#define FSTV0913_NVTH_NOSRANGE                                        0xf5320080
#define FSTV0913_VERROR_MAXMODE                                        0xf5320040
#define FSTV0913_NSLOWSN_LOCKED                                        0xf5320008
#define FSTV0913_DIS_RSFLOCK                                        0xf5320002

/*FECM*/
#define RSTV0913_FECM                                        0xf533
#define FSTV0913_DSS_DVB                                        0xf5330080
#define FSTV0913_DSS_SRCH                                        0xf5330010
#define FSTV0913_SYNCVIT                                        0xf5330002
#define FSTV0913_IQINV                                        0xf5330001

/*VTH12*/
#define RSTV0913_VTH12                                        0xf534
#define FSTV0913_VTH12                                        0xf53400ff

/*VTH23*/
#define RSTV0913_VTH23                                        0xf535
#define FSTV0913_VTH23                                        0xf53500ff

/*VTH34*/
#define RSTV0913_VTH34                                        0xf536
#define FSTV0913_VTH34                                        0xf53600ff

/*VTH56*/
#define RSTV0913_VTH56                                        0xf537
#define FSTV0913_VTH56                                        0xf53700ff

/*VTH67*/
#define RSTV0913_VTH67                                        0xf538
#define FSTV0913_VTH67                                        0xf53800ff

/*VTH78*/
#define RSTV0913_VTH78                                        0xf539
#define FSTV0913_VTH78                                        0xf53900ff

/*VITCURPUN*/
#define RSTV0913_VITCURPUN                                        0xf53a
#define FSTV0913_VIT_CURPUN                                        0xf53a001f

/*VERROR*/
#define RSTV0913_VERROR                                        0xf53b
#define FSTV0913_REGERR_VIT                                        0xf53b00ff

/*PRVIT*/
#define RSTV0913_PRVIT                                        0xf53c
#define FSTV0913_DIS_VTHLOCK                                        0xf53c0040
#define FSTV0913_E7_8VIT                                        0xf53c0020
#define FSTV0913_E6_7VIT                                        0xf53c0010
#define FSTV0913_E5_6VIT                                        0xf53c0008
#define FSTV0913_E3_4VIT                                        0xf53c0004
#define FSTV0913_E2_3VIT                                        0xf53c0002
#define FSTV0913_E1_2VIT                                        0xf53c0001

/*VAVSRVIT*/
#define RSTV0913_VAVSRVIT                                        0xf53d
#define FSTV0913_AMVIT                                        0xf53d0080
#define FSTV0913_FROZENVIT                                        0xf53d0040
#define FSTV0913_SNVIT                                        0xf53d0030
#define FSTV0913_TOVVIT                                        0xf53d000c
#define FSTV0913_HYPVIT                                        0xf53d0003

/*VSTATUSVIT*/
#define RSTV0913_VSTATUSVIT                                        0xf53e
#define FSTV0913_PRFVIT                                        0xf53e0010
#define FSTV0913_LOCKEDVIT                                        0xf53e0008

/*VTHINUSE*/
#define RSTV0913_VTHINUSE                                        0xf53f
#define FSTV0913_VIT_INUSE                                        0xf53f00ff

/*KDIV12*/
#define RSTV0913_KDIV12                                        0xf540
#define FSTV0913_K_DIVIDER_12                                        0xf540007f

/*KDIV23*/
#define RSTV0913_KDIV23                                        0xf541
#define FSTV0913_K_DIVIDER_23                                        0xf541007f

/*KDIV34*/
#define RSTV0913_KDIV34                                        0xf542
#define FSTV0913_K_DIVIDER_34                                        0xf542007f

/*KDIV56*/
#define RSTV0913_KDIV56                                        0xf543
#define FSTV0913_K_DIVIDER_56                                        0xf543007f

/*KDIV67*/
#define RSTV0913_KDIV67                                        0xf544
#define FSTV0913_K_DIVIDER_67                                        0xf544007f

/*KDIV78*/
#define RSTV0913_KDIV78                                        0xf545
#define FSTV0913_K_DIVIDER_78                                        0xf545007f

/*PDELCTRL1*/
#define RSTV0913_PDELCTRL1                                        0xf550
#define FSTV0913_ALGOSWRST                                        0xf5500001

/*PDELCTRL2*/
#define RSTV0913_PDELCTRL2                                        0xf551
#define FSTV0913_RESET_UPKO_COUNT                                        0xf5510040

/*HYSTTHRESH*/
#define RSTV0913_HYSTTHRESH                                        0xf554
#define FSTV0913_DELIN_LOCKTHRES                                        0xf55400f0
#define FSTV0913_DELIN_UNLOCKTHRES                                        0xf554000f

/*ISIENTRY*/
#define RSTV0913_ISIENTRY                                        0xf55e
#define FSTV0913_ISI_ENTRY                                        0xf55e00ff

/*ISIBITENA*/
#define RSTV0913_ISIBITENA                                        0xf55f
#define FSTV0913_ISI_BIT_EN                                        0xf55f00ff

/*MATSTR1*/
#define RSTV0913_MATSTR1                                        0xf560
#define FSTV0913_MATYPE_CURRENT1                                        0xf56000ff

/*MATSTR0*/
#define RSTV0913_MATSTR0                                        0xf561
#define FSTV0913_MATYPE_CURRENT0                                        0xf56100ff

/*UPLSTR1*/
#define RSTV0913_UPLSTR1                                        0xf562
#define FSTV0913_UPL_CURRENT1                                        0xf56200ff

/*UPLSTR0*/
#define RSTV0913_UPLSTR0                                        0xf563
#define FSTV0913_UPL_CURRENT0                                        0xf56300ff

/*DFLSTR1*/
#define RSTV0913_DFLSTR1                                        0xf564
#define FSTV0913_DFL_CURRENT1                                        0xf56400ff

/*DFLSTR0*/
#define RSTV0913_DFLSTR0                                        0xf565
#define FSTV0913_DFL_CURRENT0                                        0xf56500ff

/*SYNCSTR*/
#define RSTV0913_SYNCSTR                                        0xf566
#define FSTV0913_SYNC_CURRENT                                        0xf56600ff

/*SYNCDSTR1*/
#define RSTV0913_SYNCDSTR1                                        0xf567
#define FSTV0913_SYNCD_CURRENT1                                        0xf56700ff

/*SYNCDSTR0*/
#define RSTV0913_SYNCDSTR0                                        0xf568
#define FSTV0913_SYNCD_CURRENT0                                        0xf56800ff

/*PDELSTATUS1*/
#define RSTV0913_PDELSTATUS1                                        0xf569
#define FSTV0913_PKTDELIN_DELOCK                                        0xf5690080
#define FSTV0913_SYNCDUPDFL_BADDFL                                        0xf5690040
#define FSTV0913_UNACCEPTED_STREAM                                        0xf5690010
#define FSTV0913_BCH_ERROR_FLAG                                        0xf5690008
#define FSTV0913_PKTDELIN_LOCK                                        0xf5690002
#define FSTV0913_FIRST_LOCK                                        0xf5690001

/*PDELSTATUS2*/
#define RSTV0913_PDELSTATUS2                                        0xf56a
#define FSTV0913_FRAME_MODCOD                                        0xf56a007c
#define FSTV0913_FRAME_TYPE                                        0xf56a0003

/*BBFCRCKO1*/
#define RSTV0913_BBFCRCKO1                                        0xf56b
#define FSTV0913_BBHCRC_KOCNT1                                        0xf56b00ff

/*BBFCRCKO0*/
#define RSTV0913_BBFCRCKO0                                        0xf56c
#define FSTV0913_BBHCRC_KOCNT0                                        0xf56c00ff

/*UPCRCKO1*/
#define RSTV0913_UPCRCKO1                                        0xf56d
#define FSTV0913_PKTCRC_KOCNT1                                        0xf56d00ff

/*UPCRCKO0*/
#define RSTV0913_UPCRCKO0                                        0xf56e
#define FSTV0913_PKTCRC_KOCNT0                                        0xf56e00ff

/*TSSTATEM*/
#define RSTV0913_TSSTATEM                                        0xf570
#define FSTV0913_TSDIL_ON                                        0xf5700080
#define FSTV0913_TSRS_ON                                        0xf5700020
#define FSTV0913_TSDESCRAMB_ON                                        0xf5700010
#define FSTV0913_TS_DISABLE                                        0xf5700004
#define FSTV0913_TSOUT_NOSYNC                                        0xf5700001

/*TSCFGH*/
#define RSTV0913_TSCFGH                                        0xf572
#define FSTV0913_TSFIFO_DVBCI                                        0xf5720080
#define FSTV0913_TSFIFO_SERIAL                                        0xf5720040
#define FSTV0913_TSFIFO_TEIUPDATE                                        0xf5720020
#define FSTV0913_TSFIFO_DUTY50                                        0xf5720010
#define FSTV0913_TSFIFO_HSGNLOUT                                        0xf5720008
#define FSTV0913_TSFIFO_ERRMODE                                        0xf5720006
#define FSTV0913_RST_HWARE                                        0xf5720001

/*TSCFGM*/
#define RSTV0913_TSCFGM                                        0xf573
#define FSTV0913_TSFIFO_MANSPEED                                        0xf57300c0
#define FSTV0913_TSFIFO_PERMDATA                                        0xf5730020
#define FSTV0913_TSFIFO_INVDATA                                        0xf5730001

/*TSCFGL*/
#define RSTV0913_TSCFGL                                        0xf574
#define FSTV0913_TSFIFO_BCLKDEL1CK                                        0xf57400c0
#define FSTV0913_BCHERROR_MODE                                        0xf5740030
#define FSTV0913_TSFIFO_NSGNL2DATA                                        0xf5740008
#define FSTV0913_TSFIFO_EMBINDVB                                        0xf5740004
#define FSTV0913_TSFIFO_BITSPEED                                        0xf5740003

/*TSINSDELH*/
#define RSTV0913_TSINSDELH                                        0xf576
#define FSTV0913_TSDEL_SYNCBYTE                                        0xf5760080
#define FSTV0913_TSDEL_XXHEADER                                        0xf5760040
#define FSTV0913_TSDEL_DATAFIELD                                        0xf5760010
#define FSTV0913_TSINSDEL_RSPARITY                                        0xf5760002
#define FSTV0913_TSINSDEL_CRC8                                        0xf5760001

/*TSDIVN*/
#define RSTV0913_TSDIVN                                        0xf579
#define FSTV0913_TSFIFO_SPEEDMODE                                        0xf57900c0

/*TSCFG4*/
#define RSTV0913_TSCFG4                                        0xf57a
#define FSTV0913_TSFIFO_TSSPEEDMODE                                        0xf57a00c0

/*TSSPEED*/
#define RSTV0913_TSSPEED                                        0xf580
#define FSTV0913_TSFIFO_OUTSPEED                                        0xf58000ff

/*TSSTATUS*/
#define RSTV0913_TSSTATUS                                        0xf581
#define FSTV0913_TSFIFO_LINEOK                                        0xf5810080
#define FSTV0913_TSFIFO_ERROR                                        0xf5810040
#define FSTV0913_DIL_READY                                        0xf5810001

/*TSSTATUS2*/
#define RSTV0913_TSSTATUS2                                        0xf582
#define FSTV0913_TSFIFO_DEMODSEL                                        0xf5820080
#define FSTV0913_TSFIFOSPEED_STORE                                        0xf5820040
#define FSTV0913_DILXX_RESET                                        0xf5820020
#define FSTV0913_SCRAMBDETECT                                        0xf5820002

/*TSBITRATE1*/
#define RSTV0913_TSBITRATE1                                        0xf583
#define FSTV0913_TSFIFO_BITRATE1                                        0xf58300ff

/*TSBITRATE0*/
#define RSTV0913_TSBITRATE0                                        0xf584
#define FSTV0913_TSFIFO_BITRATE0                                        0xf58400ff

/*ERRCTRL1*/
#define RSTV0913_ERRCTRL1                                        0xf598
#define FSTV0913_ERR_SOURCE1                                        0xf59800f0
#define FSTV0913_NUM_EVENT1                                        0xf5980007

/*ERRCNT12*/
#define RSTV0913_ERRCNT12                                        0xf599
#define FSTV0913_ERRCNT1_OLDVALUE                                        0xf5990080
#define FSTV0913_ERR_CNT12                                        0xf599007f

/*ERRCNT11*/
#define RSTV0913_ERRCNT11                                        0xf59a
#define FSTV0913_ERR_CNT11                                        0xf59a00ff

/*ERRCNT10*/
#define RSTV0913_ERRCNT10                                        0xf59b
#define FSTV0913_ERR_CNT10                                        0xf59b00ff

/*ERRCTRL2*/
#define RSTV0913_ERRCTRL2                                        0xf59c
#define FSTV0913_ERR_SOURCE2                                        0xf59c00f0
#define FSTV0913_NUM_EVENT2                                        0xf59c0007

/*ERRCNT22*/
#define RSTV0913_ERRCNT22                                        0xf59d
#define FSTV0913_ERRCNT2_OLDVALUE                                        0xf59d0080
#define FSTV0913_ERR_CNT22                                        0xf59d007f

/*ERRCNT21*/
#define RSTV0913_ERRCNT21                                        0xf59e
#define FSTV0913_ERR_CNT21                                        0xf59e00ff

/*ERRCNT20*/
#define RSTV0913_ERRCNT20                                        0xf59f
#define FSTV0913_ERR_CNT20                                        0xf59f00ff

/*FECSPY*/
#define RSTV0913_FECSPY                                        0xf5a0
#define FSTV0913_SPY_ENABLE                                        0xf5a00080
#define FSTV0913_SERIAL_MODE                                        0xf5a00020
#define FSTV0913_BERMETER_DATAMODE                                        0xf5a0000c
#define FSTV0913_BERMETER_LMODE                                        0xf5a00002
#define FSTV0913_BERMETER_RESET                                        0xf5a00001

/*FSPYCFG*/
#define RSTV0913_FSPYCFG                                        0xf5a1
#define FSTV0913_FECSPY_INPUT                                        0xf5a100c0
#define FSTV0913_RST_ON_ERROR                                        0xf5a10020
#define FSTV0913_ONE_SHOT                                        0xf5a10010
#define FSTV0913_I2C_MODE                                        0xf5a1000c
#define FSTV0913_SPY_HYSTERESIS                                        0xf5a10003

/*FSPYDATA*/
#define RSTV0913_FSPYDATA                                        0xf5a2
#define FSTV0913_SPY_STUFFING                                        0xf5a20080
#define FSTV0913_SPY_CNULLPKT                                        0xf5a20020
#define FSTV0913_SPY_OUTDATA_MODE                                        0xf5a2001f

/*FSPYOUT*/
#define RSTV0913_FSPYOUT                                        0xf5a3
#define FSTV0913_FSPY_DIRECT                                        0xf5a30080
#define FSTV0913_STUFF_MODE                                        0xf5a30007

/*FSTATUS*/
#define RSTV0913_FSTATUS                                        0xf5a4
#define FSTV0913_SPY_ENDSIM                                        0xf5a40080
#define FSTV0913_VALID_SIM                                        0xf5a40040
#define FSTV0913_FOUND_SIGNAL                                        0xf5a40020
#define FSTV0913_DSS_SYNCBYTE                                        0xf5a40010
#define FSTV0913_RESULT_STATE                                        0xf5a4000f

/*FBERCPT4*/
#define RSTV0913_FBERCPT4                                        0xf5a8
#define FSTV0913_FBERMETER_CPT4                                        0xf5a800ff

/*FBERCPT3*/
#define RSTV0913_FBERCPT3                                        0xf5a9
#define FSTV0913_FBERMETER_CPT3                                        0xf5a900ff

/*FBERCPT2*/
#define RSTV0913_FBERCPT2                                        0xf5aa
#define FSTV0913_FBERMETER_CPT2                                        0xf5aa00ff

/*FBERCPT1*/
#define RSTV0913_FBERCPT1                                        0xf5ab
#define FSTV0913_FBERMETER_CPT1                                        0xf5ab00ff

/*FBERCPT0*/
#define RSTV0913_FBERCPT0                                        0xf5ac
#define FSTV0913_FBERMETER_CPT0                                        0xf5ac00ff

/*FBERERR2*/
#define RSTV0913_FBERERR2                                        0xf5ad
#define FSTV0913_FBERMETER_ERR2                                        0xf5ad00ff

/*FBERERR1*/
#define RSTV0913_FBERERR1                                        0xf5ae
#define FSTV0913_FBERMETER_ERR1                                        0xf5ae00ff

/*FBERERR0*/
#define RSTV0913_FBERERR0                                        0xf5af
#define FSTV0913_FBERMETER_ERR0                                        0xf5af00ff

/*FSPYBER*/
#define RSTV0913_FSPYBER                                        0xf5b2
#define FSTV0913_FSPYBER_SYNCBYTE                                        0xf5b20010
#define FSTV0913_FSPYBER_UNSYNC                                        0xf5b20008
#define FSTV0913_FSPYBER_CTIME                                        0xf5b20007

/*SFECSTATUS*/
#define RSTV0913_SFECSTATUS                                        0xf5c3
#define FSTV0913_SFEC_ON                                        0xf5c30080
#define FSTV0913_SFEC_OFF                                        0xf5c30040
#define FSTV0913_LOCKEDSFEC                                        0xf5c30008
#define FSTV0913_SFEC_DELOCK                                        0xf5c30004
#define FSTV0913_SFEC_DEMODSEL                                        0xf5c30002
#define FSTV0913_SFEC_OVFON                                        0xf5c30001

/*SFKDIV12*/
#define RSTV0913_SFKDIV12                                        0xf5c4
#define FSTV0913_SFECKDIV12_MAN                                        0xf5c40080

/*SFKDIV23*/
#define RSTV0913_SFKDIV23                                        0xf5c5
#define FSTV0913_SFECKDIV23_MAN                                        0xf5c50080

/*SFKDIV34*/
#define RSTV0913_SFKDIV34                                        0xf5c6
#define FSTV0913_SFECKDIV34_MAN                                        0xf5c60080

/*SFKDIV56*/
#define RSTV0913_SFKDIV56                                        0xf5c7
#define FSTV0913_SFECKDIV56_MAN                                        0xf5c70080

/*SFKDIV67*/
#define RSTV0913_SFKDIV67                                        0xf5c8
#define FSTV0913_SFECKDIV67_MAN                                        0xf5c80080

/*SFKDIV78*/
#define RSTV0913_SFKDIV78                                        0xf5c9
#define FSTV0913_SFECKDIV78_MAN                                        0xf5c90080

/*SFSTATUS*/
#define RSTV0913_SFSTATUS                                        0xf5cc
#define FSTV0913_SFEC_LINEOK                                        0xf5cc0080
#define FSTV0913_SFEC_ERROR                                        0xf5cc0040
#define FSTV0913_SFEC_DATA7                                        0xf5cc0020
#define FSTV0913_SFEC_PKTDNBRFAIL                                        0xf5cc0010
#define FSTV0913_TSSFEC_DEMODSEL                                        0xf5cc0008
#define FSTV0913_SFEC_NOSYNC                                        0xf5cc0004
#define FSTV0913_SFEC_UNREGULA                                        0xf5cc0002
#define FSTV0913_SFEC_READY                                        0xf5cc0001

/*SFERRCTRL*/
#define RSTV0913_SFERRCTRL                                        0xf5d8
#define FSTV0913_SFEC_ERR_SOURCE                                        0xf5d800f0
#define FSTV0913_SFEC_NUM_EVENT                                        0xf5d80007

/*SFERRCNT2*/
#define RSTV0913_SFERRCNT2                                        0xf5d9
#define FSTV0913_SFERRC_OLDVALUE                                        0xf5d90080
#define FSTV0913_SFEC_ERR_CNT2                                        0xf5d9007f

/*SFERRCNT1*/
#define RSTV0913_SFERRCNT1                                        0xf5da
#define FSTV0913_SFEC_ERR_CNT1                                        0xf5da00ff

/*SFERRCNT0*/
#define RSTV0913_SFERRCNT0                                        0xf5db
#define FSTV0913_SFEC_ERR_CNT0                                        0xf5db00ff

/*TSGENERAL*/
#define RSTV0913_TSGENERAL                                        0xf630
#define FSTV0913_MUXSTREAM_OUTMODE                                        0xf6300008
#define FSTV0913_TSFIFO_PERMPARAL                                        0xf6300006

/*DISIRQCFG*/
#define RSTV0913_DISIRQCFG                                        0xf700
#define FSTV0913_ENRXEND                                        0xf7000040
#define FSTV0913_ENRXFIFO8B                                        0xf7000020
#define FSTV0913_ENTRFINISH                                        0xf7000010
#define FSTV0913_ENTIMEOUT                                        0xf7000008
#define FSTV0913_ENTXEND                                        0xf7000004
#define FSTV0913_ENTXFIFO64B                                        0xf7000002
#define FSTV0913_ENGAPBURST                                        0xf7000001

/*DISIRQSTAT*/
#define RSTV0913_DISIRQSTAT                                        0xf701
#define FSTV0913_IRQRXEND                                        0xf7010040
#define FSTV0913_IRQRXFIFO8B                                        0xf7010020
#define FSTV0913_IRQTRFINISH                                        0xf7010010
#define FSTV0913_IRQTIMEOUT                                        0xf7010008
#define FSTV0913_IRQTXEND                                        0xf7010004
#define FSTV0913_IRQTXFIFO64B                                        0xf7010002
#define FSTV0913_IRQGAPBURST                                        0xf7010001

/*DISTXCFG*/
#define RSTV0913_DISTXCFG                                        0xf702
#define FSTV0913_DISTX_RESET                                        0xf7020080
#define FSTV0913_TIM_OFF                                        0xf7020040
#define FSTV0913_TIM_CMD                                        0xf7020030
#define FSTV0913_ENVELOP                                        0xf7020008
#define FSTV0913_DIS_PRECHARGE                                        0xf7020004
#define FSTV0913_DISEQC_MODE                                        0xf7020003

/*DISTXSTATUS*/
#define RSTV0913_DISTXSTATUS                                        0xf703
#define FSTV0913_TX_FIFO_FULL                                        0xf7030040
#define FSTV0913_TX_IDLE                                        0xf7030020
#define FSTV0913_GAP_BURST                                        0xf7030010
#define FSTV0913_TX_FIFO64B                                        0xf7030008
#define FSTV0913_TX_END                                        0xf7030004
#define FSTV0913_TR_TIMEOUT                                        0xf7030002
#define FSTV0913_TR_FINISH                                        0xf7030001

/*DISTXBYTES*/
#define RSTV0913_DISTXBYTES                                        0xf704
#define FSTV0913_TXFIFO_BYTES                                        0xf70400ff

/*DISTXFIFO*/
#define RSTV0913_DISTXFIFO                                        0xf705
#define FSTV0913_DISEQC_TX_FIFO                                        0xf70500ff

/*DISTXF22*/
#define RSTV0913_DISTXF22                                        0xf706
#define FSTV0913_F22TX                                        0xf70600ff

/*DISTIMEOCFG*/
#define RSTV0913_DISTIMEOCFG                                        0xf708
#define FSTV0913_RXCHOICE                                        0xf7080006
#define FSTV0913_TIMEOUT_OFF                                        0xf7080001

/*DISTIMEOUT*/
#define RSTV0913_DISTIMEOUT                                        0xf709
#define FSTV0913_TIMEOUT_COUNT                                        0xf70900ff

/*DISRXCFG*/
#define RSTV0913_DISRXCFG                                        0xf70a
#define FSTV0913_EXTENVELOP                                        0xf70a0040
#define FSTV0913_PINSELECT                                        0xf70a0038
#define FSTV0913_IGNORE_SHORT22K                                        0xf70a0004
#define FSTV0913_SIGNED_RXIN                                        0xf70a0002
#define FSTV0913_DISRX_ON                                        0xf70a0001

/*DISRXSTAT1*/
#define RSTV0913_DISRXSTAT1                                        0xf70b
#define FSTV0913_RXEND                                        0xf70b0080
#define FSTV0913_RXACTIVE                                        0xf70b0040
#define FSTV0913_RXDETECT                                        0xf70b0020
#define FSTV0913_CONTTONE                                        0xf70b0010
#define FSTV0913_8BFIFOREADY                                        0xf70b0008
#define FSTV0913_FIFOEMPTY                                        0xf70b0004

/*DISRXSTAT0*/
#define RSTV0913_DISRXSTAT0                                        0xf70c
#define FSTV0913_RXFAIL                                        0xf70c0080
#define FSTV0913_FIFOPFAIL                                        0xf70c0040
#define FSTV0913_RXNONBYTE                                        0xf70c0020
#define FSTV0913_FIFOOVF                                        0xf70c0010
#define FSTV0913_SHORT22K                                        0xf70c0008
#define FSTV0913_RXMSGLOST                                        0xf70c0004

/*DISRXBYTES*/
#define RSTV0913_DISRXBYTES                                        0xf70d
#define FSTV0913_RXFIFO_BYTES                                        0xf70d001f

/*DISRXPARITY1*/
#define RSTV0913_DISRXPARITY1                                        0xf70e
#define FSTV0913_DISRX_PARITY1                                        0xf70e00ff

/*DISRXPARITY0*/
#define RSTV0913_DISRXPARITY0                                        0xf70f
#define FSTV0913_DISRX_PARITY0                                        0xf70f00ff

/*DISRXFIFO*/
#define RSTV0913_DISRXFIFO                                        0xf710
#define FSTV0913_DISEQC_RX_FIFO                                        0xf71000ff

/*DISRXDC1*/
#define RSTV0913_DISRXDC1                                        0xf711
#define FSTV0913_DC_VALUE1                                        0xf7110103

/*DISRXDC0*/
#define RSTV0913_DISRXDC0                                        0xf712
#define FSTV0913_DC_VALUE0                                        0xf71200ff

/*DISRXF221*/
#define RSTV0913_DISRXF221                                        0xf714
#define FSTV0913_F22RX1                                        0xf714000f

/*DISRXF220*/
#define RSTV0913_DISRXF220                                        0xf715
#define FSTV0913_F22RX0                                        0xf71500ff

/*DISRXF100*/
#define RSTV0913_DISRXF100                                        0xf716
#define FSTV0913_F100RX                                        0xf71600ff

/*DISRXSHORT22K*/
#define RSTV0913_DISRXSHORT22K                                        0xf71c
#define FSTV0913_SHORT22K_LENGTH                                        0xf71c001f

/*ACRPRESC*/
#define RSTV0913_ACRPRESC                                        0xf71e
#define FSTV0913_ACR_PRESC                                        0xf71e0007

/*ACRDIV*/
#define RSTV0913_ACRDIV                                        0xf71f
#define FSTV0913_ACR_DIV                                        0xf71f00ff

/*NBITER_NF4*/
#define RSTV0913_NBITER_NF4                                        0xfa03
#define FSTV0913_NBITER_NF_QPSK_1_2                                        0xfa0300ff

/*NBITER_NF5*/
#define RSTV0913_NBITER_NF5                                        0xfa04
#define FSTV0913_NBITER_NF_QPSK_3_5                                        0xfa0400ff

/*NBITER_NF6*/
#define RSTV0913_NBITER_NF6                                        0xfa05
#define FSTV0913_NBITER_NF_QPSK_2_3                                        0xfa0500ff

/*NBITER_NF7*/
#define RSTV0913_NBITER_NF7                                        0xfa06
#define FSTV0913_NBITER_NF_QPSK_3_4                                        0xfa0600ff

/*NBITER_NF8*/
#define RSTV0913_NBITER_NF8                                        0xfa07
#define FSTV0913_NBITER_NF_QPSK_4_5                                        0xfa0700ff

/*NBITER_NF9*/
#define RSTV0913_NBITER_NF9                                        0xfa08
#define FSTV0913_NBITER_NF_QPSK_5_6                                        0xfa0800ff

/*NBITER_NF10*/
#define RSTV0913_NBITER_NF10                                        0xfa09
#define FSTV0913_NBITER_NF_QPSK_8_9                                        0xfa0900ff

/*NBITER_NF11*/
#define RSTV0913_NBITER_NF11                                        0xfa0a
#define FSTV0913_NBITER_NF_QPSK_9_10                                        0xfa0a00ff

/*NBITER_NF12*/
#define RSTV0913_NBITER_NF12                                        0xfa0b
#define FSTV0913_NBITER_NF_8PSK_3_5                                        0xfa0b00ff

/*NBITER_NF13*/
#define RSTV0913_NBITER_NF13                                        0xfa0c
#define FSTV0913_NBITER_NF_8PSK_2_3                                        0xfa0c00ff

/*NBITER_NF14*/
#define RSTV0913_NBITER_NF14                                        0xfa0d
#define FSTV0913_NBITER_NF_8PSK_3_4                                        0xfa0d00ff

/*NBITER_NF15*/
#define RSTV0913_NBITER_NF15                                        0xfa0e
#define FSTV0913_NBITER_NF_8PSK_5_6                                        0xfa0e00ff

/*NBITER_NF16*/
#define RSTV0913_NBITER_NF16                                        0xfa0f
#define FSTV0913_NBITER_NF_8PSK_8_9                                        0xfa0f00ff

/*NBITER_NF17*/
#define RSTV0913_NBITER_NF17                                        0xfa10
#define FSTV0913_NBITER_NF_8PSK_9_10                                        0xfa1000ff

/*GAINLLR_NF4*/
#define RSTV0913_GAINLLR_NF4                                        0xfa43
#define FSTV0913_GAINLLR_NF_QPSK_1_2                                        0xfa43007f

/*GAINLLR_NF5*/
#define RSTV0913_GAINLLR_NF5                                        0xfa44
#define FSTV0913_GAINLLR_NF_QPSK_3_5                                        0xfa44007f

/*GAINLLR_NF6*/
#define RSTV0913_GAINLLR_NF6                                        0xfa45
#define FSTV0913_GAINLLR_NF_QPSK_2_3                                        0xfa45007f

/*GAINLLR_NF7*/
#define RSTV0913_GAINLLR_NF7                                        0xfa46
#define FSTV0913_GAINLLR_NF_QPSK_3_4                                        0xfa46007f

/*GAINLLR_NF8*/
#define RSTV0913_GAINLLR_NF8                                        0xfa47
#define FSTV0913_GAINLLR_NF_QPSK_4_5                                        0xfa47007f

/*GAINLLR_NF9*/
#define RSTV0913_GAINLLR_NF9                                        0xfa48
#define FSTV0913_GAINLLR_NF_QPSK_5_6                                        0xfa48007f

/*GAINLLR_NF10*/
#define RSTV0913_GAINLLR_NF10                                        0xfa49
#define FSTV0913_GAINLLR_NF_QPSK_8_9                                        0xfa49007f

/*GAINLLR_NF11*/
#define RSTV0913_GAINLLR_NF11                                        0xfa4a
#define FSTV0913_GAINLLR_NF_QPSK_9_10                                        0xfa4a007f

/*GAINLLR_NF12*/
#define RSTV0913_GAINLLR_NF12                                        0xfa4b
#define FSTV0913_GAINLLR_NF_8PSK_3_5                                        0xfa4b007f

/*GAINLLR_NF13*/
#define RSTV0913_GAINLLR_NF13                                        0xfa4c
#define FSTV0913_GAINLLR_NF_8PSK_2_3                                        0xfa4c007f

/*GAINLLR_NF14*/
#define RSTV0913_GAINLLR_NF14                                        0xfa4d
#define FSTV0913_GAINLLR_NF_8PSK_3_4                                        0xfa4d007f

/*GAINLLR_NF15*/
#define RSTV0913_GAINLLR_NF15                                        0xfa4e
#define FSTV0913_GAINLLR_NF_8PSK_5_6                                        0xfa4e007f

/*GAINLLR_NF16*/
#define RSTV0913_GAINLLR_NF16                                        0xfa4f
#define FSTV0913_GAINLLR_NF_8PSK_8_9                                        0xfa4f007f

/*GAINLLR_NF17*/
#define RSTV0913_GAINLLR_NF17                                        0xfa50
#define FSTV0913_GAINLLR_NF_8PSK_9_10                                        0xfa50007f

/*CFGEXT*/
#define RSTV0913_CFGEXT                                        0xfa80
#define FSTV0913_BYPBCH                                        0xfa800040
#define FSTV0913_BYPLDPC                                        0xfa800020
#define FSTV0913_SHORTMULT                                        0xfa800004

/*GENCFG*/
#define RSTV0913_GENCFG                                        0xfa86
#define FSTV0913_BROADCAST                                        0xfa860010

/*LDPCERR1*/
#define RSTV0913_LDPCERR1                                        0xfa96
#define FSTV0913_LDPC_ERRORS1                                        0xfa9600ff

/*LDPCERR0*/
#define RSTV0913_LDPCERR0                                        0xfa97
#define FSTV0913_LDPC_ERRORS0                                        0xfa9700ff

/*BCHERR*/
#define RSTV0913_BCHERR                                        0xfa98
#define FSTV0913_ERRORFLAG                                        0xfa980010
#define FSTV0913_BCH_ERRORS_COUNTER                                        0xfa98000f

/*MAXEXTRAITER*/
#define RSTV0913_MAXEXTRAITER                                        0xfab1
#define FSTV0913_MAX_EXTRA_ITER                                        0xfab100ff

/*STATUSITER*/
#define RSTV0913_STATUSITER                                        0xfabc
#define FSTV0913_STATUS_ITER                                        0xfabc00ff

/*STATUSMAXITER*/
#define RSTV0913_STATUSMAXITER                                        0xfabd
#define FSTV0913_STATUS_MAX_ITER                                        0xfabd00ff

/*TSTRES0*/
#define RSTV0913_TSTRES0                                        0xff11
#define FSTV0913_FRESFEC                                        0xff110080

/*TCTL4*/
#define RSTV0913_TCTL4                                        0xff48
#define FSTV0913_CFR2TOCFR1_DVBS1                                        0xff4800c0

#define STV0913_NBREGS		408
#define STV0913_NBFIELDS		711

/* functions --------------------------------------------------------------- */

/* create instance of STC register mappings */
STCHIP_Error_t STV0913_Init(Demod_InitParams_t *InitParams,STCHIP_Handle_t *hChip);  

#endif
//vim:ts=4
