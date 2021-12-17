/*
 * file_ops.c
 *
 * Copyright (C) 2019, LomboTech Co.Ltd.
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
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#define LOG_TAG		"file"
#include "base/include/file_ops.h"

#define FILE_COPY_SIZE		(16 * 1024)

/**
 * @brief  read file
 * @param  *file_path: the  file
 * @param  *buf: the data buf
 * @retval the read length if success  -1 failed
 */
int file_read(char *file_path, char *buf, int buf_len)
{
	FILE *file_fp = NULL;
	int  read_cnt = 0;
	if (NULL == file_path) {
		printf("%s:file_path is null\n", __func__);
		return -1;
	}
	if (NULL == buf) {
		printf("%s:buf is null\n", __func__);
		return -1;
	}
	if (buf_len <= 0) {
		printf("%s:buf_len is %d\n", __func__, buf_len);
		return -1;
	}
	file_fp = fopen(file_path, "rb");
	if (file_fp == NULL) {
		printf("%s: open %s failed\n", __func__, file_path);
		return -1;
	}
	read_cnt = fread(buf, 1, buf_len, file_fp);
	fclose(file_fp);

	return read_cnt;
}
/**
 * @brief  copy file
 * @param  *src_file_path: the source file
 * @param  *dest_file_path: the target file
 * @retval 0 if success
 */
int file_copy(char *src_file_path, char *dest_file_path)
{
	FILE *dest_fp = NULL, *src_fp = NULL;
	char *buff = NULL;
	int read_cnt, write_cnt, ret = -1;

	buff = (char*)malloc(FILE_COPY_SIZE);
	if (!buff) {
		printf("%s: alloc file buffer failed", __func__);
		goto free_res;
	}

	dest_fp = fopen(dest_file_path, "wb");
	if (dest_fp == NULL) {
		printf("%s: open %s failed\n", __func__, dest_file_path);
		goto free_res;
	}

	src_fp = fopen(src_file_path, "r");
	if (src_fp == NULL) {
		printf("%s: %s not found\n", __func__, src_file_path);
		goto free_res;
	}

	while (1) {
		read_cnt = fread(buff, 1, FILE_COPY_SIZE, src_fp);
		if (read_cnt > 0) {
			write_cnt = fwrite(buff, 1, read_cnt, dest_fp);
			if (read_cnt !=  write_cnt) {
				printf("%s: write file failed\n", __func__);
				goto free_res;
			}
		} else
			break;
	}

	ret = 0;

free_res:
	if (buff)
		free(buff);
	if (dest_fp)
		fclose(dest_fp);
	if (src_fp)
		fclose(src_fp);

	return ret;
}

/**
 * @brief  writes data to file
 * @param  *file_path: the file path
 * @param  *buf: data address
 * @param  size: data size
 * @retval -1 if faile
 */
int file_write(char *file_path, char *buf, int size)
{
	FILE *flip = NULL;
	int count = 0;

	flip = fopen(file_path, "wb");
	if (flip == NULL) {
		printf("%s:open %s failed\n", __func__, file_path);
		return -1;
	}

	count = fwrite(buf, 1, size, flip);
	if (count != size) {
		printf("write file(%s) failed, size:%d != write_count:%d\n",
			file_path, size, count);
		fclose(flip);
		return -1;
	}
	fclose(flip);

	return 0;
}

/**
 * @brief  the number of bytes of data in a file
 * @param  *file_path: he file path
 * @retval -1 if faile
 */
