/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/ioport.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>

#ifdef CONFIG_FELICA
#include <mach/hlte_felica_gpio.h>
#endif /* CONFIG_FELICA */
#define KS8851_IRQ_GPIO -1

#define GPIOMUX_SET_NC(n) \
	{ \
		.gpio	   = n, \
		.settings = { \
			[GPIOMUX_ACTIVE] = &gpio_suspend_config[2], \
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2], \
		}, \
	}

static struct gpiomux_setting mdm2ap_status_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting mdm2ap_errfatal_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting ap2mdm_soft_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct msm_gpiomux_config mdm_configs[] __initdata = {
	/* MDM2AP_STATUS */
	{
		.gpio = 46,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_SOFT_RESET, aka AP2MDM_PON_RESET_N */
	{
		.gpio = 24,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_soft_reset_cfg,
		}
	},
};

static struct gpiomux_setting gpio_uart_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting gpio_uart_config2 = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static struct gpiomux_setting gpio_eth_config = {
	.pull = GPIOMUX_PULL_UP,
	.drv = GPIOMUX_DRV_2MA,
	.func = GPIOMUX_FUNC_GPIO,
};

static struct gpiomux_setting gpio_spi_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_spi_cs3_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};

#if defined(CONFIG_SENSORS_SSP_STM)
static struct gpiomux_setting gpio_spi11_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_spi11_config2 = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_spi11_sus_config[] = {
	{
		.func = GPIOMUX_FUNC_1,
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_1,
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},
	{
		.func = GPIOMUX_FUNC_2,
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
};
#endif

static struct gpiomux_setting blsp7_i2c_config = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting gpio_i2c_func3_config = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config msm_eth_configs[] = {
	{
		.gpio = KS8851_IRQ_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
		}
	},
};
#endif

#if defined(CONFIG_SEC_LOCALE_KOR)
static struct gpiomux_setting gpio_spi_qup3_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif

static struct gpiomux_setting gpio_suspend_config[] = {
	{
		.func = GPIOMUX_FUNC_GPIO,  /* IN-NP */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_GPIO,  /* O-LOW */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},
	{
		.func = GPIOMUX_FUNC_GPIO,  /* IN-PD */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_IN,
	},
	{
		.func = GPIOMUX_FUNC_3,  /* FUNC-PD */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_IN,		
	},
};

#if !defined(CONFIG_SENSORS_SSP_STM)
static struct gpiomux_setting gpio_epm_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

//#ifdef CONFIG_MACH_KS01
static struct gpiomux_setting wcnss_5wire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wcnss_5wire_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};
//#endif

static struct gpiomux_setting gpio_i2c_config = {
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

#if !defined(CONFIG_BCM2079X_NFC_I2C) && !defined(CONFIG_NFC_PN547)
static struct gpiomux_setting nfc_i2c_config = {
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
};
#endif

static struct gpiomux_setting lcd_en_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting lcd_en_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ext_buck_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting ext_buck_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting taiko_reset = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting taiko_int = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting es325_reset = {
	.pull = GPIOMUX_PULL_NONE,
	.drv = GPIOMUX_DRV_2MA,
	.func = GPIOMUX_FUNC_GPIO,
	.dir = GPIOMUX_OUT_HIGH,
};

#if defined(CONFIG_BCM2079X_NFC_I2C)
static struct gpiomux_setting nfc_irq_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting nfc_firmware_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};
#elif defined(CONFIG_NFC_PN547)
static struct gpiomux_setting nfc_irq_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting nfc_firmware_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};
#endif

#if !defined(CONFIG_SENSORS_SSP)
static struct gpiomux_setting hap_lvl_shft_suspended_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hap_lvl_shft_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting cm36653_suspended_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting cm36653_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config hap_lvl_shft_config[] __initdata = {
	{
		.gpio = 86,
		.settings = {
#if defined(CONFIG_SENSORS_CM36653)
			[GPIOMUX_SUSPENDED] = &cm36653_suspended_config,
			[GPIOMUX_ACTIVE] = &cm36653_active_config,
#else
			[GPIOMUX_SUSPENDED] = &hap_lvl_shft_suspended_config,
			[GPIOMUX_ACTIVE] = &hap_lvl_shft_active_config,
#endif
		},
	},
};
#endif

static struct msm_gpiomux_config ext_buck_configs[] __initdata = {
	{
		.gpio      = 60,		/* Ext.BUCK SDA */
		.settings = {
			[GPIOMUX_ACTIVE] = &ext_buck_act_cfg,
			[GPIOMUX_SUSPENDED] = &ext_buck_sus_cfg,
		},
	},
	{
		.gpio      = 61,		/* Ext.BUCK SCL */
		.settings = {
			[GPIOMUX_ACTIVE] = &ext_buck_act_cfg,
			[GPIOMUX_SUSPENDED] = &ext_buck_sus_cfg,
		},
	},
};

