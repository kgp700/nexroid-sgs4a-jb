/*
 * driver/barcode_emul Barcode emulator driver
 *
 * Copyright (C) 2012 Samsung Electronics
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/earlysuspend.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/firmware.h>
#include <linux/barcode_emul.h>
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#if defined(CONFIG_IR_REMOCON_FPGA)
#include <linux/ir_remote_con.h>
#endif
#include "barcode_emul_ice4_hlte.h"
#include <linux/err.h>
#include <mach/gpiomux.h>

#if defined(TEST_DEBUG)
#define pr_barcode	pr_emerg
#else
#define pr_barcode	pr_info
#endif

#define LOOP_BACK	48
#define TEST_CODE1	49
#define TEST_CODE2	50
#define FW_VER_ADDR	0x80
#define BEAM_STATUS_ADDR	0xFE
#define SEC_FPGA_MAX_FW_PATH	255
#define SEC_FPGA_FW_FILENAME		"i2c_top_bitmap.bin"

#define BARCODE_I2C_ADDR	0x6C
#define FIRMWARE_MAX_RETRY	2
#if defined(CONFIG_MACH_H3GDUOS)
#define GPIO_FPGA_MAIN_CLK 78
#else
#define GPIO_FPGA_MAIN_CLK 58
#endif

#define MSM_SEC_FPGA_I2C_BUS_ID 13

#if defined(CONFIG_IR_REMOCON_FPGA)
#define IRDA_I2C_ADDR		0x50
#define IRDA_TEST_CODE_SIZE	140
#define IRDA_TEST_CODE_ADDR	0x00
#define MAX_SIZE		2048
#define READ_LENGTH	8
#endif

#define BOARD_REV02 2
#define BOARD_REV03 3
#define BOARD_REV07 3
#define TIME_LIMIT_MSEC 300
#define tm(time) (u32)ktime_to_us(time)

extern int system_rev;

struct barcode_emul_data {
	struct i2c_client		*client;
	struct workqueue_struct		*firmware_dl;
	struct delayed_work		fw_dl;
	const struct firmware		*fw;
#if defined(CONFIG_IR_REMOCON_FPGA)
	struct mutex			mutex;
	struct {
		unsigned char		addr;
		unsigned char		data[MAX_SIZE];
	} i2c_block_transfer;
	struct mc96_platform_data	*pdata;
	int				length;
	int				count;
	int				dev_id;
	int				ir_freq;
	int				ir_sum;
	int				on_off;
#endif
};

#if defined(CONFIG_IR_REMOCON_FPGA)
static int ack_number;
static int count_number;
#endif
static struct barcode_emul_platform_data *g_pdata;
static int Is_clk_enabled;
static int enable_counte;
static int Is_beaming;
static struct mutex		en_mutex;
static struct i2c_client *g_client;
#if defined(CONFIG_MACH_HLTESKT)||defined(CONFIG_MACH_HLTEKTT)||defined(CONFIG_MACH_HLTELGT)\
	|| defined(CONFIG_MACH_FLTESKT) || defined(CONFIG_MACH_LT03SKT) || defined(CONFIG_MACH_LT03LGT) || defined(CONFIG_MACH_LT03KTT)\
	|| defined(CONFIG_MACH_HLTEDCM) || defined(CONFIG_MACH_HLTEKDI) || defined(CONFIG_MACH_JS01LTEDCM)\
	|| defined(CONFIG_MACH_H3GDUOS_CTC) || defined(CONFIG_MACH_H3GDUOS_CU)
bool fw_dl_complete;
#else
static bool fw_dl_complete;
#endif


static struct regulator *barcode_l19_3p3=NULL;

static int bc_poweron(bool enable)
{
	int ret;
	struct device dev;

	dev = g_client->dev;

	if(enable)
	{
		static int check=1;
		if(check)
		{
			barcode_l19_3p3 = regulator_get(&dev, "vdd");

			if (IS_ERR(barcode_l19_3p3)) {
				pr_err("%s: could not get vdda vreg, rc=%ld\n",
					__func__, PTR_ERR(barcode_l19_3p3));
				return PTR_ERR(barcode_l19_3p3);
			}
			check=0;
		}

		ret = regulator_set_voltage(barcode_l19_3p3,
			3300000, 3300000);
		if (ret)
			pr_err("%s: error vreg_l19 set voltage ret=%d\n",
				__func__, ret);

		ret = regulator_enable(barcode_l19_3p3);
		if (ret)
			pr_err("%s: error l19 enabling regulator\n", __func__);

		printk("l19 enabled \n");
	}
	else
	{
		ret = regulator_disable(barcode_l19_3p3);
		if (ret)
			pr_err("%s: error l19 enabling regulator\n", __func__);

		printk("l19 disabled \n");
	}
	return 0;
}

static int ice4_clock_en(int onoff)
{
	static struct clk *fpga_main_src_clk;
	static struct clk *fpga_main_clk;
	
#if defined(CONFIG_MACH_H3GDUOS)
     if (onoff) {
     msm_tlmm_misc_reg_write(TLMM_SPARE_REG, 0x1);
     } else {
      msm_tlmm_misc_reg_write(TLMM_SPARE_REG, 0x5);
     }  
      if (!fpga_main_src_clk){
      	fpga_main_src_clk = clk_get(NULL, "gp1_src_clk");
      }
      if (IS_ERR(fpga_main_src_clk)) {
      	pr_err( "%s: unable to get fpga_main_src_clk\n", __func__);
      }
      
      if (!fpga_main_clk){
      	fpga_main_clk = clk_get(NULL, "gp1_clk");
      }
      if (IS_ERR(fpga_main_clk)) {
      	pr_err( "%s: unable to get fpga_main_clk\n", __func__);
      }
	
#else	
	if (!fpga_main_src_clk){
		fpga_main_src_clk = clk_get(NULL, "gp2_src_clk");
	}
	if (IS_ERR(fpga_main_src_clk)) {
   		pr_err( "%s: unable to get fpga_main_src_clk\n", __func__);
   	}

	if (!fpga_main_clk){
		fpga_main_clk = clk_get(NULL, "gp2_clk");
	}
	if (IS_ERR(fpga_main_clk)) {
   		pr_err( "%s: unable to get fpga_main_clk\n", __func__);
   	}
#endif
	if (onoff) {
		clk_set_rate(fpga_main_src_clk, 24000000);
		clk_prepare_enable(fpga_main_clk);
	} else {
		clk_disable_unprepare(fpga_main_clk);
		clk_put(fpga_main_src_clk);
		clk_put(fpga_main_clk);
		fpga_main_src_clk = NULL;
		fpga_main_clk = NULL;
	}	
	return 0;
}

static void fpga_enable(int enable_clk,int enable_rst_n)
{
	int ret;
	if (enable_clk) {
		if (!Is_clk_enabled && (enable_counte ==0)) {
			mutex_lock(&en_mutex);
			ret = ice4_clock_en(1);
			if(enable_rst_n)
				gpio_set_value(g_pdata->rst_n, GPIO_LEVEL_LOW);
			usleep_range(1000, 2000);
			Is_clk_enabled = 1;
		}
		enable_counte++;
	} else {
		if (Is_clk_enabled && !Is_beaming && (enable_counte==1)) {
			Is_clk_enabled = 0;
			usleep_range(2000, 2500);
			gpio_set_value(g_pdata->rst_n, GPIO_LEVEL_HIGH);
			ret = ice4_clock_en(0);
			mutex_unlock(&en_mutex);
		}
		if(enable_counte<0){
			printk(KERN_ERR "%s enable_counte ERR!= %d\n",__func__,enable_counte);
			enable_counte =0;
		}else{
			enable_counte--;
		}
	}
}

#ifdef CONFIG_OF

static int barcode_parse_dt(struct device *dev,
			struct barcode_emul_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	/* reset, irq gpio info */
	pdata->cresetb = of_get_named_gpio_flags(np, "barcode,cresetb",
				0, &pdata->cresetb_flag);
	pdata->rst_n = of_get_named_gpio_flags(np, "barcode,reset_n",
		0, &pdata->rst_n_flag);
	pdata->spi_clk =of_get_named_gpio_flags(np, "barcode,scl-gpio",
				0, &pdata->spi_clk_flag);		
	pdata->spi_si =of_get_named_gpio_flags(np, "barcode,sda-gpio",
				0, &pdata->spi_si_flag);
	pdata->irda_irq =of_get_named_gpio_flags(np, "barcode,irq-gpio",
				0, &pdata->irda_irq_flag);

	return 0;
}
#else
static int barcode_parse_dt(struct device *dev,
			struct cypress_touchkey_platform_data *pdata)
{
	return -ENODEV;
}
#endif


