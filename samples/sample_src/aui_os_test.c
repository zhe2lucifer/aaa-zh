/****************************INCLUDE HEAD FILE************************************/
#include <osal/osal.h>
#include <stdio.h>
#include <string.h>
#include "aui_os_test.h"
#include "aui_help_print.h"

/****************************LOCAL MACRO******************************************/
#define OS_SND_INJECT_ONCE_LEN	(64*1024)

//typedef unsigned short	WORD;
//typedef WORD			ID;

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

/****************************LOCAL FUNC DECLEAR***********************************/

/****************************EXTERN VAR*******************************************/
void *g_p_hdl_task0=NULL;
void *g_p_hdl_task1=NULL;
void *g_p_hdl_task2=NULL;
void *g_p_hdl_task_self=NULL;

void *g_p_hdl_msgQ=NULL;
void *g_p_hdl_sem=NULL;
void *g_p_hdl_event=NULL;
void *g_p_hdl_mutex=NULL;
void *g_p_hdl_timer=NULL;

ID g_mutex_id=OSAL_INVALID_ID;
void *g_p_hdl_msgQTest=NULL;
void *g_p_hdl_mutex_test=NULL;
ID s_flagIDDemo=OSAL_INVALID_ID;
ID s_flagIDDemo2=OSAL_INVALID_ID;
/****************************EXTERN FUNC *****************************************/
//extern RET_CODE win_es_player_audio_init(ali_audio_config *dec_config, OSAL_ID	*tsk_id);


/****************************LOCAL FUNC DECLEAR***********************************/
AUI_RTN_CODE testT1(void *pv_para1,void *pv_para2)
{
	static int i=0;
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		if(i++>10)
		{
			break;
		}
		osal_task_sleep(10000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
            		AUI_PRINTF("\r\n T1 exit failed.");
			return -1;
		}
		
        	AUI_PRINTF("T1");
		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, OSAL_WAIT_FOREVER_TIME))
		{
			AUI_PRINTF("\r\n lock failed.");
		}
		
        	AUI_PRINTF("\r\n T1 be unlock by T2.");
