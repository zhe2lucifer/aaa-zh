/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       config.h
 *  @brief      ini configuration file read/write
 *
 *  @version    1.0
 *  @date       4/8/2014  16:2:8
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#include "aui_ini_config.h"

#define MAX_ITEM_NUM 500
#define DEFAULT_GROUP "[General]"

#define FREE_ONE_ITEM(item) \
    do {\
        free(item.key);\
        free(item.value);\
        free(item.group);\
        item.key = NULL;\
        item.value = NULL;\
        item.group = NULL;\
    } while(0)

#define WRITE_ONE_ITEM(fd, buf, item) \
    do {\
        memset(buf, 0, sizeof(buf));\
        if (item.group != NULL) {\
           if(-1 != write(fd, "\n", 1)){\
            sprintf(buf, "%s\n", item.group);\
		   }\
        } else {\
            sprintf(buf, "%s=%s\n", item.key, item.value);\
        }\
        if(-1 == write(fd, buf, strlen(buf))){\
		}\
    } while(0);


typedef struct item_t {
	char *key;
	char *value;
	char *group;
} ONE_ITEM;

/**
 *  strip right space
 *
 */
static char *strtrimr(char *pstr)
{
	int i;
	i = strlen(pstr) - 1;
	while (isspace(pstr[i]) && (i >= 0))
		pstr[i--] = '\0';
	return pstr;
}

/**
 *  strip left space
 *
 */
static char *strtriml(char *pstr)
{
	int i = 0, j;
	j = strlen(pstr) - 1;
	while (isspace(pstr[i]) && (i <= j))
		i++;
	if (0 < i)
		strcpy(pstr, &pstr[i]);
	return pstr;
}

/**
 *  strip right and left space
 *
 */
static char *strtrim(char *pstr)
{
	char *p;
	p = strtrimr(pstr);
	return strtriml(p);
}

/**
 *  return 0 succeed, otherwise failed.
 *
 */
static int lock_file(int lock, int fd)
{
	if (lock) {
		return flock(fd, LOCK_EX);
	} else {
		flock(fd, LOCK_UN);
		return 1;
	}
}

static int file_to_items(ONE_ITEM *items,
						 unsigned int max_item_size,
						 unsigned int *valid_item_size,
						 const char *file)
{
	char line[1024];
	FILE *fp;
	fp = fopen(file, "r");
	if (fp == NULL) {
		printf("[config] file_to_items fopen %s failed\n", file);
		return -1;
	}

	int file_fd = fileno(fp);
	if (lock_file(1, file_fd)) {
		printf("[config] file_to_items lock %s failed\n", file);
		fclose(fp);
		return -1;
	}

	unsigned int i = 0;
	while (fgets(line, 1023, fp)) {
		if (i >= max_item_size)
			break;
		memset(items + i, 0, sizeof(ONE_ITEM));//!!
		char *p = strtrim(line);
		int len = strlen(p);
		if (len <= 0) {
			continue;
		} else if (p[0] == '#') {
			continue;
		} else {
			if (p[0] == '[' && p[strlen(p) - 1] == ']') { //it's name of one group
				items[i].group = (char *) malloc(strlen(p) + 1);//so when group != NULL, it's a group name.
				strcpy(items[i].group, p);
				i++;
			} else {
				char *p2 = strchr(p, '=');
				if (p2 == NULL) {
					continue;
				} else {
					if (i == 0) {
						items[i].group = (char *) malloc(strlen(DEFAULT_GROUP)
														 + 1);
						strcpy(items[i].group, DEFAULT_GROUP);
						i++; // it will lead items[i] != NULL because not memset.
					}
					*p2++ = '\0';
					items[i].group = NULL; //!! note that it's not null because i++
					items[i].key = (char *) malloc(strlen(p) + 1);
					items[i].value = (char *) malloc(strlen(p2) + 64);
					strcpy(items[i].key, p);
					strcpy(items[i].value, p2);
					i++;
				}
			}
		}
	}

	(*valid_item_size) = i;
	lock_file(0, file_fd);
	fclose(fp);

	return 0;
}

/**
 *  @brief          get the settings from a ini file
 *
 *  @param[in]      key         index key
 *  @param[out]     value       setting value
 *  @param[in]      group       the settings group name
 *  @param[in]      file        the configuration file path
 *
 *  @return         0 - success
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           4/8/2014  16:26:20
 *
 *  @note
 */
int read_conf_value(const char *key,
					char *value,
					const char *group,
					const char *file)
{
	if (!key || !value || !group || !file) {
		return -1;
	}

	ONE_ITEM items[MAX_ITEM_NUM];
	unsigned int valid_size = 0;
	if (file_to_items(items, MAX_ITEM_NUM, &valid_size, file) < 0)
		return -1;

	unsigned int i = 0;
	int bFindGroup = 0;
	for (i = 0; i < valid_size; i++) {
		if (items[i].group != NULL) { //it's a group
			if (bFindGroup == 1) //the next group appear, so end.
				break;
			if (!strcmp(items[i].group, group)) {
				bFindGroup = 1;
			}
		} else if ((bFindGroup == 1) && (!strcmp(items[i].key, key))) { //it's a key map
			strcpy(value, items[i].value);
			break;
		}
	}

	for (i = 0; i < valid_size; i++) {
		FREE_ONE_ITEM(items[i]);
	}
	return 0;
}