static void barcode_gpio_config(void)
{
	pr_info("%s\n", __func__);
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_si, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_clk, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);
	gpio_tlmm_config(GPIO_CFG(GPIO_FPGA_MAIN_CLK, \
		2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);	

	gpio_request(g_pdata->cresetb, "irda_creset");
	gpio_direction_output(g_pdata->cresetb, 0);

	gpio_request(g_pdata->rst_n, "irda_rst_n");
	gpio_direction_output(g_pdata->rst_n, 0);

	gpio_request(g_pdata->irda_irq, "irda_irq");
	gpio_direction_input(g_pdata->irda_irq);

	

}
#if defined(CONFIG_MACH_HLTESKT) || defined(CONFIG_MACH_HLTEKTT) || defined(CONFIG_MACH_HLTELGT)\
	|| defined(CONFIG_MACH_FLTESKT) || defined(CONFIG_MACH_LT03SKT) || defined(CONFIG_MACH_LT03LGT) || defined(CONFIG_MACH_LT03KTT)\
	|| defined(CONFIG_MACH_HLTEDCM) || defined(CONFIG_MACH_HLTEKDI) || defined(CONFIG_MACH_JS01LTEDCM)
static void barcode_gpio_reconfig(void)
{
	pr_info("%s\n", __func__);
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_si, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_clk, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA), 1);
	gpio_tlmm_config(GPIO_CFG(GPIO_FPGA_MAIN_CLK, \
		2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);	
		
	gpio_tlmm_config(GPIO_CFG(g_pdata->cresetb,0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_direction_output(g_pdata->cresetb, 0);
	
	gpio_tlmm_config(GPIO_CFG(g_pdata->rst_n,0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);	
	gpio_direction_output(g_pdata->rst_n, 0);
	
	gpio_tlmm_config(GPIO_CFG(g_pdata->irda_irq,0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_direction_input(g_pdata->irda_irq);

	mdelay(5);
}
#endif
/*
 * Send barcode emulator firmware data thougth spi communication
 */
static int barcode_send_firmware_data(const u8 *data, int len)
{
	unsigned int i, j;
	unsigned char spibit;

	i = 0;
	while (i < len) {
		j = 0;
		spibit = data[i];
		while (j < 8) {
			gpio_set_value_cansleep(g_pdata->spi_clk,
						GPIO_LEVEL_LOW);

			if (spibit & 0x80)
				gpio_set_value_cansleep(g_pdata->spi_si,
						GPIO_LEVEL_HIGH);
			else
				gpio_set_value_cansleep(g_pdata->spi_si,
						GPIO_LEVEL_LOW);

			j = j+1;
			gpio_set_value_cansleep(g_pdata->spi_clk,
						GPIO_LEVEL_HIGH);
			spibit = spibit<<1;
		}
		i = i+1;
	}

	i = 0;
	while (i < 200) {
		gpio_set_value_cansleep(g_pdata->spi_clk, GPIO_LEVEL_LOW);
		i = i+1;
		gpio_set_value_cansleep(g_pdata->spi_clk, GPIO_LEVEL_HIGH);
	}
	return 0;
}

static int barcode_fpga_fimrware_update_start(const u8 *data, int len)
{
	int retry = FIRMWARE_MAX_RETRY;
	pr_barcode("%s\n", __func__);

	fpga_enable(1,0);
	do {
		gpio_set_value(g_pdata->rst_n, GPIO_LEVEL_LOW);
		gpio_set_value(g_pdata->cresetb, GPIO_LEVEL_LOW);
		usleep_range(30, 50);

		gpio_set_value(g_pdata->cresetb, GPIO_LEVEL_HIGH);
		usleep_range(1000, 1300);

		barcode_send_firmware_data(data, len);
		usleep_range(50, 70);

		udelay(5);
		pr_barcode("FPGA firmware update success\n");
		fw_dl_complete = true;
		break;		
		
	} while (retry);
	fpga_enable(0,0);
	return 0;
}

void ice4_fpga_firmware_update_hlte(void)
{
#if defined(CONFIG_MACH_HLTESKT) || defined(CONFIG_MACH_HLTEKTT) || defined(CONFIG_MACH_HLTELGT)\
	|| defined(CONFIG_MACH_FLTESKT) || defined(CONFIG_MACH_LT03SKT) || defined(CONFIG_MACH_LT03LGT) || defined(CONFIG_MACH_LT03KTT)\
	|| defined(CONFIG_MACH_HLTEDCM) || defined(CONFIG_MACH_HLTEKDI) || defined(CONFIG_MACH_JS01LTEDCM)
	barcode_gpio_reconfig();
#endif
	if (g_pdata->fw_type == ICE_I2C_2) {
		barcode_fpga_fimrware_update_start(spiword_i2c_2,
						sizeof(spiword_i2c_2));
	}
	else if (g_pdata->fw_type == ICE_I2C_R2) {
		barcode_fpga_fimrware_update_start(spiword_i2c_r2,
						sizeof(spiword_i2c_r2));
	}
	else if (g_pdata->fw_type == ICE_I2C_R3) {
		barcode_fpga_fimrware_update_start(spiword_i2c_r3,
						sizeof(spiword_i2c_r3));
	}

	//verification with dummy gpio
#if defined(CONFIG_MACH_HLTESKT) || defined(CONFIG_MACH_HLTEKTT) || defined(CONFIG_MACH_HLTELGT)\
	|| defined(CONFIG_MACH_FLTESKT) || defined(CONFIG_MACH_LT03SKT) || defined(CONFIG_MACH_LT03LGT) || defined(CONFIG_MACH_LT03KTT)\
	|| defined(CONFIG_MACH_HLTEDCM) || defined(CONFIG_MACH_HLTEKDI) || defined(CONFIG_MACH_JS01LTEDCM)
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_si, 0,
		GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_clk, 0,
		GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA), 1);
#else
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_si, 0,
		GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);
	gpio_tlmm_config(GPIO_CFG(g_pdata->spi_clk, 0,
		GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);
#endif
	usleep_range(10000, 12000);
	
}

static ssize_t barcode_emul_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int ret;
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	pr_barcode("%s start\n", __func__);
	
	fpga_enable(1,1);

	client->addr = BARCODE_I2C_ADDR;
	ret = i2c_master_send(client, buf, size);
	if (ret < 0) {
		pr_err("%s: i2c err1 %d\n", __func__, ret);
		ret = i2c_master_send(client, buf, size);
		if (ret < 0)
			pr_err("%s: i2c err2 %d\n", __func__, ret);
	}
	pr_barcode("%s complete\n", __func__);
	if ((buf[0] == 0xFF) && (buf[1] != STOP_BEAMING)) {
		pr_barcode("%s BEAMING START\n", __func__);
		Is_beaming = BEAMING_ON;
	} else if ((buf[0] == 0xFF) && (buf[1] == STOP_BEAMING)) {
		pr_barcode("%s BEAMING STOP\n", __func__);
		Is_beaming = BEAMING_OFF;
	}

	fpga_enable(0,0);

	return size;
}

static ssize_t barcode_emul_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	return strlen(buf);
}
static DEVICE_ATTR(barcode_send, 0664, barcode_emul_show, barcode_emul_store);

static ssize_t barcode_emul_fw_update_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	if (request_firmware(&data->fw, SEC_FPGA_FW_FILENAME,
							dev)) {
		pr_err("%s: Can't open firmware file\n",
						__func__);
		goto firmwareload_fail;
	}
	barcode_fpga_fimrware_update_start(data->fw->data, data->fw->size);
	release_firmware(data->fw);
firmwareload_fail:
	return size;
}

