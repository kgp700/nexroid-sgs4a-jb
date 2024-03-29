/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
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
 */
#include "ssp.h"

#define SSP_FIRMWARE_REVISION		101901
#define SSP_FIRMWARE_REVISION_03	13091300/*MCU L5, 6500*/

/* Bootload mode cmd */
#define BL_FW_NAME_L0			"ssp_L0.fw"
#define BL_FW_NAME			"ssp.fw"
#define BL_FW_NAME_03			"ssp_rev03.fw"
#define BL_UMS_FW_NAME			"ssp.bin"
#define BL_CRASHED_FW_NAME		"ssp_crashed.fw"

#define BL_UMS_FW_PATH			255

#define APP_SLAVE_ADDR			0x18
#define BOOTLOADER_SLAVE_ADDR		0x26

/* Bootloader mode status */
#define BL_WAITING_BOOTLOAD_CMD		0xc0	/* valid 7 6 bit only */
#define BL_WAITING_FRAME_DATA		0x80	/* valid 7 6 bit only */
#define BL_FRAME_CRC_CHECK		0x02
#define BL_FRAME_CRC_FAIL		0x03
#define BL_FRAME_CRC_PASS		0x04
#define BL_APP_CRC_FAIL			0x40	/* valid 7 8 bit only */
#define BL_BOOT_STATUS_MASK		0x3f

/* Command to unlock bootloader */
#define BL_UNLOCK_CMD_MSB		0xaa
#define BL_UNLOCK_CMD_LSB		0xdc

static int ssp_wait_for_chg(struct ssp_data *data)
{
	int timeout_counter = 0;
	int count = 1E4;

	while ((timeout_counter++ <= count) && gpio_get_value(data->chg))
		udelay(20);

	if (timeout_counter > count) {
		dev_err(&data->client->dev, "ssp_wait_for_chg() timeout!\n");
		return -EIO;
	}

	return 0;
}

unsigned int get_module_rev(struct ssp_data *data)
{
	if (data->ssp_changes == SSP_MCU_L0)
		return SSP_FIRMWARE_REVISION;
	else
		return SSP_FIRMWARE_REVISION_03;
}

