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
#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

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
					const char *file);

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
					   const char *file);

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
					 const char *file);

#ifdef __cplusplus
}
#endif

#endif