static ssize_t barcode_emul_fw_update_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	return strlen(buf);
}
static DEVICE_ATTR(barcode_fw_update, 0664,
		barcode_emul_fw_update_show,
		barcode_emul_fw_update_store);

static int barcode_emul_read(struct i2c_client *client, u16 addr,
		u8 length, u8 *value)
{

	struct i2c_msg msg[2];
	int ret;

	pr_info("client address before read %u \n",client->addr);
	*value = 0;
	client->addr = BARCODE_I2C_ADDR;

	msg[0].addr  = client->addr;
	msg[0].flags = 0x00;
	msg[0].len   = 1;
	msg[0].buf   = (u8 *) &addr;

	msg[1].addr  = client->addr;
	msg[1].flags = I2C_M_RD | I2C_CLIENT_PEC;
	msg[1].len   = length;
	msg[1].buf   = (u8 *) value;

	pr_info("fw version before read %u \n",*value);
	fpga_enable(1,1);

	ret = i2c_transfer(client->adapter, msg, 2);
	if  (ret != 2) {
		pr_barcode("%s: err1 %d\n", __func__, ret);
		ret = i2c_transfer(client->adapter, msg, 2);
		if (ret != 2)
		{
			pr_barcode("%s: err2 %d\n", __func__, ret);
			fpga_enable(0,0);
			return -ret;
		}else {
			fpga_enable(0,0);
			return 0;
		}
	} else {
		fpga_enable(0,0);
		return 0;
	}
}

