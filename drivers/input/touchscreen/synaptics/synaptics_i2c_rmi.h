/*
 * Synaptics DSX touchscreen driver
 *
 * Copyright (C) 2012 Synaptics Incorporated
 *
 * Copyright (C) 2012 Alexandra Chin <alexandra.chin@tw.synaptics.com>
 * Copyright (C) 2012 Scott Lin <scott.lin@tw.synaptics.com>
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
 */
#ifndef _SYNAPTICS_RMI4_H_
#define _SYNAPTICS_RMI4_H_

#define SYNAPTICS_RMI4_DRIVER_VERSION "DS5 1.0"
#include <linux/device.h>
#include <linux/i2c/synaptics_rmi.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/*#define dev_dbg(dev, fmt, arg...) dev_info(dev, fmt, ##arg)*/

#define SYNAPTICS_DEVICE_NAME	"GT-I95XX"
/* DVFS feature : TOUCH BOOSTER */
#define TSP_BOOSTER
#ifdef TSP_BOOSTER
#define DVFS_STAGE_TRIPLE	3
#define DVFS_STAGE_DUAL		2
#define DVFS_STAGE_SINGLE	1
#define DVFS_STAGE_NONE		0
#include <linux/cpufreq.h>

#define TOUCH_BOOSTER_OFF_TIME	500
#define TOUCH_BOOSTER_CHG_TIME	130
#endif

/* To support suface touch, firmware should support data
 * which is required related app ex) MT_ANGLE, MT_PALM ...
 * Synpatics IC report those data through F51's edge swipe
 * fucntionality.
 */
#define SURFACE_TOUCH
#define PROXIMITY
#define EDGE_SWIPE
#define	USE_OPEN_CLOSE
#define GLOVE_MODE
#define READ_LCD_ID
#define HAND_GRIP_MODE

#if defined(CONFIG_SEC_H_PROJECT)
#define USE_HOVER_REZERO
#endif

#if defined(CONFIG_SEC_H_PROJECT) || defined(CONFIG_SEC_JS_PROJECT)
#define TSP_INIT_COMPLETE
#else
#undef TSP_INIT_COMPLETE
#endif

#if defined(CONFIG_SEC_F_PROJECT) || defined(CONFIG_SEC_JVE_PROJECT)
#define TOUCKEY_ENABLE
#endif
#if defined(CONFIG_SEC_F_PROJECT)
#define TOUCHKEY_CHANNEL_ADD
#define EDGE_SWIPE_SCALE
#endif


/* TA_CON mode @ H mode */

#if defined(CONFIG_SEC_H_PROJECT)

#if defined(CONFIG_MACH_HLTEATT) || defined(CONFIG_MACH_HLTETMO)
#define TA_CON_REVISION		0x08
#elif defined(CONFIG_MACH_HLTEEUR) || defined(CONFIG_MACH_HLTEVZW) ||\
	defined(CONFIG_MACH_HLTESPR) || defined(CONFIG_MACH_HLTEUSC) ||\
	defined(CONFIG_MACH_HLTEDCM)
#define TA_CON_REVISION		0x07
#elif defined(CONFIG_MACH_HLTEKDI)
#define TA_CON_REVISION		0x05
#elif defined(CONFIG_MACH_HLTESKT) || defined(CONFIG_MACH_HLTEKTT) ||\
	defined(CONFIG_MACH_HLTELGT)
#define TA_CON_REVISION		0x04
#else
#define TA_CON_REVISION		0xFF
#endif

#elif defined(CONFIG_SEC_F_PROJECT)
#define TA_CON_REVISION		0x08

#elif defined(CONFIG_SEC_JVE_PROJECT)
#define TA_CON_REVISION		0x10

#else
#define TA_CON_REVISION		0xFF
#endif

#ifdef GLOVE_MODE
#define GLOVE_MODE_EN (1 << 0)
#define CLOSED_COVER_EN (1 << 1)
#define FAST_DETECT_EN (1 << 2)
#endif

#ifdef USE_OPEN_DWORK
#define TOUCH_OPEN_DWORK_TIME 10
#endif

