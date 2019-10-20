/*
 * Linux AUI logger header file
 */
#ifndef __AUI_LOG__
#define __AUI_LOG__

#include <aui_errno_stb.h>
#include <aui_common.h>

/**
    How to enable aui/aliplatform LOG in TDS

    1. You need to compile AUI with debug mode(_RD_DEBUG_ is defined)
        
    2. METHOD-1:

        You can set log level in your application by API aui_log_priority_set()

    3. In Sample code, you can set log level in 

        aui> sys
        aui> log DSC,DBG

    4. In CI test, you can reference to aui_nestor_test.py
*/

#ifdef _RD_DEBUG_

void aui_log(int module, int prio, int line,
	     const char *func, int err, const char *fmt, ...);

void aui_log_dump(int module, int line, const char *func,
		  const char *prompt, char *data, int len);

#define AUI_MODULE(x)	static int __aui_module = AUI_MODULE_##x;

/* macro in order of priority */
#define AUI_ERR(...)	aui_log(__aui_module, AUI_LOG_PRIO_ERR, \
				__LINE__, __func__, 0, __VA_ARGS__)
#define AUI_WARN(...)	aui_log(__aui_module, AUI_LOG_PRIO_WARNING, \
				__LINE__, __func__, 0, __VA_ARGS__)
#define AUI_INFO(...)	aui_log(__aui_module, AUI_LOG_PRIO_INFO, \
				__LINE__, __func__, 0, __VA_ARGS__)
#define AUI_API(...)	aui_log(__aui_module, AUI_LOG_PRIO_INFO, \
				__LINE__, __func__, 0, __VA_ARGS__)
#define AUI_DBG(...)	aui_log(__aui_module, AUI_LOG_PRIO_DEBUG, \
				__LINE__, __func__, 0, __VA_ARGS__)

#define AUI_DUMP(prompt, data, len) \
	aui_log_dump(__aui_module, __LINE__, __func__, prompt, data, len)

#ifdef AUI_LINUX
#define aui_rtn(err, ...)  do {						\
		aui_log(__aui_module, AUI_LOG_PRIO_ERR,			\
			__LINE__, __func__, err, __VA_ARGS__);		\
		return err;						\
	} while(0)
#else
#define aui_rtn(err, ...)  do {					\
		aui_log(__aui_module, AUI_LOG_PRIO_ERR,				\
			__LINE__, __func__, err, __VA_ARGS__);		\
		return err;						\
	} while(0)

#endif

#define AUI_LOG(module, prio, ...)			\
	aui_log(module, prio, __LINE__, __func__, 0, __VA_ARGS__)

/* deprecated macros */

#else /* DEBUG */

#define AUI_MODULE(...)

/* About how to enable log, please reference to the comment in the header of this file */

#define AUI_ERR(...) do { } while(0)
#define AUI_DBG(...) do { } while(0)
#define AUI_WARN(...) do { } while(0)
#define AUI_INFO(...) do { } while(0)
#define AUI_API(...) do { } while(0)

#define AUI_DUMP(...) do { } while(0)

#ifdef AUI_LINUX
#define aui_rtn(err, ...) return err
#else
#define aui_rtn(err, ...) return err
#endif

#define AUI_LOG(module, prio, ...) do { } while(0)

/* deprecated macros */
//when disable DEBUG, redefined with line 83
//#define aui_dbg_printf(module, level, ...) do { } while(0)

#endif /* _RD_DEBUG_ */

// Should not use these macros anymore
#define aui_dbg_printf(module, level, ...) do{ } while(0)
#define pr_debug(...) do { } while(0)
#define pr_info(...) do { } while(0)
#define pr_err(...) do { } while(0)

/* deprecated macros */
#define AUI_ENTER_FUNC(MOD_NAME) do { } while(0)
#define AUI_LEAVE_FUNC(MOD_NAME) do { } while(0)

#endif