static ssize_t barcode_emul_test_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	int ret, i;
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	struct {
			unsigned char addr;
			unsigned char data[20];
		} i2c_block_transfer;
	unsigned char barcode_data[14] = {0xFF, 0xAC, 0xDB, 0x36, 0x42, 0x85,
			0x0A, 0xA8, 0xD1, 0xA3, 0x46, 0xC5, 0xDA, 0xFF};

	client->addr = BARCODE_I2C_ADDR;

	fpga_enable(1,1);

	if (buf[0] == LOOP_BACK) {
		for (i = 0; i < size; i++)
			i2c_block_transfer.data[i] = *buf++;

		i2c_block_transfer.addr = 0x01;
		pr_barcode("%s: write addr: %d, value: %d\n", __func__,
			i2c_block_transfer.addr, i2c_block_transfer.data[0]);

		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 2);
		if (ret < 0) {
			pr_barcode("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 2);
			if (ret < 0)
				pr_barcode("%s: err2 %d\n", __func__, ret);
		}

	} else if (buf[0] == TEST_CODE1) {
		unsigned char BSR_data[6] =\
			{0xC8, 0x00, 0x32, 0x01, 0x00, 0x32};

		pr_barcode("barcode test code start\n");

		/* send NH */
		i2c_block_transfer.addr = 0x80;
		i2c_block_transfer.data[0] = 0x05;
		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 2);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 2);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

		/* setup BSR data */
		for (i = 0; i < 6; i++)
			i2c_block_transfer.data[i+1] = BSR_data[i];

		/* send BSR1 */
		/* NS 1= 200, ISD 1 = 100us, IPD 1 = 200us,
		   BL 1=14, BW 1=4MHZ */
		i2c_block_transfer.addr = 0x81;
		i2c_block_transfer.data[0] = 0x00;

		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 8);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 8);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

		/* send BSR2 */
		/* NS 2= 200, ISD 2 = 100us, IPD 2 = 200us,
		   BL 2=14, BW 2=2MHZ*/
		i2c_block_transfer.addr = 0x88;
		i2c_block_transfer.data[0] = 0x01;

		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 8);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 8);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}


		/* send BSR3 */
		/* NS 3= 200, ISD 3 = 100us, IPD 3 = 200us,
		   BL 3=14, BW 3=1MHZ*/
		i2c_block_transfer.addr = 0x8F;
		i2c_block_transfer.data[0] = 0x02;

		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 8);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 8);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

		/* send BSR4 */
		/* NS 4= 200, ISD 4 = 100us, IPD 4 = 200us,
		   BL 4=14, BW 4=500KHZ*/
		i2c_block_transfer.addr = 0x96;
		i2c_block_transfer.data[0] = 0x04;

		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 8);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 8);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

		/* send BSR5 */
		/* NS 5= 200, ISD 5 = 100us, IPD 5 = 200us,
		   BL 5=14, BW 5=250KHZ*/
		i2c_block_transfer.addr = 0x9D;
		i2c_block_transfer.data[0] = 0x08;

		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 8);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 8);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

		/* send barcode data */
		i2c_block_transfer.addr = 0x00;
		for (i = 0; i < 14; i++)
			i2c_block_transfer.data[i] = barcode_data[i];

		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 15);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 15);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

		/* send START */
		i2c_block_transfer.addr = 0xFF;
		i2c_block_transfer.data[0] = 0x0E;
		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 2);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 2);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

	} else if (buf[0] == TEST_CODE2) {
		pr_barcode("barcode test code stop\n");
		i2c_block_transfer.addr = 0xFF;
		i2c_block_transfer.data[0] = 0x00;
		ret = i2c_master_send(client,
			(unsigned char *) &i2c_block_transfer, 2);
		if (ret < 0) {
			pr_err("%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				(unsigned char *) &i2c_block_transfer, 2);
			if (ret < 0)
				pr_err("%s: err2 %d\n", __func__, ret);
		}

	fpga_enable(0,0);
	}
	return size;
}