#define SYNAPTICS_HW_RESET_TIME	100
#define SYNAPTICS_POWER_MARGIN_TIME	150

#define SYNAPTICS_PRODUCT_ID_NONE	0
#define SYNAPTICS_PRODUCT_ID_S5000	1
#define SYNAPTICS_PRODUCT_ID_S5050	2

#define FW_IMAGE_NAME_NONE		NULL
#define FW_IMAGE_NAME_S5000		"tsp_synaptics/synaptics_s5000.fw"
#define FW_IMAGE_NAME_S5050		"tsp_synaptics/synaptics.fw"
#define FW_IMAGE_NAME_S5050_NONTR	"tsp_synaptics/synaptics_nonTR.fw"
#define FW_IMAGE_NAME_S5050_F		"tsp_synaptics/synaptics_f.fw"
#define FW_IMAGE_NAME_S5050_HSYNCF		"tsp_synaptics/synaptics_f_HSYNC.fw"
#define FW_IMAGE_NAME_S5050_J	"tsp_synaptics/synaptics_j.fw"
#define FW_IMAGE_NAME_S5050_G2_J	"tsp_synaptics/synaptics_g2_j.fw"

#define SYNAPTICS_FACTORY_TEST_PASS		2
#define SYNAPTICS_FACTORY_TEST_FAIL		1
#define SYNAPTICS_FACTORY_TEST_NONE		0

#define SYNAPTICS_MAX_FW_PATH	64

#define SYNAPTICS_DEFAULT_UMS_FW "/sdcard/synaptics.fw"

#define DATE_OF_FIRMWARE_BIN_OFFSET	0xEF00
#define IC_REVISION_BIN_OFFSET	0xEF02
#define FW_VERSION_BIN_OFFSET	0xEF03

#define DATE_OF_FIRMWARE_BIN_OFFSET_S5050	0x016D00
#define IC_REVISION_BIN_OFFSET_S5050 		0x016D02
#define FW_VERSION_BIN_OFFSET_S5050 		0x016D03


#define PDT_PROPS (0X00EF)
#define PDT_START (0x00E9)
#define PDT_END (0x000A)
#define PDT_ENTRY_SIZE (0x0006)
#define PAGES_TO_SERVICE (10)
#define PAGE_SELECT_LEN (2)

#define SYNAPTICS_RMI4_F01 (0x01)
#define SYNAPTICS_RMI4_F11 (0x11)
#define SYNAPTICS_RMI4_F12 (0x12)
#define SYNAPTICS_RMI4_F1A (0x1a)
#define SYNAPTICS_RMI4_F34 (0x34)
#define SYNAPTICS_RMI4_F51 (0x51)
#define SYNAPTICS_RMI4_F54 (0x54)
#define SYNAPTICS_RMI4_F55 (0x55)
#define SYNAPTICS_RMI4_FDB (0xdb)

#define SYNAPTICS_RMI4_PRODUCT_INFO_SIZE 2
#define SYNAPTICS_RMI4_DATE_CODE_SIZE 3
#define SYNAPTICS_RMI4_PRODUCT_ID_SIZE 10
#define SYNAPTICS_RMI4_BUILD_ID_SIZE 3
#define SYNAPTICS_RMI4_PRODUCT_ID_LENGTH 10
#define SYNAPTICS_RMI4_PACKAGE_ID_SIZE 4

#define MAX_NUMBER_OF_BUTTONS 4
#define MAX_INTR_REGISTERS 4
#define MAX_NUMBER_OF_FINGERS 10
#define F12_FINGERS_TO_SUPPORT 10

#define MASK_16BIT 0xFFFF
#define MASK_8BIT 0xFF
#define MASK_7BIT 0x7F
#define MASK_6BIT 0x3F
#define MASK_5BIT 0x1F
#define MASK_4BIT 0x0F
#define MASK_3BIT 0x07
#define MASK_2BIT 0x03
#define MASK_1BIT 0x01

/*
 * struct synaptics_rmi4_fn_desc - function descriptor fields in PDT
 * @query_base_addr: base address for query registers
 * @cmd_base_addr: base address for command registers
 * @ctrl_base_addr: base address for control registers
 * @data_base_addr: base address for data registers
 * @intr_src_count: number of interrupt sources
 * @fn_number: function number
 */