int file_length(char *file_path)
{
	FILE *fp = NULL;
	int file_len;

	fp = fopen(file_path, "rb");
	if (fp == NULL) {
		printf("open %s failed", file_path);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	file_len = ftell(fp);
	if (file_len < 0) {
		fclose(fp);
		printf("get file length error");
		return -1;
	}
	fseek(fp, 0, SEEK_SET);
	fclose(fp);
	return file_len;
}

/**
 * @brief  modify file name
 * @param  *src_file_path: the source file
 * @param  *dest_file_path: the target file
 * @retval 0 if success
 */
int file_move(char *src_file_path, char *dest_file_path)
{
	int ret;

	ret = file_copy(src_file_path, dest_file_path);
	if (ret) {
		printf("failed to copy %s to %s", src_file_path, dest_file_path);
		return ret;
	}

	ret = remove(src_file_path);
	if (ret) {
		printf("failed to remove %s", src_file_path);
		remove(dest_file_path);
		return ret;
	}

	return 0;
}

int file_remove(char *file_path)
{
	int ret;

	if (0 != is_file_exist(file_path)) {
		printf("file(%s) does not exist", file_path);
		return -1;
	}

	ret = remove(file_path);
	if (ret) {
		printf("failed to remove file(%s)", file_path);
		return -1;
	}

	return 0;
}

/**
 * @brief  create a directory
 * @param  *muldir:directory name
 * @retval 0 if success
 */
int mk_mul_dirs(char *muldir)
{
	int i, len;
	char str[128];
	int ret = 0;

	strcpy(str, muldir);
	len = strlen(str);
	for (i = 0; i < len; i++) {
		if ((str[i] == '/') && i) {
			str[i] = '\0';
			if (access(str, 0) != 0) {
				ret = mkdir(str, 0777);
				if (ret != 0) {
					printf("Create multiple folder: %s fail\n", str);
					return -1;
				}
			}
			str[i] = '/';
		}
	}
	if (len > 0 && access(str, 0) != 0) {
		ret = mkdir(str, 0777);
		if (ret != 0) {
			printf("Create multiple folder: %s fail\n", str);
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  determine if the file exits
 * @param  *file_path: file path
 * @retval -1 if faile
 */
int is_file_exist(char *file_path)
{
	if (!file_path)
		return -1;

	if (access(file_path, F_OK) != -1)
		return 0;

	return -1;
}

/**
 * is_dir_exist - Check if the folder exists.
 * @dir_path: Folder path.
 *
 * Returns 0 if the folder exists, otherwise not 0.
 */
int is_dir_exist(char *dir_path)
{
	DIR *dir;

	if (!dir_path)
		return -1;

	dir = opendir(dir_path);
	if (!dir)
		return -1;

	closedir(dir);

	return 0;
}

/**
 * create_dir - create the folder exists.
 * @dir_path: Folder path.
 *
 * Returns 0 if create success, otherwise not 0.
 */
int create_dir(const char *path)
{
	if (access(path, R_OK | W_OK | X_OK))
		return mkdir(path, 0755);
	return 0;
}

/**
 * get_filename_ext - Gettng file extension.
 * @file_name file name
 *
 * Returns file extension.
 */
int get_filename_ext(const char *filename, char *ext, int ext_max_len)
{
	int ret = 0;
	const char *dot = NULL;

	assert((filename != NULL) && (ext != NULL));

	dot = strrchr(filename, '.');
	if (!dot || (dot == filename))
		ret = -1;
	else {
		ret = 0;
		snprintf(ext, ext_max_len, "%s", dot + 1);
	}

	return ret;
}

/**
 * check_file_is_jpg - Check if the picture is JPG
 * @file_name file name
 *
 * Returns 0 if it is JPG, otherwise not 0.
 */
int check_file_is_jpg(char *file_name)
{
	int ret = -1;
	int cmp_ret = 0;
	char ext[64];
	char jpg_ext[] = "jpg";

	assert(file_name != NULL);

	memset(ext, 0, sizeof(ext));
	ret = get_filename_ext(file_name, ext, sizeof(ext));
	if (0 == ret) {
		cmp_ret = strcasecmp(ext, jpg_ext);
		if (0 == cmp_ret)
			ret = 0;
		else
			ret = -1;
	}

	return ret;
}

/**
 * remove_pic_by_path - remove all pic file by path.
 * @len: input length of string path.
 * @path: A pointer to the string.
 *
 * Returns 0 if success, otherwise not 0.
 */
int remove_pic_by_path(char *path)
{
	int ret = -1;

	if (path == NULL)
		return -1;

	if (0 != is_file_exist(path)) {
		printf("[remove_pic_by_path]file %s not exist.\n", path);
		return -1;
	}

	if (0 != check_file_is_jpg(path)) {
		printf("[remove_pic_by_path]file %s not jpg ext.\n", path);
		return -1;
	}

	ret = remove(path);
	if (ret)
		printf("[remove_pic_by_path]remove %s error.\n", path);

	return ret;
}

/**
 * file_system_sync - Put the memory resident information about the file system into
 * the physical medium.
 * None
 *
 * Returns 0 if success, otherwise not 0.
 */
int file_system_sync(void)
{
	int ret = -1;

	ret = system("sync");
	if (ret != 0)
		printf("sync error\n");

	return ret;
}