static ssize_t barcode_emul_test_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	return strlen(buf);
}
static DEVICE_ATTR(barcode_test_send, 0664,
		barcode_emul_test_show, barcode_emul_test_store);

static ssize_t barcode_ver_check_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	u8 fw_ver;

	barcode_emul_read(data->client, FW_VER_ADDR, 1, &fw_ver);
	pr_info("Actual value read f/w %u \n",fw_ver);
	fw_ver = (fw_ver >> 5) & 0x7;

	return snprintf(buf, sizeof(buf), "%u\n", fw_ver+14);
}

static DEVICE_ATTR(barcode_ver_check, 0664, barcode_ver_check_show, NULL);

static ssize_t barcode_led_status_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	u8 status;
	barcode_emul_read(data->client, BEAM_STATUS_ADDR, 1, &status);
	status = status & 0x1;
	return snprintf(buf, sizeof(buf), "%d\n", status);
}
static DEVICE_ATTR(barcode_led_status, 0664, barcode_led_status_show, NULL);

#if defined(CONFIG_IR_REMOCON_FPGA)
#ifndef CONFIG_CRC_REPEAT_FW
static void irda_add_checksum_length(struct barcode_emul_data *ir_data,
				int count)
{
	struct barcode_emul_data *data = ir_data;
	int i = 0, csum = 0;

	pr_barcode("%s: length: %04x\n", __func__, count);

	data->i2c_block_transfer.data[0] = count >> 8;
	data->i2c_block_transfer.data[1] = count & 0xff;

	while (i < count) {
		csum += data->i2c_block_transfer.data[i];
		i++;
	}

	pr_barcode("%s: checksum: %04x\n", __func__, csum);

	data->i2c_block_transfer.data[count] = csum >> 8;
	data->i2c_block_transfer.data[count+1] = csum & 0xff;
}
#endif
/* sysfs node ir_send */
static void ir_remocon_work(struct barcode_emul_data *ir_data, int count)
{

	struct barcode_emul_data *data = ir_data;
	struct i2c_client *client = data->client;

	int buf_size = count+2;
	int ret;
//	int sleep_timing;
//	int end_data;
	int actual_time;
	int emission_time;
	int ack_pin_onoff;
	ktime_t t1,t2;

	if (count_number >= 100)
		count_number = 0;

	count_number++;

	pr_barcode("%s: total buf_size: %d\n", __func__, buf_size);

	bc_poweron(1);
	fpga_enable(1,1);

	mutex_lock(&data->mutex);

	client->addr = IRDA_I2C_ADDR;

	if (g_pdata->fw_type) {
		data->i2c_block_transfer.addr = 0x00;
		data->i2c_block_transfer.data[0] = count >> 8;
		data->i2c_block_transfer.data[1] = count & 0xff;
		buf_size++;
		ret = i2c_master_send(client,
			(unsigned char *) &(data->i2c_block_transfer),
								buf_size);
		if (ret < 0) {
			dev_err(&client->dev, "%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
			(unsigned char *) &(data->i2c_block_transfer),
								buf_size);
			if (ret < 0)
				dev_err(&client->dev, "%s: err2 %d\n",
								__func__, ret);
		}
	} else {
		irda_add_checksum_length(data, count);
		ret = i2c_master_send(client, data->i2c_block_transfer.data,
								buf_size);
		if (ret < 0) {
			dev_err(&client->dev, "%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
				data->i2c_block_transfer.data, buf_size);
			if (ret < 0)
				dev_err(&client->dev, "%s: err2 %d\n",
								__func__, ret);
		}
	}
	usleep_range(10000, 12000);

	ack_pin_onoff = 0;
/* Unavailable in Lattice */
	if (gpio_get_value(g_pdata->irda_irq)) {
		pr_barcode("%s : %d Checksum NG!\n",
			__func__, count_number);
		ack_pin_onoff = 1;
	} else {
		pr_barcode("%s : %d Checksum OK!\n",
			__func__, count_number);
		ack_pin_onoff = 2;
	}
	ack_number = ack_pin_onoff;

	mutex_unlock(&data->mutex);

/*
	for (int i = 0; i < buf_size; i++) {
		printk(KERN_INFO "%s: data[%d] : 0x%02x\n", __func__, i,
				data->i2c_block_transfer.data[i]);
	}
*/
	data->count = 2;
#if 0
	end_data = data->i2c_block_transfer.data[count-2] << 8
		| data->i2c_block_transfer.data[count-1];

	emission_time = \
		(1000 * (data->ir_sum - end_data) / (data->ir_freq)) + 10;
	sleep_timing = emission_time - 130;
	if (sleep_timing > 0)
		msleep(sleep_timing);
#endif
/*
	printk(KERN_INFO "%s: sleep_timing = %d\n", __func__, sleep_timing);
*/
	emission_time = \
		(1000 * (data->ir_sum) / (data->ir_freq));
/*	if (emission_time > 0)
		msleep(emission_time);
*/
	actual_time = 0;
	t1 = ktime_get();
	while((gpio_get_value(g_pdata->irda_irq) == 0) && (actual_time <= emission_time))
	{
		int diff;
		t2 = ktime_get();
		diff = (tm(t2) - tm(t1))/1000;
		msleep(10);
		actual_time += 10;
		if(diff > TIME_LIMIT_MSEC)
			break;
	}
		pr_barcode("%s: emission_time = %d\n",
					__func__, emission_time);

	if (gpio_get_value(g_pdata->irda_irq)) {
		pr_barcode("%s : %d Sending IR OK!\n",
				__func__, count_number);
		ack_pin_onoff = 4;
	} else {
		pr_barcode("%s : %d Sending IR NG!\n",
				__func__, count_number);
		ack_pin_onoff = 2;
	}

	ack_number += ack_pin_onoff;
#ifndef USE_STOP_MODE
	data->on_off = 0;
#endif
	data->ir_freq = 0;
	data->ir_sum = 0;

	fpga_enable(0,0);
	bc_poweron(0);

}