/**
 *  @brief          get the settings from a ini file
 *
 *  @param[in]      key         index key
 *  @param[out]     value       setting value
 *  @param[in]      group       the settings group name
 *  @param[in]      value_len   the length of the value
 *  @param[in]      file        the configuration file path
 *
 *  @return         0 - success
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           4/8/2014  16:26:20
 *
 *  @note
 */
int read_conf_value_ex(const char *key,
					   char *value,
					   const char *group,
					   int value_len,
					   const char *file)
{
	if (!key || !value || !group || !file) {
		return -1;
	}

	ONE_ITEM items[MAX_ITEM_NUM];
	unsigned int valid_size = 0;
	if (file_to_items(items, MAX_ITEM_NUM, &valid_size, file) < 0)
		return -1;

	unsigned int i = 0;
	int bFindGroup = 0;
	for (i = 0; i < valid_size; i++) {
		if (items[i].group != NULL) { //it's a group
			if (bFindGroup == 1) //the next group appear, so end.
				break;
			if (!strcmp(items[i].group, group)) {
				bFindGroup = 1;
			}
		} else if ((bFindGroup == 1) && (!strcmp(items[i].key, key))) { //it's a key map
			strncpy(value, items[i].value, value_len);
			break;
		}
	}

	for (i = 0; i < valid_size; i++) {
		FREE_ONE_ITEM(items[i]);
	}
	return 0;
}

/**
 *  @brief          save configuration to a ini file
 *
 *  @param[in]      key     index key
 *  @param[in]      value   the value want to set
 *  @param[in]      group   the setting group
 *  @param[in]      file    the configuration file path
 *
 *  @return         0 - success
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           4/8/2014  16:24:11
 *
 *  @note           file will be created if it's not exist.
 */
int write_conf_value(const char *key,
					 const char *value,
					 const char *group,
					 const char *file)
{
	if (!key || !value || !group || !file) {
		return -1;
	}

	ONE_ITEM items[MAX_ITEM_NUM];
	unsigned int valid_size = 0;
	int bfind = 0;
	int find_group = 0;

	unsigned int find_index = 0;
	//why MAX_ITEM_NUM - 2: reserver 2 ONE_ITEM to write back.
	if (file_to_items(items, MAX_ITEM_NUM - 2, &valid_size, file) == 0) {
		for (find_index = 0; find_index < valid_size; find_index++) {
			if (items[find_index].group != NULL) { //it's a group name
				if (find_group == 1) //the next group occur, so end.
					break;
				if (!strcmp(items[find_index].group, group)) {
					//printf("[config] write_conf_value find group %s\n", group);
					find_group = 1;
				}
			} else if (find_group == 1) { //have find the group, look for key.
				if (!strcmp(items[find_index].key, key)) {
					strcpy(items[find_index].value, value); //got it and write value.
					//printf("write key: %s  = %s\n", key, value);
					bfind = 1;
					break;
				}
			}
		}
	}

	int add_inser_item_flag = 0;
	ONE_ITEM inser_item;
	inser_item.key = NULL;
	inser_item.value = NULL;
	if (!bfind) {
		if (find_group == 1) { //find the group but not find key. so write new value in this group.
			add_inser_item_flag = 1;
			inser_item.key = (char *) malloc(strlen(key) + 1);
			inser_item.value = (char *) malloc(strlen(value) + 1);
			strcpy(inser_item.key, key);
			strcpy(inser_item.value, value);
			inser_item.group = NULL;
		} else {
			items[valid_size].group = (char *) malloc(strlen(group) + 1); //no this group, so add it.
			strcpy(items[valid_size].group, group);
			items[valid_size].key = NULL;
			items[valid_size].value = NULL;
			valid_size++;

			items[valid_size].key = (char *) malloc(strlen(key) + 1); //add key and value in this group
			items[valid_size].value = (char *) malloc(strlen(value) + 1);
			strcpy(items[valid_size].key, key);
			strcpy(items[valid_size].value, value);
			items[valid_size].group = NULL;
			valid_size++;
		}
	}

	int fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 666);
	if (fd < 0) {
		printf("[config] write_conf_value open %s failed\n", file);
		return -1;
	}

	if (lock_file(1, fd)) {
		printf("[config] write_conf_value lock %s failed\n", file);
		close(fd);
		return -1;
	}

	unsigned int i = 0;
	char buf[1024] = {0};
	if (add_inser_item_flag != 1) {
		for (i = 0; i < valid_size; i++) {
			WRITE_ONE_ITEM(fd, buf, items[i]);
		}
	} else {
		for (i = 0; i < find_index; i++) {
			WRITE_ONE_ITEM(fd, buf, items[i]);
		}
		WRITE_ONE_ITEM(fd, buf, inser_item);
		for (i = find_index; i < valid_size; i++) {
			WRITE_ONE_ITEM(fd, buf, items[i]);
		}
	}

	fsync(fd);
	lock_file(0, fd);
	close(fd);

	for (i = 0; i < valid_size; i++)
		FREE_ONE_ITEM(items[i]);
	if (add_inser_item_flag == 1)
		FREE_ONE_ITEM(inser_item);

	return 0;
}