/*
		AUI_PRINTF("\r\nT1:[%08x][%s][%s]",(unsigned long)hdl_tmp,
					(char *)((((aui_hdl_task *)g_p_hdl_task0)->task_attr).sz_name),
					(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/		
	}
	
    	AUI_PRINTF("\r\n T1 exit success.");
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE testT2(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		osal_task_sleep(4000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
            		AUI_PRINTF("\r\n T2 exit failed.");
			return -1;
		}
		if(0!=aui_os_mutex_unlock(g_p_hdl_mutex_test))
		{
			AUI_PRINTF("\r\n unlock failed.");
		}
		
    	AUI_PRINTF("T2 unlock successed");
	}
}
AUI_RTN_CODE testT3(void *pv_para1,void *pv_para2)
{
	while(1)
	{
		osal_task_sleep(8000);
		AUI_PRINTF("3");
	}
}

void test_timerCB1(unsigned int uiP1)
{
	AUI_PRINTF("\r\n CB1:uiP1=[%08x]",uiP1);
}
void test_timerCB2(unsigned int uiP1)
{
	AUI_PRINTF("\r\n CB2:uiP1=[%08x]",uiP1);
}
unsigned long test_os_init(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_attr_os attr_os;

    MEMSET(&attr_os,0,sizeof(aui_attr_os));
    attr_os.task_rtn_mode=AUI_TASK_RTN_MODE_MANUAL_FREE;
    attr_os.event_num=256;

    if(0!=aui_os_init(&attr_os))
    {
    	return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}
unsigned long test_os_task_create(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_task attr_task;
	aui_attr_mutex attr_mutex;
	MEMSET(&attr_task,0,sizeof(aui_attr_task));
	MEMSET(&attr_mutex,0,sizeof(aui_attr_mutex));

	if(NULL==g_p_hdl_mutex_test)
	{
		strcpy(attr_mutex.sz_name,"mutexttest");
		attr_mutex.mutex_type=argv[0][0]-'0';
		AUI_PRINTF("\r\nmutex type=[%d]",attr_mutex.mutex_type);
		if(0!=aui_os_mutex_create(&attr_mutex,&g_p_hdl_mutex_test))
		{
			return -1;
		}
	}
	if(0==g_mutex_id)
	{
		g_mutex_id=osal_mutex_create();
		if(OSAL_INVALID_ID==g_mutex_id)
		{
			return -1;
		}
	}
	if(0==strcmp(argv[1],"1"))
	{
		strcpy(attr_task.sz_name,"111111");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=testT1;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task0));	
	}
	else if(0==strcmp(argv[1],"2"))
	{
		strcpy(attr_task.sz_name,"222222");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=testT2;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	}	
	//attr_task.p_fun_task_entry=testT2;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	//attr_task.p_fun_task_entry=testT3;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task2));	
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_task_delete(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task0,FALSE));
	AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1,TRUE));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task2));
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_task_self_id(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_task_get_self_hdl(&g_p_hdl_task_self));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task2));
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_task_join(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_task_join(g_p_hdl_task0,12000));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task2));
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_msgQCreate(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_msgq attr_msgQ;
	MEMSET(&attr_msgQ,0,sizeof(aui_attr_msgq));

	strcpy(attr_msgQ.sz_name,"msgq1");
	attr_msgQ.ul_buf_size=0x100;
	attr_msgQ.ul_msg_size_max=50;
	AUI_TEST_CHECK_RTN(aui_os_msgq_create(&attr_msgQ,&g_p_hdl_msgQ));	
	//attr_task.p_fun_task_entry=testT2;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	//attr_task.p_fun_task_entry=testT3;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task2));	
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_msgQDelete(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_msgq_delete(&g_p_hdl_msgQ));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task2));
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_msgQSnd(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_msgq_snd(g_p_hdl_msgQ,argv[0],strlen(argv[0]),10));
	return AUI_RTN_SUCCESS;

}
unsigned long test_os_msgQRcv(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_rcv_msg_len=0;
	char ac_tmp[101]={0};
	
	AUI_TEST_CHECK_RTN(aui_os_msgq_rcv(g_p_hdl_msgQ,ac_tmp,100,&ul_rcv_msg_len,10));
	AUI_PRINTF("\r\nrcv=[%s]",ac_tmp);
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_sem_create(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_sem attr_sem;
	unsigned long ul_init_val=0;
	unsigned long ul_max_val=0;
	MEMSET(&attr_sem,0,sizeof(aui_attr_sem));

	ul_init_val = ATOI(argv[0]);
	ul_max_val = ATOI(argv[1]);

	strcpy(attr_sem.sz_name,"sem1");
	attr_sem.ul_init_val=ul_init_val;
	attr_sem.ul_max_val=ul_max_val;
	AUI_TEST_CHECK_RTN(aui_os_sem_create(&attr_sem,&g_p_hdl_sem));	
	//attr_task.p_fun_task_entry=testT2;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	//attr_task.p_fun_task_entry=testT3;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task2));	
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_sem_delete(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_sem_delete(&g_p_hdl_sem));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task2));
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_sem_wait(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_time_out=0;
	
	ul_time_out = ATOI(argv[0]);
	
	AUI_PRINTF("\r\nsem wait start: %dms", ul_time_out);
	AUI_TEST_CHECK_RTN(aui_os_sem_wait(g_p_hdl_sem,ul_time_out));
	AUI_PRINTF("\r\nsem wait end");
	return AUI_RTN_SUCCESS;

}
unsigned long test_os_sem_release(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_PRINTF("\r\nsem release start");
	AUI_TEST_CHECK_RTN(aui_os_sem_release(g_p_hdl_sem));
	AUI_PRINTF("\r\nsem release end");
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_event_create(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_event attr_event;
	unsigned long ul_init_val=0;
	unsigned long ul_max_val=0;
	MEMSET(&attr_event,0,sizeof(aui_attr_event));

	ul_init_val = ATOI(argv[0]);
	ul_max_val = ATOI(argv[1]);

	strcpy(attr_event.sz_name,"event1");
	attr_event.b_auto_reset=ul_init_val;
	attr_event.b_initial_state=ul_max_val;
	AUI_TEST_CHECK_RTN(aui_os_event_create(&attr_event,&g_p_hdl_event));	
	//attr_task.p_fun_task_entry=testT2;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	//attr_task.p_fun_task_entry=testT3;
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task2));	
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_event_delete(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_event_delete(&g_p_hdl_event));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1));
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task2));
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_event_wait(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_time_out=0;

	ul_time_out = ATOI(argv[0]);
	
	AUI_PRINTF("\r\nevent clear start, waittime: %dms", ul_time_out);
	AUI_TEST_CHECK_RTN(aui_os_event_wait(g_p_hdl_event,ul_time_out,0));
	AUI_PRINTF("\r\nevent clear end");
	return AUI_RTN_SUCCESS;

}
unsigned long test_os_event_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_PRINTF("\r\nevent set start");
	AUI_TEST_CHECK_RTN(aui_os_event_set(g_p_hdl_event, 0));
	AUI_PRINTF("\r\nevent set end");
	return AUI_RTN_SUCCESS;
}

unsigned long test_os_mutex_create(unsigned long *argc,char **argv,char *sz_out_put)
{                                                                                
	aui_attr_mutex attr_mutex;                                                       
	unsigned long ul_init_val=0;                                                     
	MEMSET(&attr_mutex,0,sizeof(aui_attr_mutex));                                    

	ul_init_val = ATOI(argv[0]);
	
	strcpy(attr_mutex.sz_name,"mutex1");                                             
	attr_mutex.mutex_type=ul_init_val;                                                
                                           
	AUI_TEST_CHECK_RTN(aui_os_mutex_create(&attr_mutex,&g_p_hdl_mutex_test));	             
	//attr_task.p_fun_task_entry=testT2;                                               
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	             
	//attr_task.p_fun_task_entry=testT3;                                               
	//AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task2));	             
	return AUI_RTN_SUCCESS;                                                        
}                                                                                
unsigned long test_os_mutex_delete(unsigned long *argc,char **argv,char *sz_out_put)
{                                                                                
	AUI_TEST_CHECK_RTN(aui_os_mutex_delete(&g_p_hdl_mutex_test));                          
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1));                         
	//AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task2));                         
	return AUI_RTN_SUCCESS;                                                        
}                                                                                
unsigned long test_os_mutex_lock(unsigned long *argc,char **argv,char *sz_out_put) 
{                                                                                
	unsigned long ul_time_out=0;        

	ul_time_out = ATOI(argv[0]);
	
	AUI_PRINTF("\r\nmutex lock start");                                          
	AUI_TEST_CHECK_RTN(aui_os_mutex_lock(g_p_hdl_mutex_test,ul_time_out));                  
	AUI_PRINTF("\r\nmutex lock end");                                            
	return AUI_RTN_SUCCESS;                                                        
                                                                                 
}                                                                                
unsigned long test_os_mutex_unlock(unsigned long *argc,char **argv,char *sz_out_put)   
{                                                                                
	AUI_PRINTF("\r\nmutex unlock start");                                            
	AUI_TEST_CHECK_RTN(aui_os_mutex_unlock(g_p_hdl_mutex_test));                         
	AUI_PRINTF("\r\nmutex unlock end");                                              
	return AUI_RTN_SUCCESS;                                                        
}  

unsigned long test_os_mutex_try_lock(unsigned long *argc,char **argv,char *sz_out_put)   
{                                                                                
	aui_attr_mutex attr_mutex;

	MEMSET(&attr_mutex,0,sizeof(aui_attr_mutex));

	AUI_PRINTF("\r\nmutex try lock start");                                            
	//AUI_TEST_CHECK_RTN(aui_os_mutex_try_lock(g_p_hdl_mutex));    
	if(0==strcmp(argv[0],"0"))
	{

		if(NULL==g_p_hdl_mutex_test)
		{
			#if 1
			strcpy(attr_mutex.sz_name,"mutexttest");

			attr_mutex.mutex_type=argv[0][0]-'0';
			AUI_PRINTF("\r\nmutex type=[%d]",attr_mutex.mutex_type);
			if(0!=aui_os_mutex_create(&attr_mutex,&g_p_hdl_mutex_test))
			{
				//return -1;
			}
			#else
			g_p_hdl_mutex_test=osal_mutex_create();
			#endif
		}
		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, 1000))
		{
			AUI_PRINTF("\r\n1");
			////return -1;
		}
		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, 1000))
		{
			AUI_PRINTF("\r\n2");
			////return -1;
		}
		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, 1000))
		{
			AUI_PRINTF("\r\n3");
			//return -1;
		}
		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, 1000))
		{
			AUI_PRINTF("\r\n4");
			//return -1;
		}
		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, 1000))
		{
			AUI_PRINTF("\r\n5");
			//return -1;
		}
		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, 1000))
		{
			AUI_PRINTF("\r\n6");
			//return -1;
		}
		if(0!=aui_os_mutex_unlock(g_p_hdl_mutex_test))
		{
			AUI_PRINTF("\r\n7");
			//return -1;
		}
		AUI_PRINTF("\r\n000mutex try lock end");  


	}
	else if(0==strcmp(argv[0],"1"))
	{
		if(0==g_mutex_id)
		{
			g_mutex_id=osal_mutex_create();
			if(OSAL_INVALID_ID==g_mutex_id)
			{
				//return -1;
			}
		}

		if(0!=osal_mutex_lock(g_mutex_id, 1000))
		{
			AUI_PRINTF("\r\na");
			//return -1;
		}
		if(0!=osal_mutex_lock(g_mutex_id, 1000))
		{
			AUI_PRINTF("\r\nb");
			//return -1;
		}
		if(0!=osal_mutex_lock(g_mutex_id, 1000))
		{
			AUI_PRINTF("\r\nc");
			//return -1;
		}
		if(0!=osal_mutex_lock(g_mutex_id, 1000))
		{
			AUI_PRINTF("\r\nd");
			//return -1;
		}
		if(0!=osal_mutex_lock(g_mutex_id, 1000))
		{
			AUI_PRINTF("\r\ne");
			//return -1;
		}
		if(0!=osal_mutex_lock(g_mutex_id, 1000))
		{
			AUI_PRINTF("\r\nf");
			//return -1;
		}
		if(0!=osal_mutex_unlock(g_mutex_id))
		{
			AUI_PRINTF("\r\ng");
			//return -1;
		}
		

	}
	return AUI_RTN_SUCCESS;                                                        
}                                                                                    
                                                                                 
unsigned long test_os_timer_create(unsigned long *argc,char **argv,char *sz_out_put)
{                                                                                
	aui_attr_timer attr_timer;                                                       
	unsigned long ul_init_val=0;                                                     
	unsigned long ul_time_out=0;                                                     
	MEMSET(&attr_timer,0,sizeof(aui_attr_timer));                                    
                                                                                 
	strcpy(attr_timer.sz_name,argv[0]);   
	if(0==strcmp(argv[1],"TIMER_ALARM"))
	{
		attr_timer.ul_type=TIMER_ALARM;
		attr_timer.timer_call_back=test_timerCB1;   
	}
	else if(0==strcmp(argv[1],"TIMER_CYCLIC"))
	{
		attr_timer.ul_type=TIMER_CYCLIC;
		attr_timer.timer_call_back=test_timerCB2;   
	}

	ul_time_out = ATOI(argv[2]);
   	attr_timer.ul_time_out=ul_time_out;  
	ul_init_val = ATOI(argv[3]);
   	attr_timer.ul_para1=ul_init_val;  
    	
	AUI_TEST_CHECK_RTN(aui_os_timer_create(&attr_timer,&g_p_hdl_timer));	             
	             
	return AUI_RTN_SUCCESS;                                                        
}
unsigned long test_os_timer_delete(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_timer_delete(&g_p_hdl_timer));   
	return AUI_RTN_SUCCESS;  
}
unsigned long test_os_timer_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_val1=0;
	
	if(0==strcmp(argv[0],"DELAY"))
	{
	
		ul_val1 = ATOI(argv[1]);
		AUI_TEST_CHECK_RTN(aui_os_timer_set(g_p_hdl_timer,AUI_TIMER_DELAY_SET,(void *)(&ul_val1)));   
	}
	else if(0==strcmp(argv[0],"VAL"))
	{
		ul_val1 = ATOI(argv[1]);
		AUI_TEST_CHECK_RTN(aui_os_timer_set(g_p_hdl_timer,AUI_TIMER_VAL_SET,(void *)(&ul_val1)));  		
	}
	return AUI_RTN_SUCCESS;  
}
unsigned long test_os_timer_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_val1=0;
	
	if(0==strcmp(argv[0],"SECOND"))
	{

		AUI_TEST_CHECK_RTN(aui_os_timer_get(g_p_hdl_timer,AUI_TIMER_SECOND_GET,(void *)(&ul_val1)));  
		AUI_PRINTF("\r\n sec val=%d.",ul_val1);
	}
	else if(0==strcmp(argv[0],"TICK"))
	{
		AUI_TEST_CHECK_RTN(aui_os_timer_get(g_p_hdl_timer,AUI_TIMER_TICK_GET,(void *)(&ul_val1))); 
		AUI_PRINTF("\r\n tick val=%d.",ul_val1);
	}
	
	return AUI_RTN_SUCCESS;  
}	
unsigned long test_os_timer_run(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_timer_run(g_p_hdl_timer,argv[0][0]-'0'));   
	return AUI_RTN_SUCCESS;  
}	


////////////////////////////////////////FOR TDS DEMO///////////////////////////////////////////////
unsigned int s_set_flag_bit=0x01;
unsigned int s_wait_flag_bit=0x01;
unsigned int s_wait_mode=OSAL_TWF_ORW; 
unsigned long s_ul_clr_flag=0;
AUI_RTN_CODE demo_task1(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		osal_task_sleep(2000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}
/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task0)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/		
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE demo_task2(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		osal_task_sleep(3000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}
/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task1)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/		
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE demo_task3(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		osal_task_sleep(2000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}
/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task0)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/		
		aui_os_msgq_snd(g_p_hdl_msgQTest,"demo_task3",strlen("demo_task3"),1000);
		
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE demo_task4(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	char ac_tmp[100+1]={0};
	unsigned long ul_rcv_msg_len=0;
	
	while(1)
	{
		MEMSET(ac_tmp,0,101);
		osal_task_sleep(2000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}
/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task1)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/		
		
		AUI_TEST_CHECK_RTN(aui_os_msgq_rcv(g_p_hdl_msgQTest,ac_tmp,100,&ul_rcv_msg_len,OSAL_WAIT_FOREVER_TIME));
		AUI_PRINTF("\r\nrcv=[%d][%s]",ul_rcv_msg_len,ac_tmp);
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE demo_task5(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		osal_task_sleep(4000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}

		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, OSAL_WAIT_FOREVER_TIME))
		{
			AUI_PRINTF("\r\n lock failed.");
		}

/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task0)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/		
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE demo_task6(void *pv_para1,void *pv_para2)
{
	static int i=0;
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		if(i++>10)
		{
			break;
		}
		osal_task_sleep(2000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}
		if(0!=aui_os_mutex_unlock(g_p_hdl_mutex_test))
		{
			AUI_PRINTF("\r\n unlock failed.");
		}
/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task1)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/												
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE demo_task7(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	while(1)
	{
		osal_task_sleep(6000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}

		if(0!=osal_flag_set(s_flagIDDemo, s_set_flag_bit))
		{
			AUI_PRINTF("\r\n flag set failed.");
		}

/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task0)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/		
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE demo_task8(void *pv_para1,void *pv_para2)
{
	aui_hdl hdl_tmp=NULL;
	UINT flgptn=0; 
	TMO tmout=OSAL_WAIT_FOREVER_TIME;
	while(1)
	{
		osal_task_sleep(2000);
		if(0!=aui_os_task_get_self_hdl(&hdl_tmp))
		{
			AUI_PRINTF("\r\n[%s] get self handle failed.",__FUNCTION__);
			return -1;
		}
		
		if(0!=osal_flag_wait(&flgptn,s_flagIDDemo, s_wait_flag_bit,s_wait_mode,tmout))
		{
			AUI_PRINTF("\r\n flag wait failed.");
		}
		if(s_ul_clr_flag)
		{
			if(0!=osal_flag_clear(s_flagIDDemo, s_wait_flag_bit))
			{
				AUI_PRINTF("\r\n flag clr failed.");
			}
		}
/*
		AUI_PRINTF("\r\n[%s]:[%08x][%d][%s][%s]",__FUNCTION__,
												(unsigned long)hdl_tmp,(((aui_hdl_task *)hdl_tmp)->ul_id),
												(char *)((((aui_hdl_task *)g_p_hdl_task1)->task_attr).sz_name),
												(char *)((((aui_hdl_task *)hdl_tmp)->task_attr).sz_name));
*/
	}
}
static int s_i_test=0;
AUI_RTN_CODE demo_task9(void *pv_para1,void *pv_para2)
{
	while(1)
	{
		osal_task_sleep(2000);

		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, OSAL_WAIT_FOREVER_TIME))
		{
			AUI_PRINTF("\r\n lock failed.");
		}
		s_i_test++;
		s_i_test++;

		AUI_PRINTF("\r\n[%s]:[%d]",__FUNCTION__,s_i_test);
		if(0!=aui_os_mutex_unlock(g_p_hdl_mutex_test))
		{
			AUI_PRINTF("\r\n unlock failed.");
		}
		
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE demo_task10(void *pv_para1,void *pv_para2)
{
	while(1)
	{
		osal_task_sleep(2000);

		if(0!=aui_os_mutex_lock(g_p_hdl_mutex_test, OSAL_WAIT_FOREVER_TIME))
		{
			AUI_PRINTF("\r\n lock failed.");
		}
		s_i_test--;
		s_i_test--;

		AUI_PRINTF("\r\n[%s]:[%d]",__FUNCTION__,s_i_test);
		if(0!=aui_os_mutex_unlock(g_p_hdl_mutex_test))
		{
			AUI_PRINTF("\r\n unlock failed.");
		}
		
	}
}

