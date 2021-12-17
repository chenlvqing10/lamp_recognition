/*
 * file_ops.h
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

#ifndef __FILE_OPS_H__
#define __FILE_OPS_H__

/*
* file_read - read data from file
* @file_path path of  file
* @buf data buf
* @buf_len   data buf length
* return data len if success, otherwise return -1
*/
int file_read(char *file_path, char *buf, int buf_len);
/*
* file_copy - copy source file to destination file
* @src_file_path path of source file
* @dest_file_path path of destination file
* return 0 if success, otherwise return -1
*/
int file_copy(char *src_file_path, char *dest_file_path);

/*
* file_move - move src_file_path to dest_file_path
* @src_file_path path of source file
* @dest_file_path path of destination file
* return 0 if success, otherwise return -1
*/
int file_move(char *src_file_path, char *dest_file_path);

/*
* file_wirte - wirte buffer to destination file
* @file_path path of destination file
* @buf data buffer
* @size buffer size
* return 0 if success, otherwise return -1
*/
int file_write(char *file_path, char *buf, int size);

/*
* file_length - get length of file
* @file_path path of file
* return -1 if failed, otherwise return length
*/
int file_length(char *file_path);

/**
 * file_remove - delete file.
 * @file_path: file path.
 *
 * Returns 0 if delete success, otherwise not 0.
 */
int file_remove(char *file_path);

/*
* mk_mul_dirs - make directories
* @file_path path of directory
* return -1 if failed, otherwise return 0
*/
int mk_mul_dirs(char *muldir);

/*
* is_file_exist -check whether file exists or not
* @file_path path of file
* return 0 if file exists, otherwise return -1
*/
int is_file_exist(char *file_path);

/**
 * is_dir_exist - Check if the folder exists.
 * @dir_path: Folder path.
 *
 * Returns 0 if the folder exists, otherwise not 0.
 */
int is_dir_exist(char *dir_path);

/**
 * create_dir - create the folder exists.
 * @dir_path: Folder path.
 *
 * Returns 0 if create success, otherwise not 0.
 */
int create_dir(const char *path);

/**
 * get_filename_ext - Gettng file extension.
 * @file_name file name
 *
 * Returns file extension.
 */
int get_filename_ext(const char *filename, char *ext, int ext_max_len);
/**
 * check_file_is_jpg - Check if the picture is JPG
 * @file_name file name
 *
 * Returns 0 if it is JPG, otherwise not 0.
 */
int check_file_is_jpg(char *file_name);

/**
 * remove_pic_by_path - remove all pic file by path.
 * @path: A pointer to the string.
 * @len: input length of string path.
 *
 * Returns 0 if success, otherwise not 0.
 */
int remove_pic_by_path(char *path);

/**
 * file_system_sync - Put the memory resident information about the file system into
 * the physical medium.
 * None
 *
 * Returns 0 if success, otherwise not 0.
 */
int file_system_sync(void);

#endif

