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
#include <aui_common_list.h>
#include "aui_log.h"
#include <alipltfretcode.h>
#include <pthread.h>
#include <syslog.h>

#ifdef _RD_DEBUG_

#define LOG_NONE (-1)

#define AUI_MODULE_TAG (1<<31)

#define AUI_LOG_INIT_ONCE() do { \
            if (g_aui_log_init_flag == 0) { \
                if (aui_log_init()==AUI_RTN_SUCCESS) \
                    g_aui_log_init_flag = 1; \
            } \
        } while (0)

static int g_aui_log_init_flag = 0;

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

/*
 * When multi AUI modules set the same SL module's level, keep the highest value
 */
static int alisl_module_dbg_level_init[NUM_OF_SL_MODULES] = { LOG_NONE };

#undef AUI_MOD_DEF
#undef SL_MOD

/*
 * link with sharelib for each module
 */
#define AUI_MOD_DEF(x, level) (AUI_MODULE_##x | AUI_MODULE_TAG),
#define SL_MOD(x) ALIPLTF_##x,

static unsigned int module_links[] = {
    #include "aui_modules.def"
};

#undef AUI_MOD_DEF
#undef SL_MOD


/* debug history to dump before the error */
//#define BACKLOG_MAX 40

#ifdef BACKLOG_MAX
struct log {
    struct aui_list_head list;
    int module;
    int prio;
    char str;
};
static struct log logs;
static int logCnt = -1;

static pthread_mutex_t mutex;
static pthread_mutexattr_t mutexAttr;
#endif


/* ali share library initialize function */
extern int alisl_log_init(void);
extern int alisl_log_set_priority(int module, int prio);

// magic setting for CI test
static int g_aui_log_print_output_only = 0;
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

#define PRINT_CONS(prio, str) if(g_aui_log_print_output_only) printf("%s %s", aui_log_header[prio], str)

#define PRINT(prio, str) if(g_aui_log_print_output_only==0) syslog(prio, "%s", str)

AUI_RTN_CODE aui_log_init(void)
{
    int prio = LOG_NONE;
    unsigned int v, i;
    char str[50], *s;

#ifdef BACKLOG_MAX
    AUI_INIT_LIST_HEAD(&logs.list);
    logCnt = 0;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    if (pthread_mutex_init(&mutex, &mutexAttr))
        return -1;
#endif

    openlog("AUI", LOG_CONS, LOG_USER);

    for (i = 0; i < AUI_MODULE_LAST; i++) {
        snprintf(str, sizeof(str), "%s_DBG_LEVEL", module_name[i]);
        s = getenv(str);
        if (s) {
            aui_module_dbg_level[i] = atoi(s);
            printf("set %s debug level %d\n", module_name[i],
                   aui_module_dbg_level[i]);
        }
    }

    alisl_log_init();

    for (i = 0; i < sizeof(module_links) / sizeof(int); i++) {
        v = module_links[i];
        if (v & AUI_MODULE_TAG) {
            /* AUI_MOD_DEF() */
            int aui_mod = v & ~AUI_MODULE_TAG;
            prio = aui_module_dbg_level[aui_mod];
        } else {
            /* SL_MOD() */
            if (prio == LOG_NONE)
                continue;

            // When multi AUI modules set the same SL module's level, keep the highest value
            if (prio > alisl_module_dbg_level_init[v]) {
                alisl_log_set_priority(v, prio);
                alisl_module_dbg_level_init[v] = prio;
            }
        }
    }
    return 0;
}