static struct msm_gpiomux_config hw_rev_configs[] __initdata = {
	{
		.gpio      = 16,		/* HW_REV(0) */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_suspend_config[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio      = 14,		/* HW_REV(1) */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_suspend_config[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio      = 13,		/* HW_REV(2) */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_suspend_config[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio      = 90,		/* HW_REV(3) */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_suspend_config[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
};

/*
 * NC port for JVELTE Rev0.1
 */
static struct msm_gpiomux_config gpio_nc_configs[] __initdata = {
	GPIOMUX_SET_NC(8),
	GPIOMUX_SET_NC(9),
	GPIOMUX_SET_NC(10),
	GPIOMUX_SET_NC(11),
	GPIOMUX_SET_NC(18),
	GPIOMUX_SET_NC(23),
	GPIOMUX_SET_NC(24),
	GPIOMUX_SET_NC(25),
	GPIOMUX_SET_NC(26),
	GPIOMUX_SET_NC(28),
	GPIOMUX_SET_NC(29),
	GPIOMUX_SET_NC(30),
	GPIOMUX_SET_NC(41),
	GPIOMUX_SET_NC(42),
	GPIOMUX_SET_NC(45),
	GPIOMUX_SET_NC(46),
	GPIOMUX_SET_NC(47),
	GPIOMUX_SET_NC(48),
	GPIOMUX_SET_NC(49),
	GPIOMUX_SET_NC(50),
	GPIOMUX_SET_NC(53),
	GPIOMUX_SET_NC(54),
	GPIOMUX_SET_NC(57),
	GPIOMUX_SET_NC(58),
	GPIOMUX_SET_NC(64),
	GPIOMUX_SET_NC(65),
	GPIOMUX_SET_NC(66),
	GPIOMUX_SET_NC(67),
	GPIOMUX_SET_NC(68),
	GPIOMUX_SET_NC(69),
	GPIOMUX_SET_NC(73),
	GPIOMUX_SET_NC(75),
	GPIOMUX_SET_NC(76),
	GPIOMUX_SET_NC(79),
	GPIOMUX_SET_NC(80),
	GPIOMUX_SET_NC(81),
	GPIOMUX_SET_NC(85),
	GPIOMUX_SET_NC(91),
	GPIOMUX_SET_NC(94),
	GPIOMUX_SET_NC(95),
	GPIOMUX_SET_NC(96),
	GPIOMUX_SET_NC(100),
	GPIOMUX_SET_NC(101),
	GPIOMUX_SET_NC(108),
	GPIOMUX_SET_NC(120),
	GPIOMUX_SET_NC(121),
	GPIOMUX_SET_NC(122),
	GPIOMUX_SET_NC(124),
	GPIOMUX_SET_NC(125),
	GPIOMUX_SET_NC(129),
	GPIOMUX_SET_NC(130),
	GPIOMUX_SET_NC(131),
	GPIOMUX_SET_NC(132),
	GPIOMUX_SET_NC(135),
	GPIOMUX_SET_NC(136),
	GPIOMUX_SET_NC(137),
	GPIOMUX_SET_NC(142),
	GPIOMUX_SET_NC(143),
	GPIOMUX_SET_NC(144),
	GPIOMUX_SET_NC(145),
};

#if defined(CONFIG_BCM2079X_NFC_I2C) || defined(CONFIG_NFC_PN547)

static struct msm_gpiomux_config msm_nfc_configs[] __initdata = {
#if !defined(CONFIG_MACH_H3GDUOS)
	{
		.gpio      = 59,		/* NFC IRQ */
		.settings = {
			[GPIOMUX_ACTIVE] = &nfc_irq_cfg,
			[GPIOMUX_SUSPENDED] = &nfc_irq_cfg,
		},
	},
#else
	{
		.gpio      = 79,		/* NFC IRQ */
		.settings = {
			[GPIOMUX_ACTIVE] = &nfc_irq_cfg,
			[GPIOMUX_SUSPENDED] = &nfc_irq_cfg,
		},
	},
#endif
	{
		.gpio		= 63,		/* NFC FIRMWARE */
		.settings = {
			[GPIOMUX_ACTIVE] = &nfc_firmware_cfg,
			[GPIOMUX_SUSPENDED] = &nfc_firmware_cfg,
		},
	},
};
#endif

#if !defined(CONFIG_SENSORS_SSP)
static struct gpiomux_setting hsic_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting hsic_hub_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
#endif

#if !defined(CONFIG_SENSORS_SSP)
static struct msm_gpiomux_config msm_hsic_hub_configs[] = {
	{
		.gpio = 50,               /* HSIC_HUB_INT_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_hub_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
};
#endif

#if defined (CONFIG_VIDEO_MHL_V2) || defined (CONFIG_VIDEO_MHL_SII8246)
static struct gpiomux_setting mhl_suspend_cfg = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mhl_hpd_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting mhl_hpd_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

static struct gpiomux_setting hdmi_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hdmi_ddc_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hdmi_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting hdmi_active_2_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config msm_hdmi_configs[] __initdata = {
#ifndef CONFIG_VIDEO_MHL_V2
	{
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
#endif
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
};

static struct msm_gpiomux_config msm_hdmi_ddc_configs[] __initdata = {
	{
		.gpio = 32,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_ddc_suspend_cfg,
		},
	},
	{
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_ddc_suspend_cfg,
		},
	},
};

#if defined (CONFIG_VIDEO_MHL_V2) || defined (CONFIG_VIDEO_MHL_SII8246)
static struct msm_gpiomux_config mhl_configs[] __initdata = {
	{
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_hpd_active_cfg,
			[GPIOMUX_SUSPENDED] = &mhl_hpd_suspend_cfg,
		},
	},
	{
		.gpio      = 51, /* BLSP9 QUP I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &mhl_suspend_cfg,
		},
	},
	{
		.gpio      = 52, /* BLSP9 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &mhl_suspend_cfg,
		},
	},
};
#endif

#if !defined(CONFIG_BT_BCM4335) && !defined(CONFIG_BT_BCM4339)
static struct gpiomux_setting gpio_uart7_active_cfg = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_uart7_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config msm_blsp2_uart7_configs[] __initdata = {
	{
		.gpio	= 43,	/* BLSP2 UART7 CTS */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
	{
		.gpio	= 44,	/* BLSP2 UART7 RFR */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
};


static struct msm_gpiomux_config msm_rumi_blsp_configs[] __initdata = {
	{
		.gpio      = 45,	/* BLSP2 UART8 TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		.gpio      = 46,	/* BLSP2 UART8 RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
};
#endif

static struct msm_gpiomux_config msm_blsp_configs[] __initdata = {
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	{
		.gpio      = 0,		/* BLSP1 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs3_config,
		},
	},
	{
		.gpio      = 1,		/* BLSP1 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs3_config,
		},
	},
	{
		.gpio      = 3,		/* BLSP1 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs3_config,
		},
	},
	{
		.gpio      = 2,		/* BLSP1 QUP SPI_CS1 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs3_config,
		},
	},
#endif
#if defined(CONFIG_MACH_H3GDUOS)
	{
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_en_act_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_en_sus_cfg,
		},
	},
#else
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_en_act_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_en_sus_cfg,
		},
	},
#endif
	{
		.gpio      = 6,		/* BLSP1 QUP2 I2C_DAT */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_i2c_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[3],
		},
	},
	{
		.gpio      = 7,		/* BLSP1 QUP2 I2C_CLK */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_i2c_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[3],
		},
	},
#if defined(CONFIG_SEC_LOCALE_KOR)
	{
		.gpio	   = 8,		/* BLSP1 QUP3 SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_qup3_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio	   = 9,		/* BLSP1 QUP3 SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_qup3_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio	   = 10,	/* BLSP1 QUP3 SPI_CS0_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_qup3_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio	   = 11,	/* BLSP1 QUP3 SPI_CLK */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_qup3_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
	{
		.gpio      = 43,		/* BLSP7 QUP2 I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &blsp7_i2c_config,
		},
	},
	{
		.gpio      = 44,		/* BLSP7 QUP2 I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &blsp7_i2c_config,
		},
	},
#if !defined(CONFIG_BT_BCM4335) && !defined(CONFIG_BT_BCM4339)
	{
		.gpio      = 47,		/* BLSP7 QUP2 I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 48,		/* BLSP7 QUP2 I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#endif
#ifdef CONFIG_USB_SWITCH_TSU6721
	{
		.gpio      = 25,        /* BLSP1 QUP5 I2C_DAT */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_i2c_func4_config,
			[GPIOMUX_SUSPENDED] = &gpio_i2c_func4_config,
		},
	},
	{
		.gpio      = 26,        /* BLSP1 QUP5 I2C_CLK */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_i2c_func5_config,
			[GPIOMUX_SUSPENDED] = &gpio_i2c_func5_config,
		},
	},
#endif
#if !defined(CONFIG_SENSORS_SSP_STM)
	{
		.gpio      = 83,		/* BLSP11 QUP I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 84,		/* BLSP11 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#endif
	{
		.gpio      = 4,			/* BLSP2 UART TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config2,
		},
	},
	{
		.gpio      = 5,			/* BLSP2 UART RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config2,
		},
	},
	{
		.gpio      = 53,		/* BLSP2 QUP4 SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio      = 54,		/* BLSP2 QUP4 SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#if defined(CONFIG_BCM2079X_NFC_I2C) || defined(CONFIG_NFC_PN547)
	{
		.gpio      = 55, /* BLSP10 QUP I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 56, /* BLSP10 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#else
	{
		.gpio      = 56,		/* BLSP2 QUP4 SPI_CLK */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		.gpio      = 55,		/* BLSP2 QUP4 SPI_CS0_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
#endif
#if defined(CONFIG_SENSORS_SSP_STM)
	{
		.gpio      = 81,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi11_config2,
			[GPIOMUX_SUSPENDED] = &gpio_spi11_sus_config[2],
		},
	},
	{
		.gpio      = 82,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi11_config2,
			[GPIOMUX_SUSPENDED] = &gpio_spi11_sus_config[2],
		},
	},
	{
		.gpio      = 83,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi11_config,
			[GPIOMUX_SUSPENDED] = &gpio_spi11_sus_config[1],
		},
	},
	{
		.gpio      = 84,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi11_config,
			[GPIOMUX_SUSPENDED] = &gpio_spi11_sus_config[0],
		},
	},
#endif
#if !defined(CONFIG_SENSORS_SSP_STM)
	{
		.gpio      = 81,		/* EPM enable */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_epm_config,
		},
	},
#endif
};

static struct msm_gpiomux_config msm8974_slimbus_config[] __initdata = {
	{
		.gpio	= 70,		/* slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio	= 71,		/* slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};

static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_1, /*active 1*/ /* 0 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_1, /*suspend*/ /* 1 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},

	{
		.func = GPIOMUX_FUNC_1, /*i2c suspend*/ /* 2 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_KEEPER,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 0*/ /* 3 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend 0*/ /* 4 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
};

static struct gpiomux_setting sd_card_det_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting sd_card_det_sleep_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config sd_card_det __initdata = {
	.gpio = 62,
	.settings = {
		[GPIOMUX_ACTIVE]    = &sd_card_det_active_config,
		[GPIOMUX_SUSPENDED] = &sd_card_det_sleep_config,
	},
};

static struct msm_gpiomux_config msm_sensor_configs[] __initdata = {
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
#if 0
	{
		.gpio = 16, /* CAM_MCLK1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
#endif
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
#if !defined(CONFIG_MFD_MAX77803)
	{
		.gpio = 27, /* OIS_SYNC */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
	{
		.gpio	   = 29,	/* REV0.1 AF SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_func3_config,
		},
	},
	{
		.gpio	   = 30,	/* REV0.1 AF SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_func3_config,
		},
	},

	{
		.gpio	= 41,	/* REV0.0 AF SDA */
		.settings = {
			[GPIOMUX_ACTIVE]	= &gpio_suspend_config[2],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		.gpio	= 42,	/* REV0.0 AF SCL */
		.settings = {
			[GPIOMUX_ACTIVE]	= &gpio_suspend_config[2],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},

#if !defined(CONFIG_BCM2079X_NFC_I2C) && !defined(CONFIG_NFC_PN547)
	{
		.gpio = 77, /* NFC SDA */
		.settings = {
			[GPIOMUX_ACTIVE] = &nfc_i2c_config,
			[GPIOMUX_SUSPENDED] = &nfc_i2c_config,
		},
	},
	{
		.gpio = 78, /* NFC SCL */
		.settings = {
			[GPIOMUX_ACTIVE] = &nfc_i2c_config,
			[GPIOMUX_SUSPENDED] = &nfc_i2c_config,
		},
	},
#endif
#if !defined(CONFIG_SENSORS_SSP)
	{
		.gpio = 89, /* CAM1_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
#if !defined(CONFIG_SENSORS_SSP)
	{
		.gpio = 91, /* CAM2_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
	{
		.gpio = 92, /* CAM2_RST_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
};

static struct gpiomux_setting pri_auxpcm_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct gpiomux_setting pri_auxpcm_sus_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config msm8974_pri_auxpcm_configs[] __initdata = {
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_SUSPENDED] = &pri_auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &pri_auxpcm_act_cfg,
		},
	},
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_SUSPENDED] = &pri_auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &pri_auxpcm_act_cfg,
		},
	},
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_SUSPENDED] = &pri_auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &pri_auxpcm_act_cfg,
		},
	},
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_SUSPENDED] = &pri_auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &pri_auxpcm_act_cfg,
		},
	},
};

#ifdef CONFIG_PCM_ROUTE_VOICE_STUB
static struct gpiomux_setting sec_auxpcm_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct gpiomux_setting sec_auxpcm_sus_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sec_auxpcm_act_pu_cfg = {  // should be set PU because of SC6500 setting.
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};


static struct gpiomux_setting sec_auxpcm_sus_pu_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8974_sec_auxpcm_configs[] __initdata = {
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_SUSPENDED] = &sec_auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &sec_auxpcm_act_cfg,
		},
	},
	{
		.gpio = 59,
		.settings = {
			[GPIOMUX_SUSPENDED] = &sec_auxpcm_sus_pu_cfg,
			[GPIOMUX_ACTIVE] = &sec_auxpcm_act_pu_cfg,
		},
	},
	{
		.gpio = 60,
		.settings = {
			[GPIOMUX_SUSPENDED] = &sec_auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &sec_auxpcm_act_cfg,
		},
	},
	{
		.gpio = 61,
		.settings = {
			[GPIOMUX_SUSPENDED] = &sec_auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &sec_auxpcm_act_cfg,
		},
	},
};
#endif /* CONFIG_PCM_ROUTE_VOICE_STUB */


//#ifdef CONFIG_MACH_KS01
static struct msm_gpiomux_config wcnss_5wire_interface[] = {
	{
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
};
//#endif

static struct msm_gpiomux_config es325_config[] __initdata = {
	{
		.gpio	= 94,		/* ES325_RESET */
		.settings = {
			[GPIOMUX_SUSPENDED] = &es325_reset,
		},
	},
	{
		.gpio	= 76,		/* ES325_INT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
};

#ifdef CONFIG_MACH_H3GDUOS
static struct msm_gpiomux_config msm_taiko_config[] __initdata = {
	{
		.gpio	= 110,		/* SYS_RST_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_reset,
		},
	},
	{
		.gpio	= 72,		/* CDC_INT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_int,
		},
	},
};
#else
static struct msm_gpiomux_config msm_taiko_config[] __initdata = {
	{
		.gpio	= 78,		/* SYS_RST_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_reset,
		},
	},
	{
		.gpio	= 72,		/* CDC_INT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_int,
		},
	},
};
#endif /* CONFIG_MACH_H3GDUOS */

#ifdef CONFIG_LEDS_AN30259A
	static struct gpiomux_setting an30259a_i2c_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config an30259a_led_config[] __initdata = {
	{
		.gpio	= 104,		/* SVC_LED_SCL */
		.settings = {
			[GPIOMUX_ACTIVE] = &an30259a_i2c_config,
		},
	},
	{
		.gpio	= 105,		/* SVC_LED_SDA */
		.settings = {
			[GPIOMUX_ACTIVE] = &an30259a_i2c_config,
		},
	},
};
#if defined(CONFIG_MACH_KS01SKT) || defined(CONFIG_MACH_KS01KTT)\
	|| defined(CONFIG_MACH_KS01LGT)
static struct msm_gpiomux_config an30259a_led_config_rev7[] __initdata = {
		{
			.gpio	= 32,		/* SVC_LED_SCL */
			.settings = {
				[GPIOMUX_ACTIVE] = &an30259a_i2c_config,
			},
		},
		{
			.gpio	= 33,		/* SVC_LED_SDA */
			.settings = {
				[GPIOMUX_ACTIVE] = &an30259a_i2c_config,
			},
		},
	};


#endif

#endif

#if defined(CONFIG_BCM4335) || defined(CONFIG_BCM4335_MODULE) || defined(CONFIG_BCM4339) || defined(CONFIG_BCM4339_MODULE)

/* MSM8974 WLAN_HOST_WAKE GPIO Number */
#define GPIO_WL_HOST_WAKE 54

static struct gpiomux_setting wlan_host_wakeup_setting[] = {
	{
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
};

static struct msm_gpiomux_config wlan_host_wakeup_configs[] __initdata = {
	{
		.gpio = GPIO_WL_HOST_WAKE,
		.settings = {
			[GPIOMUX_ACTIVE] = &wlan_host_wakeup_setting[0],
			[GPIOMUX_SUSPENDED] = &wlan_host_wakeup_setting[0],
		}
	},
};

static void msm_gpiomux_wlan_host_wakeup_install(void)
{
	msm_gpiomux_install(wlan_host_wakeup_configs,
				ARRAY_SIZE(wlan_host_wakeup_configs));
}
#endif /* defined(CONFIG_BCM4335) || defined(CONFIG_BCM4335_MODULE) || defined(CONFIG_BCM4339) || defined(CONFIG_BCM4339_MODULE) */

#if defined(CONFIG_SENSORS_SSP)
static struct gpiomux_setting ssp_setting[] = {
	{
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_UP,
	},
	{
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},
};


static struct msm_gpiomux_config ssp_configs[] __initdata = {
	{
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE] = &ssp_setting[1],
			[GPIOMUX_SUSPENDED] = &ssp_setting[1],
		},
	},
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE] = &ssp_setting[1],
			[GPIOMUX_SUSPENDED] = &ssp_setting[1],
		},
	},
	{
		.gpio = 86,
		.settings = {
			[GPIOMUX_ACTIVE] = &ssp_setting[1],
			[GPIOMUX_SUSPENDED] = &ssp_setting[1],
		},
	},
	{
		.gpio = 89,
		.settings = {
			[GPIOMUX_ACTIVE] = &ssp_setting[1],
			[GPIOMUX_SUSPENDED] = &ssp_setting[1],
		},
	},
};
#endif

#if 0 //def CONFIG_MMC_MSM_SDC3_SUPPORT
static struct gpiomux_setting sdc3_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc3_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc3_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc3_clk_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config msm8974_sdc3_configs[] __initdata = {
	{
		/* DAT3 */
		.gpio      = 35,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* DAT2 */
		.gpio      = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* DAT1 */
		.gpio      = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* DAT0 */
		.gpio      = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* CMD */
		.gpio      = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* CLK */
		.gpio      = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_clk_suspend_cfg,
		},
	},
};

#if 0
static void msm_gpiomux_sdc3_install(void)
{
	msm_gpiomux_install(msm8974_sdc3_configs,
			    ARRAY_SIZE(msm8974_sdc3_configs));
}
#endif
#else
static void msm_gpiomux_sdc3_install(void) {}
#endif /* CONFIG_MMC_MSM_SDC3_SUPPORT */

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct gpiomux_setting sdc4_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc4_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc4_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc4_data_1_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8974_sdc4_configs[] __initdata = {
	{
		/* DAT3 */
		.gpio      = 92,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
	{
		/* DAT2 */
		.gpio      = 94,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
	{
		/* DAT1 */
		.gpio      = 95,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_data_1_suspend_cfg,
		},
	},
	{
		/* DAT0 */
		.gpio      = 96,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
#if !defined(CONFIG_SENSORS_SSP)
	{
		/* CMD */
		.gpio      = 91,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
#endif
	{
		/* CLK */
		.gpio      = 93,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
};

static void msm_gpiomux_sdc4_install(void)
{
	msm_gpiomux_install(msm8974_sdc4_configs,
			    ARRAY_SIZE(msm8974_sdc4_configs));
}
#else
static void msm_gpiomux_sdc4_install(void) {}
#endif /* CONFIG_MMC_MSM_SDC4_SUPPORT */

static struct msm_gpiomux_config apq8074_dragonboard_ts_config[] __initdata = {
	{
		/* BLSP1 QUP I2C_DATA */
		.gpio      = 2,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		/* BLSP1 QUP I2C_CLK */
		.gpio      = 3,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
};

#if defined(CONFIG_BT_BCM4335) || defined(CONFIG_BT_BCM4339)
static struct msm_gpiomux_config msm8974_btuart_configs[] __initdata = {
	{
		/* TXD */
		.gpio      = 45,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		/* RXD */
		.gpio      = 46,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		/* CTS */
		.gpio      = 47,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		/* RTS */
		.gpio      = 48,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
};

static void msm_gpiomux_btuart_install(void)
{
	msm_gpiomux_install(msm8974_btuart_configs,
			    ARRAY_SIZE(msm8974_btuart_configs));
}
#endif

#ifdef CONFIG_SENSORS_HALL
static struct gpiomux_setting gpio_hall_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_hall_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
    .dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config msm8974_hall_configs[] __initdata = {
	{
		.gpio      = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_hall_active_config,
			[GPIOMUX_SUSPENDED] = &gpio_hall_suspend_config,
		},
	},
};
#endif

#ifdef CONFIG_W1_SLAVE_DS28EL15
static struct gpiomux_setting gpio_cover_id_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct msm_gpiomux_config msm8974_cover_id_config[] __initdata = {
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_cover_id_config,
			[GPIOMUX_SUSPENDED] = &gpio_cover_id_config,
		},
	},
};
#endif

static struct gpiomux_setting gpio_audio_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_audio_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_audio_pullnone_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config msm8974_audio_configs[] __initdata = {
	{
		.gpio      = 76,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_audio_active_config,
			[GPIOMUX_SUSPENDED] = &gpio_audio_suspend_config,
		},
	},
	{
		.gpio      = 77,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_audio_pullnone_config,
			[GPIOMUX_SUSPENDED] = &gpio_audio_pullnone_config,
		},
	},
};

#ifdef CONFIG_USB_SWITCH_FSA9485
static struct gpiomux_setting muic_init_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PUll_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting muic_sleep_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PUll_DOWN,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config msm8974_muic_configs[] __initdata = {
	{
		.gpio = 32, /* IF_PMIC_SCL_1.8V */
		.settings = {
			[GPIOMUX_ACTIVE] = &muic_init_cfg,
			[GPIOMUX_SUSPENDED] = &muic_sleep_cfg,
		},
	},
	{
		.gpio = 33, /* IF_PMIC_SDA_1.8V */
		.settings = {
			[GPIOMUX_ACTIVE] = &muic_init_cfg,
			[GPIOMUX_SUSPENDED] = &muic_sleep_cfg,
		},                                            		
	},
};
#endif


#if defined(CONFIG_SENSORS_VFS61XX)
static struct gpiomux_setting gpio_spi_btp_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_spi_btp_clk_config = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_spi_btp_irq_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_spi_btp_rst_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config msm8974_fingerprint_configs[] __initdata = {
	{
		/* MOSI */
		.gpio = 23,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_config,
		},
	},
	{
		/* MISO */
		.gpio = 24,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_config,
		},
	},
	{
		/* CS */
		.gpio = 25,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_config,
		},
	},
	{
		/* CLK  */
		.gpio = 26,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_clk_config,
		},
	},
};
static struct msm_gpiomux_config msm8974_fingerprint_rev03_configs[] __initdata = {
	{
		/* BTP_RST_N */
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_rst_config,
		},
	},
	{
		/* BTP_INT */
		.gpio = 64,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_btp_irq_config,
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_irq_config,
		},
	},
	{
		/* BTP_LDO */
		.gpio = 130,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_rst_config,
		},
	},

};