static int check_bootloader(struct i2c_client *client, unsigned int uState)
{
	u8 uVal;
	u8 uTemp;
	struct ssp_data *data = i2c_get_clientdata(client);

recheck:
	if (i2c_master_recv(client, &uVal, 1) != 1)
		return -EIO;

	pr_debug("%s, uVal = 0x%x\n", __func__, uVal);
	if (uVal & 0x20) {
		if (i2c_master_recv(client, &uTemp, 1) != 1) {
			pr_err("[SSP]: %s - i2c recv fail\n", __func__);
			return -EIO;
		}

		if (i2c_master_recv(client, &uTemp, 1) != 1) {
			pr_err("[SSP]: %s - i2c recv fail\n", __func__);
			return -EIO;
		}

		uVal &= ~0x20;
	}

	if ((uVal & 0xF0) == BL_APP_CRC_FAIL) {
		pr_info("[SSP] SSP_APP_CRC_FAIL - There is no bootloader.\n");
		if (i2c_master_recv(client, &uVal, 1) != 1) {
			pr_err("[SSP]: %s - i2c recv fail\n", __func__);
			return -EIO;
		}

		if (uVal & 0x20) {
			if (i2c_master_recv(client, &uTemp, 1) != 1) {
				pr_err("[SSP]: %s - i2c recv fail\n", __func__);
				return -EIO;
			}

			if (i2c_master_recv(client, &uTemp, 1) != 1) {
				pr_err("[SSP]: %s - i2c recv fail\n", __func__);
				return -EIO;
			}

			uVal &= ~0x20;
		}
	}

	switch (uState) {
	case BL_WAITING_BOOTLOAD_CMD:
	case BL_WAITING_FRAME_DATA:
		uVal &= ~BL_BOOT_STATUS_MASK;
		break;
	case BL_FRAME_CRC_PASS:
		if (uVal == BL_FRAME_CRC_CHECK) {
			if (data->ssp_changes == SSP_MCU_L5)
				ssp_wait_for_chg(data);
			goto recheck;
		}
		break;
	default:
		return -EINVAL;
	}

	if (uVal != uState) {
		pr_err("[SSP]: %s - Invalid bootloader mode state\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int unlock_bootloader(struct i2c_client *client)
{
	u8 uBuf[2];

	uBuf[0] = BL_UNLOCK_CMD_LSB;
	uBuf[1] = BL_UNLOCK_CMD_MSB;

	if (i2c_master_send(client, uBuf, 2) != 2) {
		pr_err("[SSP]: %s - i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int fw_write(struct i2c_client *client,
	const u8 *pData, unsigned int uFrameSize)
{
	if (i2c_master_send(client, pData, uFrameSize) != uFrameSize) {
		pr_err("[SSP]: %s - i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int load_ums_fw_bootmode(struct i2c_client *client, const char *pFn)
{
	const u8 *buff = NULL;
	char fw_path[BL_UMS_FW_PATH+1];
	unsigned int uFrameSize;
	unsigned int uFSize = 0, uNRead = 0;
	unsigned int uPos = 0;
	int iRet = SUCCESS;
	int iCheckFrameCrcError = 0;
	int iCheckWatingFrameDataError = 0;
	int count = 0;
	struct file *fp = NULL;
	mm_segment_t old_fs = get_fs();
	struct ssp_data *data = i2c_get_clientdata(client);

	pr_info("[SSP] ssp_load_ums_fw start!!!\n");

	old_fs = get_fs();
	set_fs(get_ds());

	snprintf(fw_path, BL_UMS_FW_PATH, "/sdcard/ssp/%s", pFn);

	fp = filp_open(fw_path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		iRet = ERROR;
		pr_err("file %s open error:%d\n", fw_path, (s32)fp);
		goto err_open;
	}

	uFSize = (unsigned int)fp->f_path.dentry->d_inode->i_size;
	pr_info("ssp_load_ums firmware size: %u\n", uFSize);

	buff = kzalloc((size_t)uFSize, GFP_KERNEL);
	if (!buff) {
		iRet = ERROR;
		pr_err("fail to alloc buffer for fw\n");
		goto err_alloc;
	}

	uNRead = (unsigned int)vfs_read(fp, (char __user *)buff,
			(unsigned int)uFSize, &fp->f_pos);
	if (uNRead != uFSize) {
		iRet = ERROR;
		pr_err("fail to read file %s (nread = %u)\n", fw_path, uNRead);
		goto err_fw_size;
	}

	/* Unlock bootloader */
	iRet = unlock_bootloader(client);
	if (iRet < 0) {
		pr_err("[SSP] %s - unlock_bootloader failed! %d\n",
			__func__, iRet);
		goto out;
	}

	while (uPos < uFSize) {
		if (data->ssp_changes == SSP_MCU_L5) {
			iRet = ssp_wait_for_chg(data);
			if (iRet < 0) {
				pr_err("[SSP] %s - ssp_wait_for_chg failed! %d\n",
					__func__, iRet);
				goto out;
			}
		}
		if (check_bootloader(client, BL_WAITING_FRAME_DATA)) {
			iCheckWatingFrameDataError++;
			if (iCheckWatingFrameDataError > 10) {
				iRet = ERROR;
				pr_err("[SSP]: %s - F/W update fail\n",
					__func__);
				goto out;
			} else {
				pr_err("[SSP]: %s - F/W data_error %d, retry\n",
					__func__, iCheckWatingFrameDataError);
				continue;
			}
		}

		uFrameSize = (unsigned int)((*(buff + uPos) << 8) |
			*(buff + uPos + 1));

		/* We should add 2 at frame size as the the firmware data is not
		*  included the CRC bytes.
		*/
		uFrameSize += 2;

		/* Write one frame to device */
		fw_write(client, buff + uPos, uFrameSize);
		if (data->ssp_changes == SSP_MCU_L5) {
			iRet = ssp_wait_for_chg(data);
			if (iRet < 0) {
				pr_err("[SSP] %s - ssp_wait_for_chg failed! %d\n",
					__func__, iRet);
				goto out;
			}
		}
		if (check_bootloader(client, BL_FRAME_CRC_PASS)) {
			iCheckFrameCrcError++;
			if (iCheckFrameCrcError > 10) {
				iRet = ERROR;
				pr_err("[SSP]: %s - F/W Update Fail. crc err\n",
					__func__);
				goto out;
			} else {
				pr_err("[SSP]: %s - F/W crc_error %d, retry\n",
					__func__, iCheckFrameCrcError);
				continue;
			}
		}

		uPos += uFrameSize;
		if (count++ == 100) {
			pr_info("[SSP] Updated %u bytes / %u bytes\n", uPos,
				uFSize);
			count = 0;
		}

		mdelay(1);
	}

out:
err_fw_size:
	kfree(buff);
err_alloc:
	filp_close(fp, NULL);
err_open:
	set_fs(old_fs);

	return iRet;
}

static int load_kernel_fw_bootmode(struct i2c_client *client, const char *pFn)
{
	const struct firmware *fw = NULL;
	unsigned int uFrameSize;
	unsigned int uPos = 0;
	int iRet;
	int iCheckFrameCrcError = 0;
	int iCheckWatingFrameDataError = 0;
	int count = 0;
	struct ssp_data *data = i2c_get_clientdata(client);

	pr_info("[SSP] ssp_load_fw start!!!\n");

	iRet = request_firmware(&fw, pFn, &client->dev);
	if (iRet) {
		pr_err("[SSP]: %s - Unable to open firmware %s\n",
			__func__, pFn);
		return iRet;
	}

	/* Unlock bootloader */
	iRet = unlock_bootloader(client);
	if (iRet < 0) {
		pr_err("[SSP] %s - unlock_bootloader failed! %d\n",
			__func__, iRet);
		goto out;
	}

	while (uPos < fw->size) {
		if (data->ssp_changes == SSP_MCU_L5) {
			iRet = ssp_wait_for_chg(data);
			if (iRet < 0) {
				pr_err("[SSP] %s - ssp_wait_for_chg failed! %d\n",
					__func__, iRet);
				goto out;
			}
		}
		iRet = check_bootloader(client, BL_WAITING_FRAME_DATA);
		if (iRet) {
			iCheckWatingFrameDataError++;
			if (iCheckWatingFrameDataError > 10) {
				pr_err("[SSP]: %s - F/W update fail\n",
					__func__);
				goto out;
			} else {
				pr_err("[SSP]: %s - F/W data_error %d, retry\n",
					__func__, iCheckWatingFrameDataError);
				continue;
			}
		}

		uFrameSize = ((*(fw->data + uPos) << 8) |
			*(fw->data + uPos + 1));

		/* We should add 2 at frame size as the the firmware data is not
		*  included the CRC bytes.
		*/
		uFrameSize += 2;

		/* Write one frame to device */
		fw_write(client, fw->data + uPos, uFrameSize);
		if (data->ssp_changes == SSP_MCU_L5) {
			iRet = ssp_wait_for_chg(data);
			if (iRet < 0) {
				pr_err("[SSP] %s - ssp_wait_for_chg failed! %d\n",
					__func__, iRet);
				goto out;
			}
		}
		iRet = check_bootloader(client, BL_FRAME_CRC_PASS);
		if (iRet) {
			iCheckFrameCrcError++;
			if (iCheckFrameCrcError > 10) {
				pr_err("[SSP]: %s - F/W Update Fail. crc err\n",
					__func__);
				goto out;
			} else {
				pr_err("[SSP]: %s - F/W crc_error %d, retry\n",
					__func__, iCheckFrameCrcError);
				continue;
			}
		}

		uPos += uFrameSize;
		if (count++ == 100) {
			pr_info("[SSP] Updated %d bytes / %zd bytes\n", uPos,
				fw->size);
			count = 0;
		}
		mdelay(1);
	}
	pr_info("[SSP] Firmware download is success.(%d bytes)\n", uPos);
out:
	release_firmware(fw);
	return iRet;
}

static void change_to_bootmode(struct ssp_data *data)
{
	int iCnt = 0;

	for (iCnt = 0; iCnt < 10; iCnt++) {
		gpio_set_value_cansleep(data->rst, 0);
		udelay(10);

		gpio_set_value_cansleep(data->rst, 1);
		msleep(100);
	}

	msleep(50);
}

void toggle_mcu_reset(struct ssp_data *data)
{
	u8 uBuf[2] = {0,};

	gpio_set_value_cansleep(data->rst, 0);
	udelay(10);

	gpio_set_value_cansleep(data->rst, 1);
	mdelay(50);

	data->client->addr = BOOTLOADER_SLAVE_ADDR;

	if (i2c_master_send(data->client, uBuf, 2) != 2)
		pr_info("[SSP]: %s - ssp_Normal Mode\n", __func__);
	else
		pr_info("[SSP]: %s - ssp_load_fw_bootmode\n", __func__);

	data->client->addr = APP_SLAVE_ADDR;
}

static int update_mcu_bin(struct ssp_data *data, int iBinType)
{
	int iRet = SUCCESS;

	pr_info("[SSP] ssp_change_to_bootmode\n");
	change_to_bootmode(data);
	data->client->addr = BOOTLOADER_SLAVE_ADDR;

	switch (iBinType) {
	case KERNEL_BINARY:
		if (data->ssp_changes == SSP_MCU_L0)
			iRet = load_kernel_fw_bootmode(data->client,
				BL_FW_NAME_L0);
		else /* SSP_MCU_L5 */
			iRet = load_kernel_fw_bootmode(data->client,
				BL_FW_NAME_03);
		break;
	case KERNEL_CRASHED_BINARY:
		iRet = load_kernel_fw_bootmode(data->client,
		BL_CRASHED_FW_NAME);
	break;
	case UMS_BINARY:
		iRet = load_ums_fw_bootmode(data->client,
			BL_UMS_FW_NAME);
		break;
	default:
		pr_err("[SSP] binary type error!!\n");
	}

	msleep(SSP_SW_RESET_TIME);

	data->client->addr = APP_SLAVE_ADDR;

	return iRet;
}

int forced_to_download_binary(struct ssp_data *data, int iBinType)
{
	int iRet = 0;

	ssp_dbg("[SSP]: %s - mcu binany update!\n", __func__);

	ssp_enable(data, false);

	data->fw_dl_state = FW_DL_STATE_DOWNLOADING;
	pr_info("[SSP] %s, DL state = %d\n", __func__,
	data->fw_dl_state);

	iRet = update_mcu_bin(data, iBinType);
	if (iRet < 0) {
		ssp_dbg("[SSP]: %s - update_mcu_bin failed!\n", __func__);
		goto out;
	}

	data->fw_dl_state = FW_DL_STATE_SYNC;
	pr_info("[SSP] %s, DL state = %d\n", __func__, data->fw_dl_state);
	iRet = initialize_mcu(data);
    if (iRet == ERROR) {
        data->uResetCnt++;
        toggle_mcu_reset(data);
        msleep(SSP_SW_RESET_TIME);
        initialize_mcu(data);
    } else if (iRet < ERROR) {
        pr_err("[SSP]: %s - initialize_mcu failed\n", __func__);
		goto out;
    }

	ssp_enable(data, true);
	sync_sensor_state(data);

#ifdef CONFIG_SENSORS_SSP_SENSORHUB
	ssp_sensorhub_report_notice(data, MSG2SSP_AP_STATUS_RESET);
#endif

	data->fw_dl_state = FW_DL_STATE_DONE;
	pr_info("[SSP] %s, DL state = %d\n", __func__, data->fw_dl_state);

	iRet = SUCCESS;
out:
	return iRet;
}

static unsigned int check_firmware_rev(struct ssp_data *data)
{
	char chTxData = MSG2SSP_AP_FIRMWARE_REV;
	char chRxBuf[3] = {0,};
	unsigned int uRev = SSP_INVALID_REVISION;
	int iRet;

	iRet = ssp_i2c_read(data, &chTxData, 1, chRxBuf, 3, 0);
	if (iRet != SUCCESS) {
		pr_err("[SSP]: %s - i2c fail %d\n", __func__, iRet);
	} else
		uRev = ((unsigned int)chRxBuf[0] << 16)
			| ((unsigned int)chRxBuf[1] << 8) | chRxBuf[2];
	return uRev;
}

int check_fwbl(struct ssp_data *data)
{
	int iRet;
	unsigned int fw_revision;

	pr_info("[SSP] change_rev = %d\n", data->ssp_changes);

	if (data->ssp_changes == SSP_MCU_L0)
		fw_revision = SSP_FIRMWARE_REVISION;
	else
		fw_revision = SSP_FIRMWARE_REVISION_03;

	data->client->addr = APP_SLAVE_ADDR;
	data->uCurFirmRev = check_firmware_rev(data);

	if (data->uCurFirmRev == SSP_INVALID_REVISION) {
		toggle_mcu_reset(data);
		msleep(SSP_SW_RESET_TIME);
	}

	if ((data->uCurFirmRev == SSP_INVALID_REVISION) \
		|| (data->uCurFirmRev == SSP_INVALID_REVISION2)) {
		data->client->addr = BOOTLOADER_SLAVE_ADDR;
		iRet = check_bootloader(data->client, BL_WAITING_BOOTLOAD_CMD);

		if (iRet >= 0) {
			pr_info("[SSP] ssp_load_fw_bootmode\n");

			return FW_DL_STATE_NEED_TO_SCHEDULE;
		} else {
			pr_warn("[SSP] Firm Rev is invalid(%8u). Retry.\n",
				data->uCurFirmRev);
			data->client->addr = APP_SLAVE_ADDR;
			data->uCurFirmRev = get_firmware_rev(data);
			if (data->uCurFirmRev == SSP_INVALID_REVISION ||\
				data->uCurFirmRev == ERROR) {
				pr_err("[SSP] MCU is not working, FW download failed\n");
				return FW_DL_STATE_FAIL;
			} else if (data->uCurFirmRev != fw_revision) {
				pr_info("[SSP] MCU Firm Rev : Old = %8u, New = %8u\n",
					data->uCurFirmRev, fw_revision);

				return FW_DL_STATE_NEED_TO_SCHEDULE;
			}
			pr_info("[SSP] MCU Firm Rev : Old = %8u, New = %8u\n",
				data->uCurFirmRev, fw_revision);
		}
	} else {
		if (data->uCurFirmRev != fw_revision) {
			pr_info("[SSP] MCU Firm Rev : Old = %8u, New = %8u\n",
				data->uCurFirmRev, fw_revision);

			return FW_DL_STATE_NEED_TO_SCHEDULE;
		}
		pr_info("[SSP] MCU Firm Rev : Old = %8u, New = %8u\n",
			data->uCurFirmRev, fw_revision);
	}

	return FW_DL_STATE_NONE;
}
