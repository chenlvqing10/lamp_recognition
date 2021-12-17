/*
 * base64.c - base64 conversion to bin
 *
 *
 * Copyright (C) 2016-2019, LomboTech Co.Ltd.
 * Author: lomboswer <lomboswer@lombotech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base/include/base64.h"
#define LOG_TAG "base64"
#include "base/include/file_ops.h"
#define SAFE_FREE(ptr) if(ptr)free(ptr)
const char *base64char =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/*3-4*/
/**
 * @brief  encode the data
 * @param  *bindata: the data that needs to be encode
 * @param  *base64: base64 code value
 * @param  binlength: data length
 * @retval base64 code
 */
char *base64_encode(const unsigned char *bindata, char *base64, int binlength)
{
	int i, j;
	unsigned char current;

	for (i = 0, j = 0; i < binlength; i += 3) {
		current = (bindata[i] >> 2);
		current &= (unsigned char) 0x3F;
		base64[j++] = base64char[(int) current];

		current = ((unsigned char)(bindata[i] << 4)) & ((unsigned char) 0x30);
		if (i + 1 >= binlength) {
			base64[j++] = base64char[(int) current];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ((unsigned char)(bindata[i + 1] >> 4))
			& ((unsigned char) 0x0F);
		base64[j++] = base64char[(int) current];

		current = ((unsigned char)(bindata[i + 1] << 2))
			& ((unsigned char) 0x3C);
		if (i + 2 >= binlength) {
			base64[j++] = base64char[(int) current];
			base64[j++] = '=';
			break;
		}
		current |= ((unsigned char)(bindata[i + 2] >> 6))
			& ((unsigned char) 0x03);
		base64[j++] = base64char[(int) current];

		current = ((unsigned char) bindata[i + 2]) & ((unsigned char) 0x3F);
		base64[j++] = base64char[(int) current];
	}
	base64[j] = '\0';
	return base64;
}
/*4-3*/
/**
 * @brief  base64 data the decode
 * @param  *base64:base64 code value
 * @param  *bindata: decode the data
 * @retval 0 if success
 */
int base64_decode(const char *base64, unsigned char *bindata)
{

	int i, j;
	unsigned char k;
	unsigned char temp[4];
	for (i = 0, j = 0; base64[i] != '\0'; i += 4) {
		memset(temp, 0xFF, sizeof(temp));
		for (k = 0; k < 64; k++) {
			if (base64char[k] == base64[i])
				temp[0] = k;
		}
		for (k = 0; k < 64; k++) {
			if (base64char[k] == base64[i + 1])
				temp[1] = k;
		}
		for (k = 0; k < 64; k++) {
			if (base64char[k] == base64[i + 2])
				temp[2] = k;
		}
		for (k = 0; k < 64; k++) {
			if (base64char[k] == base64[i + 3])
				temp[3] = k;
		}

		bindata[j++] =
			((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC))
			| ((unsigned char)((unsigned char)(temp[1] >> 4)
					& 0x03));
		if (base64[i + 2] == '=')
			break;

		bindata[j++] =
			((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0))
			| ((unsigned char)((unsigned char)(temp[2] >> 2)
					& 0x0F));
		if (base64[i + 3] == '=')
			break;

		bindata[j++] =
			((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0))
			| ((unsigned char)(temp[3] & 0x3F));
	}
	return j;
}

/**
 * @brief  base64 to file
 * @note
 * @param  *base64data:
 * @param  *file:
 * @retval
 */
 int base2file(char *base64data, char *file)
{
	char *bindata;
	int len;

	if (NULL == base64data || NULL == file) {
		printf("base2file args is null\n");
		return -1;
	}
	bindata =(char*) malloc(strlen(base64data) * 3 / 4 + 1);
	if (NULL == bindata) {
		printf("base2file: malloc failed\n");
		return -1;
	}
	len = base64_decode((const char *)base64data,
		(unsigned char *)bindata);
	file_write(file, bindata, len);
	if (bindata) {
		free(bindata);
		bindata = NULL;
	}
	return 0;
}

/**
 * @brief  file to base64
 * @note
 * @param  *img_path:
 * @retval
 */
char *file2base(char *img_path)
{
	int size  = 0;
	char *bindata = NULL;
	char *base64data = NULL;
	if (img_path == NULL) {
		printf("%s: %s is not exist", __func__, img_path);
		return NULL;
	}

	size = file_length(img_path);
	if (size <= 0) {
		printf("%s: image path is invalid! skip!!!", __func__);
		return NULL;
	}

	bindata = (char*)malloc(size);
	if (bindata == NULL) {
		printf("%s: bindata malloc failed", __func__);
		return NULL;
	}
	if (file_read(img_path, bindata, size) < 0) {
		printf("%s: %s file_read  failed", __func__, img_path);
		SAFE_FREE(bindata);
		return NULL;
	}
	base64data = (char*)malloc(size * 4 / 3 + 4);
	if (base64data == NULL) {
		printf("%s: base64data malloc failed", __func__);
		SAFE_FREE(bindata);
		return NULL;
	}

	memset(base64data, 0, size * 4 / 3 + 4);
	base64_encode((const unsigned char *)bindata, base64data, size);

	SAFE_FREE(bindata);

	return base64data;
}

void free_base64data(char *base64data)
{
	if (base64data) {
		free(base64data);
		base64data = NULL;
	}
}