AUI_RTN_CODE aui_log_priority_set(int module, int prio)
{
    int priority = AUI_LOG_PRIO_ERR;
    int i = 0;
    int mod = 0;
    int link_size = sizeof(module_links) / sizeof(int);

    // Check and try to initialize AUI log once before using
    AUI_LOG_INIT_ONCE();

    // The following setting is for CI test only, use print to output the log, so that CI can get the output and send to server.
    if ((module == -1) && (prio == -1)) {
        // magic setting for CI test
        g_aui_log_print_output_only = 1;
        alisl_log_set_priority(module, prio);
        return AUI_RTN_SUCCESS;
    }
    if ((module == -1) && (prio == 0)) {
        // magic setting for CI test
        g_aui_log_print_output_only = 0;
        alisl_log_set_priority(module, prio);
        return AUI_RTN_SUCCESS;
    }
    
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
    {
        return AUI_RTN_FAIL;
    }
    
    aui_module_dbg_level[module] = priority;

    // Find the module
    for (i = 0; i < link_size; i++) {
        mod = module_links[i];
        if ((mod & AUI_MODULE_TAG) && ((mod & ~AUI_MODULE_TAG) == module)) {
            break;
        }
    }

    if (i == link_size)
        return AUI_RTN_FAIL;

    i += 1; // Next
    // Set linked SL module
    for (;i < link_size; i++) {
        mod = module_links[i];

        if (mod & AUI_MODULE_TAG) {// AUI module 
            break;
        } else {
            alisl_log_set_priority(mod, priority);
        }
    }

    return AUI_RTN_SUCCESS;
}

void aui_log(int module, int prio, int line,
         const char *func, int err, const char *fmt, ...)
{
    char str[301];
    va_list args;

    // Check and try to initialize AUI log once before using
    AUI_LOG_INIT_ONCE();

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

#ifdef BACKLOG_MAX
    if (logCnt < 0)
        goto skip_logs_list;

    pthread_mutex_lock(&mutex);
    if (prio <= mod_prio) {
        struct log *s, *tmp;
        aui_list_for_each_entry_safe(s, tmp, &logs.list, list) {
            PRINT_CONS(s->prio, &s->str);
            PRINT(s->prio, &s->str);
            aui_list_del(&s->list);
            free(s);
        }
        logCnt = 0;
    } else {
        struct log *log;
        if (logCnt >= BACKLOG_MAX) { /* remove oldest log */
            log = container_of(logs.list.next, struct log, list);
            aui_list_del(&log->list);
            free(log);
            logCnt--;
        }
        log = (struct log *)malloc(sizeof(struct aui_list_head) +
                       strlen(str) + 1);

        if (log) {
            strcpy(&log->str, str);
            log->module = module;
            log->prio = prio;
            aui_list_add_tail(&log->list, &logs.list);
            logCnt++;
        }
    }
    pthread_mutex_unlock(&mutex);

skip_logs_list:
#endif
    PRINT_CONS(prio, str);
    PRINT(prio, str);
}

void aui_log_dump(int module, int line, const char *func,
          const char *prompt, char *data, int len)
{
    int k, l = len;
    char str[200], s[20];

    // Check and try to initialize AUI log once before using
    AUI_LOG_INIT_ONCE();

    if ((module < 0) || (module >= AUI_MODULE_LAST))
        return;

    if (AUI_LOG_PRIO_DEBUG > aui_module_dbg_level[module])
        return;

    snprintf(str, sizeof(str), "%s %s:%d %s\n", module_name[module],
         func, line, prompt);
    PRINT_CONS(AUI_LOG_PRIO_DEBUG, str);
    PRINT(AUI_LOG_PRIO_DEBUG, str);

    str[0] = 0;
    for(k = 0; k < l; k++) {
        snprintf(s, 6, "0x%02X ",(unsigned char)data[k]);
        strncat(str, s, sizeof(str) - strlen(str));
        if ((k+1) % 16 == 0) {
            strncat(str, "\n", 1);
            PRINT_CONS(AUI_LOG_PRIO_DEBUG, str);
            PRINT(AUI_LOG_PRIO_DEBUG, str);
            str[0] = 0;
        }
    }
    if((k)%16!=0) {
        strncat(str, "\n", 1);
        PRINT_CONS(AUI_LOG_PRIO_DEBUG, str);
        PRINT(AUI_LOG_PRIO_DEBUG, str);
    }
}

void aui_log_close(void)
{
#ifdef BACKLOG_MAX
    struct log *s, *tmp;
    aui_list_for_each_entry_safe(s, tmp, &logs.list, list) {
        aui_list_del(&s->list);
        free(s);
    }
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&mutexAttr);
#endif
    closelog();
}

#else

AUI_RTN_CODE aui_log_init(void)
{
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_log_priority_set(int module, int prio)
{
    (void)module;
    (void)prio;
    return AUI_RTN_SUCCESS;
}


void aui_log_close(void)
{
}

#endif /* _RD_DEBUG_ */