static ssize_t remocon_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	unsigned int _data;
	int count, i;

	pr_barcode("ir_send called\n");

	for (i = 0; i < MAX_SIZE; i++) {
		if (sscanf(buf++, "%u", &_data) == 1) {
			if (_data == 0 || buf == '\0')
				break;
			if (data->count == 2) {
				data->ir_freq = _data;
				if (data->on_off) {
				//	msleep(30);
				} else {
				//	msleep(60);
					data->on_off = 1;
				}
				data->i2c_block_transfer.data[2]
								= _data >> 16;
				data->i2c_block_transfer.data[3]
							= (_data >> 8) & 0xFF;
				data->i2c_block_transfer.data[4]
								= _data & 0xFF;
				data->count += 3;
			} else {
				data->ir_sum += _data;
				count = data->count;
				data->i2c_block_transfer.data[count]
								= _data >> 8;
				data->i2c_block_transfer.data[count+1]
								= _data & 0xFF;
				data->count += 2;
			}

			while (_data > 0) {
				buf++;
				_data /= 10;
			}
		} else {
			break;
		}
	}

	ir_remocon_work(data, data->count);
	return size;
}

static ssize_t remocon_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	int i;
	char *bufp = buf;

	for (i = 5; i < MAX_SIZE - 1; i++) {
		if (data->i2c_block_transfer.data[i] == 0
			&& data->i2c_block_transfer.data[i+1] == 0)
			break;
		else
			bufp += sprintf(bufp, "%u,",
					data->i2c_block_transfer.data[i]);
	}
	return strlen(buf);
}

/* sysfs node ir_send_result */
static ssize_t remocon_ack(struct device *dev, struct device_attribute *attr,
		char *buf)
{

	pr_barcode("%s : ack_number = %d\n", __func__, ack_number);

	if (ack_number == 6)
		return sprintf(buf, "1\n");
	else
		return sprintf(buf, "0\n");
}

static int irda_read_device_info(struct barcode_emul_data *ir_data)
{
	struct barcode_emul_data *data = ir_data;
	struct i2c_client *client = data->client;
	u8 buf_ir_test[8];
	int ret;

	pr_barcode("%s called\n", __func__);

	msleep(60);

	fpga_enable(1,1);

	client->addr = IRDA_I2C_ADDR;
	ret = i2c_master_recv(client, buf_ir_test, READ_LENGTH);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	pr_barcode("%s: buf_ir dev_id: 0x%02x, 0x%02x\n", __func__,
			buf_ir_test[2], buf_ir_test[3]);
	ret = data->dev_id = (buf_ir_test[2] << 8 | buf_ir_test[3]);

	data->on_off = 0;

	fpga_enable(0,0);

	return ret;
}

/* sysfs node check_ir */
static ssize_t check_ir_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	int ret;

	ret = irda_read_device_info(data);
	return snprintf(buf, 4, "%d\n", ret);
}

