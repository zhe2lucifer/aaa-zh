/*
 * TDS and Linux AUI logger functions
 *
 * Linux implements dynamic logging using syslog.
 * TDS implements static logging using compile switch.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <api/libc/printf.h>
#include <aui_common_list.h>
#include "aui_log.h"

// define in compiler.def
#ifdef _RD_DEBUG_

/*
 * name of each module
 */
#define AUI_MOD_DEF(x, level) "AUI_"#x,
#define SL_MOD(x)

static char *module_name[] = {
#include "aui_modules.def"
};

#undef AUI_MOD_DEF
#undef SL_MOD

/*
 * default log level of each module
 */
#define AUI_MOD_DEF(x, level) AUI_LOG_PRIO_##level,
#define SL_MOD(x)

static int aui_module_dbg_level[] = {
#include "aui_modules.def"
};

#undef AUI_MOD_DEF
#undef SL_MOD

#define LOG_NONE (-1)

#define AUI_MODULE_TAG (1<<31)


#undef AUI_MOD_DEF
#undef SL_MOD


//static char errcode[] = "_ACEWNID";
static char *aui_log_header[] = {
    "_", // 0
    "AUI.LOG.ALERT   :", // 1
    "AUI.LOG.CRITICAL:", // 2
    "AUI.LOG.ERROR   :", // 3
    "AUI.LOG.WARNING :", // 4
    "AUI.LOG.NOTICE  :", // 5
    "AUI.LOG.INFO    :", // 6
    "AUI.LOG.DEBUG   :", // 7
};

#define PRINT(prio, str) libc_printf("%s %s", aui_log_header[prio], str)


AUI_RTN_CODE aui_log_init(void)
{
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_log_priority_set(int module, int prio)
{
    int priority = AUI_LOG_PRIO_ERR;
    
    if ((module < 0) || (module >= AUI_MODULE_LAST))
        return AUI_RTN_EINVAL;

    switch (prio)
    {
        case AUI_LOG_PRIO_ALERT:
        case AUI_LOG_PRIO_CRIT:
        case AUI_LOG_PRIO_ERR:
            priority = AUI_LOG_PRIO_ERR;
            break;
        case AUI_LOG_PRIO_WARNING:
            priority = AUI_LOG_PRIO_WARNING;
            break;
        case AUI_LOG_PRIO_NOTICE:
        case AUI_LOG_PRIO_INFO:
            priority = AUI_LOG_PRIO_INFO;
            break;
        case AUI_LOG_PRIO_DEBUG:
            priority = AUI_LOG_PRIO_DEBUG;
            break;
        default:
            priority = -1;
            break;
    }

    if (priority < 0)
        return AUI_RTN_FAIL;
    
    aui_module_dbg_level[module] = priority;
    
    return AUI_RTN_SUCCESS;
}

void aui_log(int module, int prio, int line,
         const char *func, int err, const char *fmt, ...)
{
    char str[301];
    va_list args;

    if ((module < 0) || (module >= AUI_MODULE_LAST))
        return;

    if (prio > aui_module_dbg_level[module])
        return;

    snprintf(str, sizeof(str), "%s %s:%d", module_name[module], func, line);
    if (err)
        snprintf(str + strlen(str), sizeof(str) - strlen(str), " err %d", err);

    // If print content is not NULL
    if (fmt) {
        strncat(str, " ", 1);
        va_start(args, fmt);
        vsnprintf(str + strlen(str), sizeof(str) - strlen(str), fmt, args);
        va_end(args);
    } else {
        strncat(str, " (null)", 7);
    }
    
    if (str[strlen(str)-1] != '\n')
        strncat(str, "\n", 1);


    PRINT(prio, str);
}

void aui_log_dump(int module, int line, const char *func,
          const char *prompt, char *data, int len)
{
    int k, l = len;
    char str[200], s[20];

    if ((module < 0) || (module >= AUI_MODULE_LAST))
        return;

    if (AUI_LOG_PRIO_DEBUG > aui_module_dbg_level[module])
        return;

    snprintf(str, sizeof(str), "%s %s:%d %s\n", module_name[module],
         func, line, prompt);
    PRINT(AUI_LOG_PRIO_DEBUG, str);

    str[0] = 0;
    for(k = 0; k < l; k++) {
        snprintf(s, 5, "0x%02X,",(unsigned char)data[k]);
        strncat(str, s, sizeof(str) - strlen(str));
        if ((k+1) % 16 == 0) {
            strncat(str, "\n", 1);
            PRINT(AUI_LOG_PRIO_DEBUG, str);
            str[0] = 0;
        }
    }
    if((k)%16!=0) {
        strncat(str, "\n", 1);
        PRINT(AUI_LOG_PRIO_DEBUG, str);
    }
}

void aui_log_close(void)
{

}

#else

AUI_RTN_CODE aui_log_init(void)
{
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_log_priority_set(int module, int prio)
{
    return AUI_RTN_SUCCESS;
}


void aui_log_close(void)
{
}

#endif /* _RD_DEBUG_ */