unsigned long test_os_task_create_demo1(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_task attr_task;
	aui_attr_mutex attr_mutex;
	MEMSET(&attr_task,0,sizeof(aui_attr_task));
	MEMSET(&attr_mutex,0,sizeof(aui_attr_mutex));

	if(0==strcmp(argv[0],"demo_task1"))
	{
		strcpy(attr_task.sz_name,"demo_task1");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=demo_task1;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task0));	
	}
	else if(0==strcmp(argv[0],"demo_task2"))
	{
		strcpy(attr_task.sz_name,"demo_task2");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=demo_task2;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	}	
	return AUI_RTN_SUCCESS;
}

unsigned long test_os_task_create_demo2(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_task attr_task;
	aui_attr_msgq attr_msgQ;
	unsigned long ul_msgQBuf_size=0;
	unsigned long ul_msg_len_max=0;
	MEMSET(&attr_task,0,sizeof(aui_attr_task));
	MEMSET(&attr_msgQ,0,sizeof(aui_attr_mutex));

	ul_msgQBuf_size = ATOI(argv[0]);
	ul_msg_len_max = ATOI(argv[1]); 

	if(NULL==g_p_hdl_msgQTest)
	{
		strcpy(attr_msgQ.sz_name,"msgqtest");
		attr_msgQ.ul_buf_size=ul_msgQBuf_size;
		attr_msgQ.ul_msg_size_max=ul_msg_len_max;
		if(0!=aui_os_msgq_create(&attr_msgQ,&g_p_hdl_msgQTest))
		{
			return -1;
		}
	}
	if(0==strcmp(argv[2],"send"))
	{
		strcpy(attr_task.sz_name,"send");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=demo_task3;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task0));	
	}
	else if(0==strcmp(argv[2],"receive"))
	{
		strcpy(attr_task.sz_name,"receive");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=demo_task4;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	}	

	return AUI_RTN_SUCCESS;
}