struct synaptics_rmi4_fn_desc {
	unsigned char query_base_addr;
	unsigned char cmd_base_addr;
	unsigned char ctrl_base_addr;
	unsigned char data_base_addr;
	unsigned char intr_src_count;
	unsigned char fn_number;
};

/*
 * synaptics_rmi4_fn_full_addr - full 16-bit base addresses
 * @query_base: 16-bit base address for query registers
 * @cmd_base: 16-bit base address for data registers
 * @ctrl_base: 16-bit base address for command registers
 * @data_base: 16-bit base address for control registers
 */
struct synaptics_rmi4_fn_full_addr {
	unsigned short query_base;
	unsigned short cmd_base;
	unsigned short ctrl_base;
	unsigned short data_base;
};

struct synaptics_rmi4_f12_extra_data {
	unsigned char data1_offset;
	unsigned char data15_offset;
	unsigned char data15_size;
	unsigned char data15_data[(F12_FINGERS_TO_SUPPORT + 7) / 8];
};

/*
 * struct synaptics_rmi4_fn - function handler data structure
 * @fn_number: function number
 * @num_of_data_sources: number of data sources
 * @num_of_data_points: maximum number of fingers supported
 * @size_of_data_register_block: data register block size
 * @data1_offset: offset to data1 register from data base address
 * @intr_reg_num: index to associated interrupt register
 * @intr_mask: interrupt mask
 * @full_addr: full 16-bit base addresses of function registers
 * @link: linked list for function handlers
 * @data_size: size of private data
 * @data: pointer to private data
 */
struct synaptics_rmi4_fn {
	unsigned char fn_number;
	unsigned char num_of_data_sources;
	unsigned char num_of_data_points;
	unsigned char size_of_data_register_block;
	unsigned char intr_reg_num;
	unsigned char intr_mask;
	struct synaptics_rmi4_fn_full_addr full_addr;
	struct list_head link;
	int data_size;
	void *data;
	void *extra;
};

/*
 * struct synaptics_rmi4_device_info - device information
 * @version_major: rmi protocol major version number
 * @version_minor: rmi protocol minor version number
 * @manufacturer_id: manufacturer id
 * @product_props: product properties information
 * @product_info: product info array
 * @date_code: device manufacture date
 * @tester_id: tester id array
 * @serial_number: device serial number
 * @product_id_string: device product id
 * @support_fn_list: linked list for function handlers
 */
struct synaptics_rmi4_device_info {
	unsigned int version_major;
	unsigned int version_minor;
	unsigned char manufacturer_id;
	unsigned char product_props;
	unsigned char product_info[SYNAPTICS_RMI4_PRODUCT_INFO_SIZE];
	unsigned char date_code[SYNAPTICS_RMI4_DATE_CODE_SIZE];
	unsigned short tester_id;
	unsigned short serial_number;
	unsigned char product_id_string[SYNAPTICS_RMI4_PRODUCT_ID_SIZE + 1];
	unsigned char build_id[SYNAPTICS_RMI4_BUILD_ID_SIZE];
	unsigned int package_id;
	unsigned int package_rev;
	struct list_head support_fn_list;
};

/**
 * struct synaptics_finger - Represents fingers.
 * @ state: finger status.
 * @ mcount: moving counter for debug.
 */
struct synaptics_finger {
	unsigned char state;
	unsigned short mcount;
};