static struct msm_gpiomux_config msm8974_fingerprint_rev04_configs[] __initdata = {
	{
		/* BTP_RST_N */
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_rst_config,
		},
	},
	{
		/* BTP_INT */
		.gpio = 64,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_btp_irq_config,
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_irq_config,
		},
	},
	{
		/* BTP_LDO */
		.gpio = 130,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_btp_rst_config,
			[GPIOMUX_ACTIVE] = &gpio_spi_btp_rst_config,
		},
	},

};

#endif

#ifdef CONFIG_INPUT_WACOM
static struct gpiomux_setting gpio_wacom_sleep_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_wacom_active_config = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_wacom_irq_sleep_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_wacom_irq_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config msm8974_wacom_configs[] __initdata = {
	{
		/* PEN_SDA_1.8V */
		.gpio = 29,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_wacom_sleep_config,
			[GPIOMUX_ACTIVE] = &gpio_wacom_active_config,
		},
	},
	{
		/* PEN_SCL_1.8V */
		.gpio = 30,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_wacom_sleep_config,
			[GPIOMUX_ACTIVE] = &gpio_wacom_active_config,
		},
	},
	{
		/* PEN_IRQ_1.8V */
		.gpio = 144,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_wacom_irq_sleep_config,
		},
	},
	{
		/* PEN_PDCT_1.8V */
		.gpio = 145,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_wacom_irq_sleep_config,
			[GPIOMUX_ACTIVE] = &gpio_wacom_irq_active_config,
		},
	},

};
#endif