unsigned long test_os_task_create_demo3(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_task attr_task;
	aui_attr_mutex attr_mutex;
	MEMSET(&attr_task,0,sizeof(aui_attr_task));
	MEMSET(&attr_mutex,0,sizeof(aui_attr_mutex));

	if(NULL==g_p_hdl_mutex_test)
	{
		strcpy(attr_mutex.sz_name,"mutexttest");
		attr_mutex.mutex_type=argv[0][0]-'0';
		AUI_PRINTF("\r\nmutex type=[%d]",attr_mutex.mutex_type);
		if(0!=aui_os_mutex_create(&attr_mutex,&g_p_hdl_mutex_test))
		{
			return -1;
		}
	}
	if(0==g_mutex_id)
	{
		g_mutex_id=osal_mutex_create();
		if(OSAL_INVALID_ID==g_mutex_id)
		{
			return -1;
		}
	}
	if(0==strcmp(argv[2],"lock"))
	{
		strcpy(attr_task.sz_name,"lock");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		if(0==strcmp(argv[1],"0"))
		{
			attr_task.p_fun_task_entry=demo_task5;
		}
		else
		{
			attr_task.p_fun_task_entry=demo_task9;
		}
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task0));	
	}
	else if(0==strcmp(argv[2],"unlock"))
	{
		strcpy(attr_task.sz_name,"unlock");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		if(0==strcmp(argv[1],"0"))
		{
			attr_task.p_fun_task_entry=demo_task6;
		}
		else
		{
			attr_task.p_fun_task_entry=demo_task10;
		}
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	}	

	return AUI_RTN_SUCCESS;
}

unsigned long test_os_task_create_demo4(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_task attr_task;
	aui_attr_mutex attr_mutex;
	MEMSET(&attr_task,0,sizeof(aui_attr_task));
	MEMSET(&attr_mutex,0,sizeof(aui_attr_mutex));

	s_set_flag_bit = ATOI(argv[0]);
	s_wait_flag_bit = ATOI(argv[1]);
	s_wait_mode = ATOI(argv[2]);
	s_ul_clr_flag = ATOI(argv[3]);

	if(OSAL_INVALID_ID==s_flagIDDemo)
	{
		s_flagIDDemo=osal_flag_create(0);
		if(OSAL_INVALID_ID==s_flagIDDemo)
		{
			return -1;
		}
	}
	if(0==strcmp(argv[3],"set"))
	{
		strcpy(attr_task.sz_name,"set");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=demo_task7;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task0));	
	}
	else if(0==strcmp(argv[3],"wait"))
	{
		strcpy(attr_task.sz_name,"wait");
		attr_task.ul_priority=OSAL_PRI_NORMAL;
		attr_task.p_fun_task_entry=demo_task8;
		attr_task.ul_quantum=10;
		attr_task.ul_stack_size=0x400;
		AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_task1));	
	}	

	return AUI_RTN_SUCCESS;
}
unsigned long test_os_task_delete_demo(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task0,TRUE));
	AUI_TEST_CHECK_RTN(aui_os_task_delete(&g_p_hdl_task1,TRUE));


	if(NULL!=g_p_hdl_msgQTest)
	{
		AUI_TEST_CHECK_RTN(aui_os_msgq_delete(&g_p_hdl_msgQTest));
		g_p_hdl_msgQTest=NULL;
	}
	if(NULL!=g_p_hdl_mutex_test)
	{
		AUI_TEST_CHECK_RTN(aui_os_mutex_delete(&g_p_hdl_mutex_test));
		g_p_hdl_mutex_test=NULL;
	}
	if(OSAL_INVALID_ID!=s_flagIDDemo)
	{
		osal_flag_create(s_flagIDDemo);
		s_flagIDDemo=OSAL_INVALID_ID;
	}
	
	return AUI_RTN_SUCCESS;
}
unsigned long test_os_interrupt(unsigned long *argc,char **argv,char *sz_out_put)
{
	UINT flgptn=0; 
	TMO tmout=OSAL_WAIT_FOREVER_TIME;
	unsigned int wait_flag_bit=0x01;
	unsigned int wait_mode=OSAL_TWF_ORW;
	ID flagIDDemo=OSAL_INVALID_ID;
	unsigned long ul_init_val=0;
	
	osal_task_dispatch_off();
	AUI_PRINTF("\r\n 1can show this msg?");
	osal_task_dispatch_on();
	ul_init_val=argv[0][0]-'0';
	flagIDDemo=osal_flag_create(ul_init_val);
	if(OSAL_INVALID_ID==flagIDDemo)
	{
		AUI_PRINTF("\r\n 2can show this msg?");
		return -1;
	}
	s_flagIDDemo2=osal_flag_create(0);
	if(OSAL_INVALID_ID==s_flagIDDemo2)
	{
		AUI_PRINTF("\r\n 3can show this msg?");
		return -1;
	}

	if(0!=osal_flag_wait(&flgptn,flagIDDemo, wait_flag_bit,wait_mode,tmout))
	{
		AUI_PRINTF("\r\n flag wait failed.");
	}
	AUI_PRINTF("\r\n dir wait a flag");
	if(0!=osal_flag_clear(flagIDDemo, wait_flag_bit))
	{
		AUI_PRINTF("\r\n flag clr failed.");
	}

	
	AUI_PRINTF("\r\n ok");
	return AUI_RTN_SUCCESS;
}