/*
 * struct synaptics_rmi4_data - rmi4 device instance data
 * @i2c_client: pointer to associated i2c client
 * @input_dev: pointer to associated input device
 * @board: constant pointer to platform data
 * @rmi4_mod_info: device information
 * @regulator: pointer to associated regulator
 * @rmi4_io_ctrl_mutex: mutex for i2c i/o control
 * @early_suspend: instance to support early suspend power management
 * @current_page: current page in sensor to acess
 * @button_0d_enabled: flag for 0d button support
 * @full_pm_cycle: flag for full power management cycle in early suspend stage
 * @num_of_intr_regs: number of interrupt registers
 * @f01_query_base_addr: query base address for f01
 * @f01_cmd_base_addr: command base address for f01
 * @f01_ctrl_base_addr: control base address for f01
 * @f01_data_base_addr: data base address for f01
 * @irq: attention interrupt
 * @sensor_max_x: sensor maximum x value
 * @sensor_max_y: sensor maximum y value
 * @irq_enabled: flag for indicating interrupt enable status
 * @touch_stopped: flag to stop interrupt thread processing
 * @fingers_on_2d: flag to indicate presence of fingers in 2d area
 * @sensor_sleep: flag to indicate sleep state of sensor
 * @wait: wait queue for touch data polling in interrupt thread
 * @i2c_read: pointer to i2c read function
 * @i2c_write: pointer to i2c write function
 * @irq_enable: pointer to irq enable function
 */
struct synaptics_rmi4_data {
	struct i2c_client *i2c_client;
	struct input_dev *input_dev;
	struct synaptics_rmi4_platform_data *board;
	struct synaptics_rmi4_power_data *pwrdata;
	struct synaptics_rmi4_device_info rmi4_mod_info;
	struct regulator *regulator;
	struct mutex rmi4_reset_mutex;
	struct mutex rmi4_io_ctrl_mutex;
	struct mutex rmi4_reflash_mutex;
	struct timer_list f51_finger_timer;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	unsigned char *firmware_image;

	struct completion init_done;
	struct synaptics_finger finger[MAX_NUMBER_OF_FINGERS];

	unsigned char current_page;
	unsigned char button_0d_enabled;
	unsigned char full_pm_cycle;
	unsigned char num_of_rx;
	unsigned char num_of_tx;
	unsigned int num_of_node;
	unsigned char num_of_fingers;
	unsigned char max_touch_width;
	unsigned char feature_enable;
	unsigned char report_enable;
	unsigned char no_sleep_setting;
	unsigned char intr_mask[MAX_INTR_REGISTERS];
	unsigned char *button_txrx_mapping;
	unsigned short num_of_intr_regs;
	unsigned short f01_query_base_addr;
	unsigned short f01_cmd_base_addr;
	unsigned short f01_ctrl_base_addr;
	unsigned short f01_data_base_addr;
	unsigned short f12_ctrl11_addr;
	unsigned short f12_ctrl26_addr;
	unsigned short f12_ctrl28_addr;
	unsigned short f34_ctrl_base_addr;
	int irq;
	int sensor_max_x;
	int sensor_max_y;
	int touch_threshold;
	int gloved_sensitivity;
	int ta_status;
	bool flash_prog_mode;
	bool irq_enabled;
	bool touch_stopped;
	bool fingers_on_2d;
	bool f51_finger;
	bool hand_edge_down;
	bool has_edge_swipe;
	bool has_glove_mode;
	bool sensor_sleep;
	bool stay_awake;
	bool staying_awake;
	bool tsp_probe;
	bool firmware_cracked;

#if defined(CONFIG_SEC_F_PROJECT) || defined(CONFIG_SEC_JVE_PROJECT)
	int touchkey_menu;
	int touchkey_back;
	bool touchkey_led;
#endif

	int ic_version;				/* define S5000, S5050 */
	int ic_revision_of_ic;
	int ic_revision_of_bin;		/* revision of reading from binary */
	int fw_version_of_ic;		/* firmware version of IC */
	int fw_version_of_bin;		/* firmware version of binary */
	int fw_release_date_of_ic;	/* Config release data from IC */
	bool doing_reflash;
	int rebootcount;
#ifdef READ_LCD_ID
	int lcd_id;
#endif

	struct regulator *vcc_en;

#ifdef CONFIG_SEC_TSP_FACTORY
	int bootmode;
#endif

	bool ta_con_mode;	/* ta_con_mode == true ? I2C (RMI) : INT(GPIO) */

#ifdef TSP_BOOSTER
	struct delayed_work	work_dvfs_off;
	struct delayed_work	work_dvfs_chg;
	struct mutex		dvfs_lock;
	bool dvfs_lock_status;
	int dvfs_old_stauts;
	int dvfs_boost_mode;
	int dvfs_freq;
#endif
	bool hover_status_in_normal_mode;

#ifdef USE_HOVER_REZERO
	struct delayed_work rezero_work;
#endif