#if defined(CONFIG_GSM_MODEM_SPRD6500)
static struct msm_gpiomux_config msm8974_sc6500_spi_configs[] __initdata = {
	{
		/* SPI_MOSI */
		.gpio      = 8,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		/* SPI_MISO */
		.gpio      = 9,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		/* SPI_CS */
		.gpio      = 10,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
	{
		/* SPI_CLK */
		.gpio      = 11,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[2],
		},
	},
};

static void msm_gpiomux_sc6500_spi_install(void)
{
	msm_gpiomux_install(msm8974_sc6500_spi_configs,
			    ARRAY_SIZE(msm8974_sc6500_spi_configs));
}
#endif
#ifdef CONFIG_FELICA

/* USE "GPIO_SHARED_I2C_SCL/SDA" 
 * ("GPIO_MHL_SCL/SDA" sets initial configuration)
 */
static struct gpiomux_setting felica_i2c_sda_setting = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting felica_i2c_scl_setting = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting felica_uart_tx_setting = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting felica_uart_rx_setting = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting felica_rfs_setting = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting felica_intu_setting = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting felica_hsel_setting = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting felica_pon_setting = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config msm8974_felica_configs[] __initdata = {
	{
		.gpio = GPIO_FELICA_I2C_SDA,/*I2C_SDA*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_i2c_sda_setting,
			[GPIOMUX_SUSPENDED] = &felica_i2c_sda_setting,
		},
	},
	{
		.gpio = GPIO_FELICA_I2C_SCL,/*I2C_SCL*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_i2c_scl_setting,
			[GPIOMUX_SUSPENDED] = &felica_i2c_scl_setting,
		},
	},
	{
		.gpio = GPIO_FELICA_UART_TX,/*2-pin UART_TX*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_uart_tx_setting,
			[GPIOMUX_SUSPENDED] = &felica_uart_tx_setting,
		},
	},
	{
		.gpio = GPIO_FELICA_UART_RX,/*2-pin UART_RX*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_uart_rx_setting,
			[GPIOMUX_SUSPENDED] = &felica_uart_rx_setting,
		},
	},
	{
		.gpio = GPIO_FELICA_RFS,/*RFS*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_rfs_setting,
			[GPIOMUX_SUSPENDED] = &felica_rfs_setting,
		},
	},
	{
		.gpio = GPIO_FELICA_INTU,/*INTU*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_intu_setting,
			[GPIOMUX_SUSPENDED] = &felica_intu_setting,
		},
	},
	{
		.gpio = GPIO_FELICA_HSEL,/*HSEL*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_hsel_setting,
			[GPIOMUX_SUSPENDED] = &felica_hsel_setting,
		},
	},
	{
		.gpio = GPIO_FELICA_PON,/*PON*/
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_pon_setting,
			[GPIOMUX_SUSPENDED] = &felica_pon_setting,
		},
	},
	
};
#endif /* CONFIG_FELICA */