/* sysfs node irda_test */
static ssize_t irda_test_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int ret, i;
	struct barcode_emul_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	struct {
			unsigned char addr;
			unsigned char data[IRDA_TEST_CODE_SIZE];
		} i2c_block_transfer;

	unsigned char BSR_data[IRDA_TEST_CODE_SIZE] = {
		0x8D, 0x00, 0x96, 0x00, 0x01, 0x50, 0x00, 0xA8,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F
	};
	pr_barcode("IRDA test code start\n");

	/* change address for IRDA */
	client->addr = IRDA_I2C_ADDR;

	/* make data for sending */
	for (i = 0; i < IRDA_TEST_CODE_SIZE; i++)
		i2c_block_transfer.data[i] = BSR_data[i];

	fpga_enable(1,1);

	/* sending data by I2C */
	i2c_block_transfer.addr = IRDA_TEST_CODE_ADDR;
	ret = i2c_master_send(client, (unsigned char *) &i2c_block_transfer,
			IRDA_TEST_CODE_SIZE);
	if (ret < 0) {
		pr_err("%s: err1 %d\n", __func__, ret);
		ret = i2c_master_send(client,
		(unsigned char *) &i2c_block_transfer, IRDA_TEST_CODE_SIZE);
		if (ret < 0)
			pr_err("%s: err2 %d\n", __func__, ret);
	}

	fpga_enable(0,0);

	return size;
}

static ssize_t irda_test_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return strlen(buf);
}

static struct device_attribute ir_attrs[] = {
	__ATTR(check_ir, S_IRUGO|S_IWUSR|S_IWGRP, check_ir_show, NULL),
	__ATTR(ir_send, S_IRUGO|S_IWUSR|S_IWGRP, remocon_show, remocon_store),
	__ATTR(ir_send_result, S_IRUGO|S_IWUSR|S_IWGRP, remocon_ack, NULL),
	__ATTR(irda_test, S_IRUGO|S_IWUSR|S_IWGRP, irda_test_show, irda_test_store)
};
#endif
/*
static int ice_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	return ice_gpiox_get((unsigned)offset);
}

static void ice_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	ice_gpiox_set((unsigned)offset, value);
}

static struct gpio_chip ice_gpio = {
	.label = "ice-fpga-gpio",
	.get = ice_gpio_get,
	.set = ice_gpio_set,
	.base = FPGA_GPIO_BASE,
	.ngpio = NR_FPGA_GPIO,
};

int ice_gpiox_get(int num)
{
	int ret;
	int cmd;

	if (!fw_dl_complete)
		return -EBUSY;

	g_client->addr = BARCODE_I2C_ADDR;

	if (g_pdata->fw_type == ICE_24M)
		cmd = 0xFC | (num >> 3);
	else
	cmd = num >> 3;

	fpga_enable(1);
	ret = i2c_smbus_read_byte_data(g_client, cmd);
	if (ret) {
		fpga_enable(0);
		fpga_enable(1);
		ret = i2c_smbus_read_byte_data(g_client, cmd);
	}

	fpga_enable(0);
	if (ret < 0) {
		pr_err("%s i2c error : %d\n", __func__, ret);
		return ret;
	} else if (ret & (1 << (num&7))) {
		pr_barcode("%s : num = %d , val = 1\n", __func__, num);
		return 1;
	} else {
		pr_barcode("%s : num = %d , val = 0\n", __func__, num);
		return 0;
	}

}

EXPORT_SYMBOL(ice_gpiox_get);

int ice_gpiox_set(int num, int val)
{
	int ret;
	int cmd;

	if (!fw_dl_complete)
		return -EBUSY;

	g_client->addr = BARCODE_I2C_ADDR;

	if (g_pdata->fw_type == ICE_24M)
		cmd = 0xFC | (num >> 3);
	else
	cmd = num >> 3;

	if (val)
		gpiox_state |= 1 << num;
	else
		gpiox_state &= ~(1 << num);

	pr_barcode("%s : num = %d , val = %d\n", __func__, num, val);

	fpga_enable(1);

	ret = i2c_smbus_write_byte_data(g_client, cmd,
			(num >> 3) ? (gpiox_state >> 8) : gpiox_state);
	if (ret) {
		fpga_enable(0);
		fpga_enable(1);
		ret = i2c_smbus_write_byte_data(g_client, cmd,
				(num >> 3) ? (gpiox_state >> 8) : gpiox_state);
	}

	fpga_enable(0);
	if (ret)
		pr_err("%s i2c error : %d\n", __func__, ret);

	return ret;
}
EXPORT_SYMBOL(ice_gpiox_set);
*/
static void fw_work(struct work_struct *work)
{
	ice4_fpga_firmware_update_hlte();
	Is_clk_enabled = 0;

}

static int __devinit barcode_emul_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct barcode_emul_data *data;
	struct barcode_emul_platform_data *pdata;
	struct device *barcode_emul_dev;
	int error;
	
#ifdef CONFIG_IR_REMOCON_FPGA
	int i;
#endif
	enable_counte =0;
	fw_dl_complete = false;
	pr_barcode("%s probe!\n", __func__);
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct barcode_emul_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}
		error = barcode_parse_dt(&client->dev, pdata);
		if (error)
			return error;
	} else
		pdata = client->dev.platform_data;