unsigned long test_os_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nOS Test Help");  

	/* OS_1_HELP */
	#define 	OS_1_HELP_PART1		"The OS initialization is including but not limited to the below item:"
	#define 	OS_1_HELP_PART2		"--> init the globle event number" 
	#define 	OS_1_HELP_PART3		"--> init the limited size of mallocing memory" 
	#define 	OS_1_HELP_PART4		"--> init the semphore working mode" 
	#define 	OS_1_HELP_PART5		"--> init the area size of the heap memory" 
	#define 	OS_1_HELP_PART6 		"--> init the setting of the parameter that includes event flag id, mutex struct, event strcut and so on.\n" 

	aui_print_help_command("\'initos\'");
	aui_print_help_instruction_newline("Init OS test.\n");
	aui_print_help_instruction_newline(OS_1_HELP_PART1);
	aui_print_help_instruction_newline(OS_1_HELP_PART2);
	aui_print_help_instruction_newline(OS_1_HELP_PART3);
	aui_print_help_instruction_newline(OS_1_HELP_PART4);
	aui_print_help_instruction_newline(OS_1_HELP_PART5);
	aui_print_help_instruction_newline(OS_1_HELP_PART6);
	
	/* OS_2_HELP */
	#define 	OS_2_HELP_PART1 		"Only these two tasks can be created that one task names \"111111\" and the other task names \"222222\". The prioity level of the two task is NORMAL.\n"
	#define 	OS_2_HELP_PART2 		"Format:	   taskcreate [mutex type],[task index]" 
	#define 	OS_2_HELP_PART3 		"			   [mutex type]: 0: TDS mutex    1: Linux mutex    2: Linux mutex that can nest." 
	#define 	OS_2_HELP_PART4 		"			   [task index]: 1: create and run the task named \"111111\"    2: create and run the task named \"222222\".\n" 
	#define 	OS_2_HELP_PART5 		"Example:	   In the TDS system, the input is" 
	#define 	OS_2_HELP_PART6 		"			   taskcreate 0,1\n" 
	#define 	OS_2_HELP_PART7 		"			   taskcreate 0,2\n" 

	aui_print_help_command("\'taskcreate\'");
	aui_print_help_instruction_newline("Create task test\n");
	aui_print_help_instruction_newline(OS_2_HELP_PART1);
	aui_print_help_instruction_newline(OS_2_HELP_PART2);
	aui_print_help_instruction_newline(OS_2_HELP_PART3);
	aui_print_help_instruction_newline(OS_2_HELP_PART4);
	aui_print_help_instruction_newline(OS_2_HELP_PART5);
	aui_print_help_instruction_newline(OS_2_HELP_PART6);
	aui_print_help_instruction_newline(OS_2_HELP_PART7);

	/* OS_3_HELP */
	#define 	OS_3_HELP_PART1 		"To test \"taskdel\", please run \"taskcreate\" to creat two task first."
	#define 	OS_3_HELP_PART2 		"In TDS system:" 
	#define 	OS_3_HELP_PART3 		"			   taskcreate 0,1" 
	#define 	OS_3_HELP_PART4 		"			   taskcreate 0,2\n" 
	#define 	OS_3_HELP_PART5 		"\"taskdel\" will do the following steps:" 
	#define 	OS_3_HELP_PART6 		"--> Delete the task named \"111111\" \033[1munforced\033[0m. When not running, the task is able to be deleted." 
	#define 	OS_3_HELP_PART7 		"--> Delete the task named \"222222\" \033[1mforced\033[0m. Whenever if running, the task is able to be deleted.\n" 

	aui_print_help_command("\'taskdel\'");
	aui_print_help_instruction_newline("Delete task test\n");
	aui_print_help_instruction_newline(OS_3_HELP_PART1);
	aui_print_help_instruction_newline(OS_3_HELP_PART2);
	aui_print_help_instruction_newline(OS_3_HELP_PART3);
	aui_print_help_instruction_newline(OS_3_HELP_PART4);
	aui_print_help_instruction_newline(OS_3_HELP_PART5);
	aui_print_help_instruction_newline(OS_3_HELP_PART6);
	aui_print_help_instruction_newline(OS_3_HELP_PART7);

	/* OS_4_HELP */
	#define 	OS_4_HELP_PART1 		"To test \"taskjoin\", create task named \"111111\" first."
	#define 	OS_4_HELP_PART2 		"Using the \"taskcreate\" command to complete the operation." 
	#define 	OS_4_HELP_PART3 		"In TDS system:" 
	#define 	OS_4_HELP_PART4 		"			   taskcreate 0,1\n" 
	#define 	OS_4_HELP_PART5 		"The task joining can judge if the task completes the running.\n" 

	aui_print_help_command("\'taskjoin\'");
	aui_print_help_instruction_newline("Join task test\n");
	aui_print_help_instruction_newline(OS_4_HELP_PART1);
	aui_print_help_instruction_newline(OS_4_HELP_PART2);
	aui_print_help_instruction_newline(OS_4_HELP_PART3);
	aui_print_help_instruction_newline(OS_4_HELP_PART4);
	aui_print_help_instruction_newline(OS_4_HELP_PART5);

	/* OS_5_HELP */
	#define 	OS_5_HELP_PART1 		"Create a message queue named \"msgq1\" that the message buffer size is 256, the buffer number is 50"
	#define 	OS_5_HELP_PART2 		"Only the message queue is created, the \"msgclose\", \"msgsnd\" and \"msgrcv\" test can be going on.\n" 

	aui_print_help_command("\'msgopen\'");
	aui_print_help_instruction_newline("Create the message queue test\n");
	aui_print_help_instruction_newline(OS_5_HELP_PART1);
	aui_print_help_instruction_newline(OS_5_HELP_PART2);

	/* OS_6_HELP */
	#define 	OS_6_HELP_PART1 		"To test deleting message queue, message queue named \"msgq1\" should be create firstly. "
	#define 	OS_6_HELP_PART2 		"Using the \"msgopen\" command to complete the operation.\n" 

	aui_print_help_command("\'msgclose\'");
	aui_print_help_instruction_newline("Delete the message queue test\n");
	aui_print_help_instruction_newline(OS_6_HELP_PART1);
	aui_print_help_instruction_newline(OS_6_HELP_PART2);

	/* OS_7_HELP */
	#define 	OS_7_HELP_PART1 		"To test sending message queue, message queue named \"msgq1\" should be create firstly. "
	#define 	OS_7_HELP_PART2 		"Using the \"msgopen\" command to complete the operation.\n" 
	#define 	OS_7_HELP_PART3 		"Format:	   msgsnd [string]" 
	#define 	OS_7_HELP_PART4 		"			   [string]: The string that to be sent out. The string size is not large than 256.\n" 
	#define 	OS_7_HELP_PART5 		"Example:	   If the \"hello_world\" string is sent out, the input is" 
	#define 	OS_7_HELP_PART6 		"			   msgsnd hello_world\n" 

	aui_print_help_command("\'msgsnd\'");
	aui_print_help_instruction_newline("Send message queue data test\n");
	aui_print_help_instruction_newline(OS_7_HELP_PART1);
	aui_print_help_instruction_newline(OS_7_HELP_PART2);
	aui_print_help_instruction_newline(OS_7_HELP_PART3);
	aui_print_help_instruction_newline(OS_7_HELP_PART4);
	aui_print_help_instruction_newline(OS_7_HELP_PART5);
	aui_print_help_instruction_newline(OS_7_HELP_PART6);

	/* OS_8_HELP */
	#define 	OS_8_HELP_PART1 		"To test receiving message queue, message queue named \"msgq1\" should be create firstly using the \"msgopen\" command. "
	#define 	OS_8_HELP_PART2 		"And then the message, with \"hello_world\" string for example, is sent out from the message queue using the \"msgsnd hello_world\" command" 
	#define 	OS_8_HELP_PART3 		"At last using the \"msgrcv\" command, it's able to get the \"hello_world\" string.\n" 
	
	aui_print_help_command("\'msgrcv\'");
	aui_print_help_instruction_newline("Receive message queue data test\n");
	aui_print_help_instruction_newline(OS_8_HELP_PART1);
	aui_print_help_instruction_newline(OS_8_HELP_PART2);
	aui_print_help_instruction_newline(OS_8_HELP_PART3);

	/* OS_9_HELP */
	#define 	OS_9_HELP_PART1 		"Format:	   semcreate [init value],[max number]" 
	#define 	OS_9_HELP_PART2 		"			   [init value]: semaphore initial value. TDS does not support the initial value currently. It is reserve now." 
	#define 	OS_9_HELP_PART3 		"			   [max number]: semaphore maximum number\n" 
	#define 	OS_9_HELP_PART4 		"Example:	   If the semphore whose maximum number is 9 is created, the input is" 
	#define 	OS_9_HELP_PART5 		"			   semcreate 0,9\n" 

	aui_print_help_command("\'semcreate\'");
	aui_print_help_instruction_newline("Create semphore test\n");
	aui_print_help_instruction_newline(OS_9_HELP_PART1);
	aui_print_help_instruction_newline(OS_9_HELP_PART2);
	aui_print_help_instruction_newline(OS_9_HELP_PART3);
	aui_print_help_instruction_newline(OS_9_HELP_PART4);
	aui_print_help_instruction_newline(OS_9_HELP_PART5);

	/* OS_10_HELP */
	#define 	OS_10_HELP_PART1 		"To test deleting semphore, the semphore should be create firstly. "
	#define 	OS_10_HELP_PART2 		"Using the \"semcreate\" command to complete the operation.\n" 

	aui_print_help_command("\'semdel\'");
	aui_print_help_instruction_newline("Delete the semphore test\n");
	aui_print_help_instruction_newline(OS_10_HELP_PART1);
	aui_print_help_instruction_newline(OS_10_HELP_PART2);

	/* OS_11_HELP */
	#define 	OS_11_HELP_PART1 		"To test getting the semphore, the semphore should be create firstly. "
	#define 	OS_11_HELP_PART2 		"Using the \"semcreate\" command to complete the operation.\n" 
	#define 	OS_11_HELP_PART3		"If the \"semtake\" command is excuted, the available semphore will minus 1" 
	#define 	OS_11_HELP_PART4		"When the current semphore value is equal to 0, it is not able to minus 1.\n" 
	#define 	OS_11_HELP_PART5 		"Format:	   semtake [timeout]" 
	#define 	OS_11_HELP_PART6 		"			   [timeout]: the waiting time of getting semphore. The system wait the [timeout]ms until gets the semphore.\n" 
	#define 	OS_11_HELP_PART7 		"Example:	   If the timeout of getting semphore is 1000ms, the input is" 
	#define 	OS_11_HELP_PART8 		"			   semtake 1000\n" 

	aui_print_help_command("\'semtake\'");
	aui_print_help_instruction_newline("Get the semphore test\n");
	aui_print_help_instruction_newline(OS_11_HELP_PART1);
	aui_print_help_instruction_newline(OS_11_HELP_PART2);
	aui_print_help_instruction_newline(OS_11_HELP_PART3);
	aui_print_help_instruction_newline(OS_11_HELP_PART4);
	aui_print_help_instruction_newline(OS_11_HELP_PART5);
	aui_print_help_instruction_newline(OS_11_HELP_PART6);
	aui_print_help_instruction_newline(OS_11_HELP_PART7);
	aui_print_help_instruction_newline(OS_11_HELP_PART8);

	/* OS_12_HELP */
	#define 	OS_12_HELP_PART1		"To test releasing the semphore, the semphore should be create firstly. "
	#define 	OS_12_HELP_PART2		"Using the \"semcreate\" command to complete the operation.\n" 
	#define 	OS_12_HELP_PART3		"If the \"semgive\" command is excuted, the available semphore will add 1." 
	#define 	OS_12_HELP_PART4		"When the current semphore value is equal to the max semphore value, it is not able to add 1.\n" 

	aui_print_help_command("\'semgive\'");
	aui_print_help_instruction_newline("Release the semphore test\n");
	aui_print_help_instruction_newline(OS_12_HELP_PART1);
	aui_print_help_instruction_newline(OS_12_HELP_PART2);
	aui_print_help_instruction_newline(OS_12_HELP_PART3);
	aui_print_help_instruction_newline(OS_12_HELP_PART4);

	/* OS_13_HELP */
	#define 	OS_13_HELP_PART1 		"Format:	   eventcreate [b_auto_reset],[b_initial_state]" 
	#define 	OS_13_HELP_PART2 		"			   [b_auto_reset]: If set auto reset event when created. 0: FALSE    1: TRUE" 
	#define 	OS_13_HELP_PART3 		"			   [b_initial_state]: If init state of event when created. 0: FALSE    1: TRUE\n" 
	#define 	OS_13_HELP_PART4 		"Example:	   If the event configurated auto-reset and init-state is created, the input is" 
	#define 	OS_13_HELP_PART5 		"			   eventcreate 1,1\n" 

	aui_print_help_command("\'eventcreate\'");
	aui_print_help_instruction_newline("Create event test\n");
	aui_print_help_instruction_newline(OS_13_HELP_PART1);
	aui_print_help_instruction_newline(OS_13_HELP_PART2);
	aui_print_help_instruction_newline(OS_13_HELP_PART3);
	aui_print_help_instruction_newline(OS_13_HELP_PART4);
	aui_print_help_instruction_newline(OS_13_HELP_PART5);

	/* OS_14_HELP */
	#define 	OS_14_HELP_PART1		"To test deleting event, the event should be create firstly. "
	#define 	OS_14_HELP_PART2		"Using the \"eventcreate\" command to complete the operation.\n" 

	aui_print_help_command("\'eventdel\'");
	aui_print_help_instruction_newline("Delete the event test\n");
	aui_print_help_instruction_newline(OS_14_HELP_PART1);
	aui_print_help_instruction_newline(OS_14_HELP_PART2);

	/* OS_15_HELP */
	#define 	OS_15_HELP_PART1		"To test setting event, the event should be create firstly. "
	#define 	OS_15_HELP_PART2		"Using the \"eventcreate\" command to complete the operation.\n" 

	aui_print_help_command("\'eventset\'");
	aui_print_help_instruction_newline("Set the event test\n");
	aui_print_help_instruction_newline(OS_15_HELP_PART1);
	aui_print_help_instruction_newline(OS_15_HELP_PART2);

	/* OS_16_HELP */
	#define 	OS_16_HELP_PART1		"To test getting event, the event should be create firstly. "
	#define 	OS_16_HELP_PART2		"Using the \"eventcreate\" command to complete the operation.\n" 
	#define 	OS_16_HELP_PART3		"When the event is set, excute the \"eventwait\" command can get the event signal.\n" 
	#define 	OS_16_HELP_PART4		"Format:	   eventwait [timeout]" 
	#define 	OS_16_HELP_PART5		"			   [timeout]: the waiting time of getting event. The system wait the [timeout]ms until gets the event.\n" 
	#define 	OS_16_HELP_PART6		"Example:	   If the timeout of getting event is 1000ms, the input is" 
	#define 	OS_16_HELP_PART7		"			   eventwait 1000\n" 

	aui_print_help_command("\'eventwait\'");
	aui_print_help_instruction_newline("Get the event test\n");
	aui_print_help_instruction_newline(OS_16_HELP_PART1);
	aui_print_help_instruction_newline(OS_16_HELP_PART2);
	aui_print_help_instruction_newline(OS_16_HELP_PART3);
	aui_print_help_instruction_newline(OS_16_HELP_PART4);
	aui_print_help_instruction_newline(OS_16_HELP_PART5);
	aui_print_help_instruction_newline(OS_16_HELP_PART6);
	aui_print_help_instruction_newline(OS_16_HELP_PART7);

	/* OS_17_HELP */
	#define 	OS_17_HELP_PART1		"Format:	   mutexcreate [mutex type]" 
	#define 	OS_17_HELP_PART2		"			   [mutex type]: 0: TDS mutex    1: Linux mutex    2: Linux mutex that can nest.\n" 
	#define 	OS_17_HELP_PART3		"Example:	   If the mutex is created in the TDS system, the input is" 
	#define 	OS_17_HELP_PART4		"			   mutexcreate 0\n" 

	aui_print_help_command("\'mutexcreate\'");
	aui_print_help_instruction_newline("Create mutex test\n");
	aui_print_help_instruction_newline(OS_17_HELP_PART1);
	aui_print_help_instruction_newline(OS_17_HELP_PART2);
	aui_print_help_instruction_newline(OS_17_HELP_PART3);
	aui_print_help_instruction_newline(OS_17_HELP_PART4);

	/* OS_18_HELP */
	#define 	OS_18_HELP_PART1		"To test deleting mutex, the mutex should be create firstly. "
	#define 	OS_18_HELP_PART2		"Using the \"mutexcreate\" command to complete the operation.\n" 

	aui_print_help_command("\'mutexdel\'");
	aui_print_help_instruction_newline("Delete the mutex test\n");
	aui_print_help_instruction_newline(OS_18_HELP_PART1);
	aui_print_help_instruction_newline(OS_18_HELP_PART2);

	/* OS_19_HELP */
	#define 	OS_19_HELP_PART1		"To test locking mutex, the mutex should be create firstly. "
	#define 	OS_19_HELP_PART2		"Using the \"mutexcreate\" command to complete the operation.\n" 
	#define 	OS_19_HELP_PART3		"When the mutex is unlocked, excute the \"mutexlock\" command can get mutex.\n" 
	#define 	OS_19_HELP_PART4		"Format:	   mutexlock [timeout]" 
	#define 	OS_19_HELP_PART5		"			   [timeout]: the waiting time of getting unlocked mutex. The system wait the [timeout]ms until gets the unlocked mutex.\n" 
	#define 	OS_19_HELP_PART6		"Example:	   If the timeout of getting unlocked mutex is 1000ms, the input is" 
	#define 	OS_19_HELP_PART7		"			   mutexlock 1000\n" 

	aui_print_help_command("\'mutexlock\'");
	aui_print_help_instruction_newline("Lock the mutex test\n");
	aui_print_help_instruction_newline(OS_19_HELP_PART1);
	aui_print_help_instruction_newline(OS_19_HELP_PART2);
	aui_print_help_instruction_newline(OS_19_HELP_PART3);
	aui_print_help_instruction_newline(OS_19_HELP_PART4);
	aui_print_help_instruction_newline(OS_19_HELP_PART5);
	aui_print_help_instruction_newline(OS_19_HELP_PART6);
	aui_print_help_instruction_newline(OS_19_HELP_PART7);

	/* OS_20_HELP */
	#define 	OS_20_HELP_PART1		"To test unlocking the mutex, the mutex should be create firstly. "
	#define 	OS_20_HELP_PART2		"Using the \"mutexunlock\" command to complete the operation.\n" 

	aui_print_help_command("\'mutexunlock\'");
	aui_print_help_instruction_newline("Unlock the mutex test\n");
	aui_print_help_instruction_newline(OS_20_HELP_PART1);
	aui_print_help_instruction_newline(OS_20_HELP_PART2);

	/* OS_21_HELP */
	#define 	OS_21_HELP_PART1		"To test trying lock mutex, the mutex should be create firstly. "
	#define 	OS_21_HELP_PART2		"Using the \"mutexcreate\" command to complete the operation.\n" 
	#define 	OS_21_HELP_PART3		"When the \"mutextrylock\" command is excuted, it will try to lock the mutex every 1000ms and repeat the operation 7 times.\n" 
	#define 	OS_21_HELP_PART4		"Format:	   mutextrylock [mode]" 
	#define 	OS_21_HELP_PART5		"			   [mode]:  0: Using the \'aui_os_mutex_lock\' interface    1: Using the \'os_mutex_lock\' interface.\n" 
	#define 	OS_21_HELP_PART6		"Example:	   If the mutex is tried to lock using \'aui_os_mutex_lock\' interface, the input is" 
	#define 	OS_21_HELP_PART7		"			   mutextrylock 0\n" 

	aui_print_help_command("\'mutextrylock\'");
	aui_print_help_instruction_newline("Try lock the mutex test\n");
	aui_print_help_instruction_newline(OS_21_HELP_PART1);
	aui_print_help_instruction_newline(OS_21_HELP_PART2);
	aui_print_help_instruction_newline(OS_21_HELP_PART3);
	aui_print_help_instruction_newline(OS_21_HELP_PART4);
	aui_print_help_instruction_newline(OS_21_HELP_PART5);
	aui_print_help_instruction_newline(OS_21_HELP_PART6);
	aui_print_help_instruction_newline(OS_21_HELP_PART7);

	/* OS_22_HELP */
	#define 	OS_22_HELP_PART1 		"The timer has two mode: alarm and cyclic."
	#define 	OS_22_HELP_PART2		"-->Alarm mode: Only trigger the callback function one time when the timer value reaches the timeout time."
	#define 	OS_22_HELP_PART3		"-->Cyclic mode: Trigger the callback function every time when the timer value reaches the timeout time until the timer is stopped by extern (e.g., by system, by people or by other)."
	#define 	OS_22_HELP_PART4		"                Cyclic timer need run \"timerrun 1\"command to enable.\n"
	#define 	OS_22_HELP_PART5 		"Format:	   timercreate [task name],[mode],[timeout],[parameter]" 
	#define 	OS_22_HELP_PART6 		"			   [task name]: Name for the timer task." 
	#define 	OS_22_HELP_PART7 		"			   [mode]: Timer mode. \"TIMER_ALARM\" or \"TIMER_CYCLIC\"" 
	#define 	OS_22_HELP_PART8		"			   [timout]: The timer's timeout time in msec." 
	#define 	OS_22_HELP_PART9		"			   [parameter]: The inputting parameter of the timer's timeout callback function\n" 
	#define 	OS_22_HELP_PART10 		"Example:	   If the alarm/cyclic timer task named \"timer1\" wants to be created that the timeout time is 2s, inputting parameter is \"123\", the input is" 
	#define 	OS_22_HELP_PART11 		"			   timercreate timer1,TIMER_ALARM,2000,123" 
	#define 	OS_22_HELP_PART12		"			   timercreate timer1,TIMER_CYCLIC,2000,345\n" 

	aui_print_help_command("\'timercreate\'");
	aui_print_help_instruction_newline("Create timer test\n");
	aui_print_help_instruction_newline(OS_22_HELP_PART1);
	aui_print_help_instruction_newline(OS_22_HELP_PART2);
	aui_print_help_instruction_newline(OS_22_HELP_PART3);
	aui_print_help_instruction_newline(OS_22_HELP_PART4);
	aui_print_help_instruction_newline(OS_22_HELP_PART5);
	aui_print_help_instruction_newline(OS_22_HELP_PART6);
	aui_print_help_instruction_newline(OS_22_HELP_PART7);
	aui_print_help_instruction_newline(OS_22_HELP_PART8);
	aui_print_help_instruction_newline(OS_22_HELP_PART9);
	aui_print_help_instruction_newline(OS_22_HELP_PART10);
	aui_print_help_instruction_newline(OS_22_HELP_PART11);
	aui_print_help_instruction_newline(OS_22_HELP_PART12);

	/* OS_23_HELP */
	#define 	OS_23_HELP_PART1		"To test timer mutex, the timer should be create firstly. "
	#define 	OS_23_HELP_PART2		"Using the \"timercreate\" command to complete the operation.\n" 

	aui_print_help_command("\'timerdel\'");
	aui_print_help_instruction_newline("Delete the timer test\n");
	aui_print_help_instruction_newline(OS_23_HELP_PART1);
	aui_print_help_instruction_newline(OS_23_HELP_PART2);

	/* OS_24_HELP */
	#define 	OS_24_HELP_PART1		"Format:	   timerset [mode],[value]" 
	#define 	OS_24_HELP_PART2		"			   [mode]: \"DELAY\": deley the [value]ms time    \"VAL\": set the timer [value]ms timeout." 
	#define 	OS_24_HELP_PART3		"			   [value]: The value of deley time or timeout time\n" 
	#define 	OS_24_HELP_PART4		"Example:	   If set 1000ms delay, the input is" 
	#define 	OS_24_HELP_PART5		"			   timerset DELAY,1000\n" 

	aui_print_help_command("\'timerset\'");
	aui_print_help_instruction_newline("Set timer parameter test\n");
	aui_print_help_instruction_newline(OS_24_HELP_PART1);
	aui_print_help_instruction_newline(OS_24_HELP_PART2);
	aui_print_help_instruction_newline(OS_24_HELP_PART3);
	aui_print_help_instruction_newline(OS_24_HELP_PART4);
	aui_print_help_instruction_newline(OS_24_HELP_PART5);

	/* OS_25_HELP */
	#define 	OS_25_HELP_PART1		"Format:	   timerget [time mode]" 
	#define 	OS_25_HELP_PART2		"			   [time mode]: \"SECOND\": get timer running time in second	  \"TICK\": get timer running time in tick." 
	#define 	OS_25_HELP_PART3		"Example:	   If get timer running time in second, the input is" 
	#define 	OS_25_HELP_PART4		"			   timerget SECOND\n" 

	aui_print_help_command("\'timerget\'");
	aui_print_help_instruction_newline("Get timer running time test\n");
	aui_print_help_instruction_newline(OS_25_HELP_PART1);
	aui_print_help_instruction_newline(OS_25_HELP_PART2);
	aui_print_help_instruction_newline(OS_25_HELP_PART3);
	aui_print_help_instruction_newline(OS_25_HELP_PART4);

	/* OS_26_HELP */
	#define 	OS_26_HELP_PART1		"Enable/Disable the cyclic timer."
	#define 	OS_26_HELP_PART2		"You need create cyclic timer first:"
	#define 	OS_26_HELP_PART3		"			   timercreate timer1,TIMER_CYCLIC,2000,345\n" 
	#define 	OS_26_HELP_PART4		"Format:	   timerrun [is_enable]" 
	#define 	OS_26_HELP_PART5		"			   [is_enable]: 0: Stop the running of the cyclic timer    1: Start the running of the cyclic timer.\n" 
	#define 	OS_26_HELP_PART6		"Example:	   If enable/disable the cyclic timer running, the input is" 
	#define 	OS_26_HELP_PART7		"			   timerrun 1" 
	#define 	OS_26_HELP_PART8		"			   timerrun 0\n" 

	aui_print_help_command("\'timerrun\'");
	aui_print_help_instruction_newline(OS_26_HELP_PART1);
	aui_print_help_instruction_newline(OS_26_HELP_PART2);
	aui_print_help_instruction_newline(OS_26_HELP_PART3);
	aui_print_help_instruction_newline(OS_26_HELP_PART4);
	aui_print_help_instruction_newline(OS_26_HELP_PART5);
	aui_print_help_instruction_newline(OS_26_HELP_PART6);
	aui_print_help_instruction_newline(OS_26_HELP_PART7);
	aui_print_help_instruction_newline(OS_26_HELP_PART8);

	return AUI_RTN_HELP;
}