	bool fast_glove_state;
	bool touchkey_glove_mode_status;

#ifdef USE_OPEN_DWORK
	struct delayed_work open_work;
#endif
	struct mutex rmi4_device_mutex;

#ifdef HAND_GRIP_MODE
	unsigned int  hand_grip_mode;
	unsigned int hand_grip;
	unsigned int old_hand_grip;
	unsigned int old_code;
#endif

	bool created_sec_class;

	void (*register_cb)(struct synaptics_rmi_callbacks *);
	struct synaptics_rmi_callbacks callbacks;

	int (*i2c_read)(struct synaptics_rmi4_data *pdata, unsigned short addr,
			unsigned char *data, unsigned short length);
	int (*i2c_write)(struct synaptics_rmi4_data *pdata, unsigned short addr,
			unsigned char *data, unsigned short length);
	int (*irq_enable)(struct synaptics_rmi4_data *rmi4_data, bool enable);
	int (*reset_device)(struct synaptics_rmi4_data *rmi4_data);
	int (*stop_device)(struct synaptics_rmi4_data *rmi4_data);
	int (*start_device)(struct synaptics_rmi4_data *rmi4_data);
};

enum exp_fn {
	RMI_DEV = 0,
	RMI_F54,
	RMI_FW_UPDATER,
	RMI_DB,
	RMI_LAST,
};

struct synaptics_rmi4_exp_fn_ptr {
	int (*read)(struct synaptics_rmi4_data *rmi4_data, unsigned short addr,
			unsigned char *data, unsigned short length);
	int (*write)(struct synaptics_rmi4_data *rmi4_data, unsigned short addr,
			unsigned char *data, unsigned short length);
	int (*enable)(struct synaptics_rmi4_data *rmi4_data, bool enable);
};

int synaptics_rmi4_new_function(enum exp_fn fn_type,
		int (*func_init)(struct synaptics_rmi4_data *rmi4_data),
		void (*func_remove)(struct synaptics_rmi4_data *rmi4_data),
		void (*func_attn)(struct synaptics_rmi4_data *rmi4_data,
			unsigned char intr_mask));

int rmidev_module_register(void);
int rmi4_f54_module_register(void);

int synaptics_rmi4_f54_set_control(struct synaptics_rmi4_data *rmi4_data);

int rmi4_fw_update_module_register(void);
int rmidb_module_register(void);
int synaptics_fw_updater(unsigned char *fw_data);
int synaptics_rmi4_fw_update_on_probe(struct synaptics_rmi4_data *rmi4_data);
int synaptics_rmi4_proximity_enables(unsigned char enables);
int synaptics_proximity_no_sleep_set(bool enables);
int synaptics_rmi4_f12_set_feature(struct synaptics_rmi4_data *rmi4_data);
int synaptics_rmi4_set_tsp_test_result_in_config(int pass_fail);
int synaptics_rmi4_tsp_read_test_result(struct synaptics_rmi4_data *rmi4_data);
int synaptics_rmi4_f51_grip_edge_exclusion_rx(bool enables);

int synaptics_rmi4_f12_ctrl11_set (struct synaptics_rmi4_data *rmi4_data,
		unsigned char data);

extern struct class *sec_class;

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif

static inline ssize_t synaptics_rmi4_show_error(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	dev_warn(dev, "%s Attempted to read from write-only attribute %s\n",
			__func__, attr->attr.name);
	return -EPERM;
}

static inline ssize_t synaptics_rmi4_store_error(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	dev_warn(dev, "%s Attempted to write to read-only attribute %s\n",
			__func__, attr->attr.name);
	return -EPERM;
}

static inline void batohs(unsigned short *dest, unsigned char *src)
{
	*dest = src[1] * 0x100 + src[0];
}

static inline void hstoba(unsigned char *dest, unsigned short src)
{
	dest[0] = src % 0x100;
	dest[1] = src / 0x100;
}
#endif