static struct gpiomux_setting gpio_pcd_int_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config msm8974_pcd_int_configs[] __initdata = {
	{
		.gpio = 100,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_pcd_int_config,
			[GPIOMUX_ACTIVE] = &gpio_pcd_int_config,
		},
	},
};

extern unsigned int system_rev;

void __init msm_8974_init_gpiomux(void)
{
	int rc;

	rc = msm_gpiomux_init_dt();
	if (rc) {
		pr_err("%s failed %d\n", __func__, rc);
		return;
	}

	pr_err("%s:%d socinfo_get_version %x\n", __func__, __LINE__,
	socinfo_get_version());
	msm_tlmm_misc_reg_write(TLMM_SPARE_REG, 0x5);

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	msm_gpiomux_install(msm_eth_configs, ARRAY_SIZE(msm_eth_configs));
#endif
	msm_gpiomux_install(msm_blsp_configs, ARRAY_SIZE(msm_blsp_configs));

#if !defined(CONFIG_BT_BCM4335) && !defined(CONFIG_BT_BCM4339)
	msm_gpiomux_install(msm_blsp2_uart7_configs, ARRAY_SIZE(msm_blsp2_uart7_configs));
#endif

#if defined(CONFIG_BT_BCM4335) || defined(CONFIG_BT_BCM4339)
	msm_gpiomux_btuart_install();
#endif

	msm_gpiomux_install(msm8974_slimbus_config,
			ARRAY_SIZE(msm8974_slimbus_config));

	msm_gpiomux_install(ext_buck_configs, ARRAY_SIZE(ext_buck_configs));
	msm_gpiomux_install(hw_rev_configs, ARRAY_SIZE(hw_rev_configs));
	if(system_rev >= 8)
		msm_gpiomux_install(gpio_nc_configs,
					ARRAY_SIZE(gpio_nc_configs));
#if !defined(CONFIG_SENSORS_SSP)
	msm_gpiomux_install(hap_lvl_shft_config,
				ARRAY_SIZE(hap_lvl_shft_config));
#endif

	msm_gpiomux_install(msm_sensor_configs, ARRAY_SIZE(msm_sensor_configs));

	msm_gpiomux_install(&sd_card_det, 1);

	msm_gpiomux_install(wcnss_5wire_interface,
				ARRAY_SIZE(wcnss_5wire_interface));

	msm_gpiomux_sdc3_install();
	msm_gpiomux_sdc4_install();

	msm_gpiomux_install(msm_taiko_config, ARRAY_SIZE(msm_taiko_config));
	msm_gpiomux_install(es325_config, ARRAY_SIZE(es325_config));

#if !defined(CONFIG_SENSORS_SSP)
	msm_gpiomux_install(msm_hsic_hub_configs,
				ARRAY_SIZE(msm_hsic_hub_configs));
#endif
#if defined(CONFIG_SENSORS_SSP)
	msm_gpiomux_install(ssp_configs,
				ARRAY_SIZE(ssp_configs));
#endif

	msm_gpiomux_install(msm_hdmi_configs, ARRAY_SIZE(msm_hdmi_configs));

	if (system_rev > 5)
		msm_gpiomux_install(msm_hdmi_ddc_configs,
				ARRAY_SIZE(msm_hdmi_ddc_configs));
#if defined (CONFIG_VIDEO_MHL_V2) || defined (CONFIG_VIDEO_MHL_SII8246)
	if(system_rev > 1)
		msm_gpiomux_install(mhl_configs, ARRAY_SIZE(mhl_configs));
#endif
#if defined(CONFIG_BCM2079X_NFC_I2C) || defined(CONFIG_NFC_PN547)
	msm_gpiomux_install(msm_nfc_configs,
	ARRAY_SIZE(msm_nfc_configs));
#endif

#if defined(CONFIG_BCM4335) || defined(CONFIG_BCM4335_MODULE) || defined(CONFIG_BCM4339) || defined(CONFIG_BCM4339_MODULE)
	msm_gpiomux_wlan_host_wakeup_install();
#endif /* defined(CONFIG_BCM4335) || defined(CONFIG_BCM4335_MODULE) || defined(CONFIG_BCM4339) || defined(CONFIG_BCM4339_MODULE) */

#if defined(CONFIG_LEDS_AN30259A)
#if defined(CONFIG_MACH_KS01SKT) || defined(CONFIG_MACH_KS01KTT)\
		|| defined(CONFIG_MACH_KS01LGT)

	if (system_rev < 7) {
		msm_gpiomux_install(an30259a_led_config,
				ARRAY_SIZE(an30259a_led_config));
	}
	else
	{
		msm_gpiomux_install(an30259a_led_config_rev7,
				ARRAY_SIZE(an30259a_led_config_rev7));
	}
#else
	msm_gpiomux_install(an30259a_led_config,
				ARRAY_SIZE(an30259a_led_config));
#endif
#endif

	msm_gpiomux_install(msm8974_pri_auxpcm_configs,
				 ARRAY_SIZE(msm8974_pri_auxpcm_configs));
#ifdef CONFIG_PCM_ROUTE_VOICE_STUB
	msm_gpiomux_install(msm8974_sec_auxpcm_configs,
			 ARRAY_SIZE(msm8974_sec_auxpcm_configs));
#endif /* CONFIG_PCM_ROUTE_VOICE_STUB */

#if !defined(CONFIG_BT_BCM4335) && !defined(CONFIG_BT_BCM4339)
	if (of_board_is_rumi())
		msm_gpiomux_install(msm_rumi_blsp_configs,
				    ARRAY_SIZE(msm_rumi_blsp_configs));
#endif

#if defined(CONFIG_GSM_MODEM_SPRD6500)
	msm_gpiomux_sc6500_spi_install();
#endif

#ifdef CONFIG_SENSORS_HALL
	msm_gpiomux_install(msm8974_hall_configs, ARRAY_SIZE(msm8974_hall_configs));
#endif

#ifdef CONFIG_W1_SLAVE_DS28EL15
	msm_gpiomux_install(msm8974_cover_id_config, ARRAY_SIZE(msm8974_cover_id_config));
#endif
	msm_gpiomux_install(msm8974_audio_configs, ARRAY_SIZE(msm8974_audio_configs));

#ifdef CONFIG_USB_SWITCH_FSA9485
	msm_gpio_mux_install(msm8974_muic_configs, ARRAY_SIZE(msm8974_muic_configs));
#endif

#ifdef CONFIG_SENSORS_VFS61XX
	msm_gpiomux_install(msm8974_fingerprint_configs,
		ARRAY_SIZE(msm8974_fingerprint_configs));
	if (system_rev > 3)
		msm_gpiomux_install(msm8974_fingerprint_rev04_configs,
			ARRAY_SIZE(msm8974_fingerprint_rev04_configs));
	else if(system_rev > 2)
		msm_gpiomux_install(msm8974_fingerprint_rev03_configs,
			ARRAY_SIZE(msm8974_fingerprint_rev03_configs));
#endif

#ifdef CONFIG_INPUT_WACOM
	msm_gpiomux_install(msm8974_wacom_configs,
		ARRAY_SIZE(msm8974_wacom_configs));
#endif

	if (socinfo_get_platform_subtype() == PLATFORM_SUBTYPE_MDM)
		msm_gpiomux_install(mdm_configs,
			ARRAY_SIZE(mdm_configs));

	if (of_board_is_dragonboard() && machine_is_apq8074())
		msm_gpiomux_install(apq8074_dragonboard_ts_config,
			ARRAY_SIZE(apq8074_dragonboard_ts_config));


#ifdef CONFIG_FELICA
	msm_gpiomux_install(msm8974_felica_configs,
			ARRAY_SIZE(msm8974_felica_configs));
#endif /* CONFIG_FELICA */

	msm_gpiomux_install(msm8974_pcd_int_configs, ARRAY_SIZE(msm8974_pcd_int_configs));
}