void aui_load_tu_os()
{
	aui_tu_reg_group("os", "OS Test");
	{
        aui_tu_reg_item(2,"h",AUI_CMD_TYPE_UNIT,test_os_help,"OS test help");	
        aui_tu_reg_item(2,"initos",AUI_CMD_TYPE_UNIT,test_os_init,"Init os test");   
        aui_tu_reg_item(2,"taskcreate",AUI_CMD_TYPE_UNIT,test_os_task_create,"Create task test");   
        aui_tu_reg_item(2,"taskdel",AUI_CMD_TYPE_UNIT,test_os_task_delete,"Delete task test");
        //aui_tu_reg_item(2,"taskselfid",AUI_CMD_TYPE_UNIT,test_os_task_self_id,"Read task self id test");	
        aui_tu_reg_item(2,"taskjoin",AUI_CMD_TYPE_UNIT,test_os_task_join,"task join test");	
        aui_tu_reg_item(2,"msgopen",AUI_CMD_TYPE_UNIT,test_os_msgQCreate,"Create msgq test");	
        aui_tu_reg_item(2,"msgclose",AUI_CMD_TYPE_UNIT,test_os_msgQDelete,"Delete msgq test");				
        aui_tu_reg_item(2,"msgsnd",AUI_CMD_TYPE_UNIT,test_os_msgQSnd,"Send msgq test");	
        aui_tu_reg_item(2,"msgrcv",AUI_CMD_TYPE_UNIT,test_os_msgQRcv,"Receive msgq test");	
        aui_tu_reg_item(2,"semcreate",AUI_CMD_TYPE_UNIT,test_os_sem_create,"Create semp test");	
        aui_tu_reg_item(2,"semdel",AUI_CMD_TYPE_UNIT,test_os_sem_delete,"Delete semp test");				
        aui_tu_reg_item(2,"semtake",AUI_CMD_TYPE_UNIT,test_os_sem_wait,"Wait semp test");	
        aui_tu_reg_item(2,"semgive",AUI_CMD_TYPE_UNIT,test_os_sem_release,"Release semp test");	
        aui_tu_reg_item(2,"eventcreate",AUI_CMD_TYPE_UNIT,test_os_event_create,"Create event test");	
        aui_tu_reg_item(2,"eventdel",AUI_CMD_TYPE_UNIT,test_os_event_delete,"Delete event test");				
        aui_tu_reg_item(2,"eventset",AUI_CMD_TYPE_UNIT,test_os_event_set,"Set event test");	
        aui_tu_reg_item(2,"eventwait",AUI_CMD_TYPE_UNIT,test_os_event_wait,"Wait event test");	
        aui_tu_reg_item(2,"mutexcreate",AUI_CMD_TYPE_UNIT,test_os_mutex_create,"Create mutex test");	
        aui_tu_reg_item(2,"mutexdel",AUI_CMD_TYPE_UNIT,test_os_mutex_delete,"Delete mutex test");				
        aui_tu_reg_item(2,"mutexlock",AUI_CMD_TYPE_UNIT,test_os_mutex_lock,"Lock mutex test");	
        aui_tu_reg_item(2,"mutexunlock",AUI_CMD_TYPE_UNIT,test_os_mutex_unlock,"Unlock mutex test");
        aui_tu_reg_item(2,"mutextrylock",AUI_CMD_TYPE_UNIT,test_os_mutex_try_lock,"Try Lock mutex test");	
        aui_tu_reg_item(2,"timercreate",AUI_CMD_TYPE_UNIT,test_os_timer_create,"Create time test");	
        aui_tu_reg_item(2,"timerdel",AUI_CMD_TYPE_UNIT,test_os_timer_delete,"Delete time test");				
        aui_tu_reg_item(2,"timerset",AUI_CMD_TYPE_UNIT,test_os_timer_set,"Set time test");	
        aui_tu_reg_item(2,"timerget",AUI_CMD_TYPE_UNIT,test_os_timer_get,"Get time test");
        aui_tu_reg_item(2,"timerrun",AUI_CMD_TYPE_UNIT,test_os_timer_run,"Start run time");	
	}
}