#if !defined(CONFIG_MACH_VIENNAEUR) && !defined(CONFIG_MACH_LT03EUR)\
	&& !defined(CONFIG_MACH_LT03SKT) && !defined(CONFIG_MACH_LT03KTT)\
	&& !defined(CONFIG_MACH_LT03LGT)
	if(system_rev < BOARD_REV02)
		pdata->fw_type = ICE_I2C_2;

	 else if (system_rev == BOARD_REV02)
		pdata->fw_type = ICE_I2C_R2;

	 else
		pdata->fw_type = ICE_I2C_R3;	
#else
	pdata->fw_type = ICE_I2C_R3;
#endif
	pr_barcode("%d system_rev!\n", system_rev);

	g_pdata = pdata;
	pr_barcode("%s setting gpio config.\n", __func__);
	barcode_gpio_config();
//	bc_poweron(client->dev);
	client->irq = gpio_to_irq(pdata->irda_irq);

	data = kzalloc(sizeof(struct barcode_emul_data), GFP_KERNEL);
	if (NULL == data) {
		pr_err("Failed to data allocate %s\n", __func__);
		return -ENOMEM;		
	}
	
	data->client = client;
	mutex_init(&en_mutex);
#ifdef CONFIG_IR_REMOCON_FPGA
	data->pdata = client->dev.platform_data;
	mutex_init(&data->mutex);
	data->count = 2;
	data->on_off = 0;
#endif
	i2c_set_clientdata(client, data);

	barcode_emul_dev = device_create(sec_class, NULL, 0,
				data, "sec_barcode_emul");

	if (IS_ERR(barcode_emul_dev))
		pr_err("Failed to create barcode_emul_dev device\n");

	if (device_create_file(barcode_emul_dev, &dev_attr_barcode_send) < 0)
		pr_err("Failed to create device file(%s)!\n",
				dev_attr_barcode_send.attr.name);
	if (device_create_file(barcode_emul_dev,
			&dev_attr_barcode_test_send) < 0)
		pr_err("Failed to create device file(%s)!\n",
				dev_attr_barcode_test_send.attr.name);
	if (device_create_file(barcode_emul_dev,
			&dev_attr_barcode_fw_update) < 0)
		pr_err("Failed to create device file(%s)!\n",
				dev_attr_barcode_fw_update.attr.name);
	if (device_create_file(barcode_emul_dev,
			&dev_attr_barcode_ver_check) < 0)
		pr_err("Failed to create device file(%s)!\n",
				dev_attr_barcode_ver_check.attr.name);
	if (device_create_file(barcode_emul_dev,
			&dev_attr_barcode_led_status) < 0)
		pr_err("Failed to create device file(%s)!\n",
				dev_attr_barcode_led_status.attr.name);
#if defined(CONFIG_IR_REMOCON_FPGA)
	barcode_emul_dev = device_create(sec_class, NULL, 0, data, "sec_ir");
	if (IS_ERR(barcode_emul_dev))
		pr_err("Failed to create barcode_emul_dev device in sec_ir\n");

	/* sysfs entries */
	for (i = 0; i < ARRAY_SIZE(ir_attrs); i++) {
		if (device_create_file(barcode_emul_dev, &ir_attrs[i]) < 0)
			pr_err("Failed to create device file(%s)!\n", ir_attrs[i].attr.name);
	}
#endif

	/*Create dedicated thread so that
	 the delay of our work does not affect others*/
	data->firmware_dl =
		create_singlethread_workqueue("barcode_firmware_dl");
	INIT_DELAYED_WORK(&data->fw_dl, fw_work);
	/* min 1ms is needed */
	queue_delayed_work(data->firmware_dl,
			&data->fw_dl, msecs_to_jiffies(20));

	g_client = client;

	Is_beaming = BEAMING_OFF;

	pr_err("probe complete %s\n", __func__);

	return 0;
}

static int __devexit barcode_emul_remove(struct i2c_client *client)
{
	struct barcode_emul_data *data = i2c_get_clientdata(client);

	i2c_set_clientdata(client, NULL);
	kfree(data);
	return 0;
}

static const struct i2c_device_id barcode_id[] = {
	{"barcode_hlte", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, barcode_id);
#ifdef CONFIG_OF
static struct of_device_id barcode_match_table[] = {
	{ .compatible = "barcode_hlte,barcode_emul_hlte",},
	{ },
};
#else
#define barcode_match_table	NULL
#endif

static struct i2c_driver ice4_i2c_driver = {
	.driver = {
		.name = "barcode_hlte",
		.owner = THIS_MODULE,
		.of_match_table = barcode_match_table,
	},
	.probe = barcode_emul_probe,
	.remove = __devexit_p(barcode_emul_remove),
	.id_table = barcode_id,
};

static int __init barcode_emul_init(void)
{
/*	int ret;
	ret = gpiochip_add(&ice_gpio);
	if (ret) {
		pr_err("gpiochip_add failed ret = %d\n", ret);
		return ret;
	}*/
	return i2c_add_driver(&ice4_i2c_driver);
}
module_init(barcode_emul_init);

static void __exit barcode_emul_exit(void)
{
	i2c_del_driver(&ice4_i2c_driver);
}
module_exit(barcode_emul_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SEC Barcode emulator");
