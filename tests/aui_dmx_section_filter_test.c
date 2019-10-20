#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

#include <aui_nim.h>
#include <aui_tsi.h>
#include <aui_deca.h>
#include <aui_decv.h>
#include <aui_snd.h>
#include <aui_dmx.h>
#include <sys/times.h>

#include "aui_nim_init.h"
#include "aui_test_stream.h"

static void print_help(char *sz_appname)
{
    printf("Uasage %s [n:t:T:f:s:b:d:c:l:m:h]\n", sz_appname);
    printf("\nCommand line options\n\n");
	printf("\t-m --<mode> 0: need connect frontend; 1: don`t connect frontend\n");
    printf("\t-n --nim <nim_id>\n");    
    printf("\t-t --tnim <nim_type>\n");
    printf("\t-T --dvbt <DVBT type>\n");
    printf("\t-f --freq <frequency>\n");
    printf("\t-s --sym <symbole_rate>\n");
    printf("\t-b --band <dvbt_band>\n");
    printf("\t-d --dmx <dmx_id>\n");
    printf("\t-c --spid <section_pid>\n");
    printf("\t-l -- <life>\n");
    printf("\t-h --help\n");
    printf("\te.g. for dvbs: %s -n 0 -t 0 -f 3840 -s 27500 -d 0  -c 0 -l 10 -m 1\n", sz_appname);
}

int cnt = 0;
static long fun_sectionCB
(
	aui_hdl filter_handle,
	unsigned char *p_section_data_buf_addr,
	unsigned long ul_section_data_len,
	void *usr_data,
	void *reserved
)
{
    cnt++;
    if(0 == (cnt%40))
    {
    #if 1
    AUI_PRINTF("\r\n@process %d -> filter[%p] Got data length[%lu], first 5 bytes:\n[%02x][%02x][%02x][%02x][%02x]\n",
            getpid(),filter_handle,ul_section_data_len,
            p_section_data_buf_addr[0],p_section_data_buf_addr[1],p_section_data_buf_addr[2],
            p_section_data_buf_addr[3],p_section_data_buf_addr[4]);
    #endif
    }
    return 0;
}

static    aui_attr_dmx_channel attr_channel;
static    aui_attr_dmx_filter attr_filter;
static    aui_hdl hdl_channel=NULL;
static    aui_hdl hdl_filter=NULL;
static    aui_hdl dmx_hdl;
static    aui_attr_dmx attr_dmx;

static int register_section(int dmx_id, unsigned short pid, aui_p_fun_sectionCB fun_sectionCB)
{
    memset(&attr_dmx, 0, sizeof(attr_dmx));
    attr_dmx.uc_dev_idx = dmx_id;
    if(aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_id, &dmx_hdl))
	{
		if (aui_dmx_open(&attr_dmx, &dmx_hdl))
        {
			AUI_PRINTF("\r\n dmx open fail\n");
			return -1;
		}
	}
    aui_dmx_start(dmx_hdl, NULL);
    /* If you want to get all the tables that the first byte equals to 0x80,
     * you should set mask[] = {0xff}, value[] = {0x80}, 
     * reverse[] = {0x00}, ul_mask_val_len = 1 */

    /* Get all tables */ 
    unsigned char mask[] = {0x00};
    unsigned char value[] = {0x80};
    unsigned char reverse[] = {0x00};
    /* Just care for the first BYTE */
    unsigned long ul_mask_val_len=1;
    /* Dont do CRC check for the section data */
	unsigned char uc_crc_check=0;
    // Muti-times capture the section,if uc_continue_capture_flag==0,then just the first section will call the user callback
	unsigned char uc_continue_capture_flag=1;
    
    memset(&attr_channel,0,sizeof(aui_attr_dmx_channel));
    memset(&attr_filter,0,sizeof(aui_attr_dmx_filter));
    
    //Capture the channel which bind with PID (e.g. PID is 0)
    attr_channel.us_pid = pid;
    //Config the channel data type are section data
    attr_channel.dmx_data_type = AUI_DMX_DATA_SECT;
    //Open the dmx channel device
    aui_dmx_channel_open(dmx_hdl, &attr_channel, &hdl_channel);
    //Start the dmx channel device    
    aui_dmx_channel_start(hdl_channel, &attr_channel);
   
    attr_filter.puc_mask = mask;
    attr_filter.puc_reverse = reverse;
    attr_filter.puc_val = value;
    attr_filter.ul_mask_val_len = ul_mask_val_len;
    
    //Open the dmx filter device
    aui_dmx_filter_open(hdl_channel,&attr_filter,&hdl_filter);
    printf("\r\nfilter handler is [%p]", hdl_filter);
    //The fun_sectionCB are the callback for app and implement by user
    aui_dmx_reg_sect_call_back(hdl_filter,fun_sectionCB);
    
    //Configure the filter:user want to continue get all the table that the first byte equal 0x80
    aui_dmx_filter_mask_val_cfg(hdl_filter,mask,value,reverse, ul_mask_val_len,uc_crc_check,uc_continue_capture_flag);

    //Start the dmx filter device
    aui_dmx_filter_start(hdl_filter, &attr_filter);

    return 0;
}

int unregister_section()
{
    aui_dmx_filter_stop(hdl_filter, NULL);
    aui_dmx_filter_close(&hdl_filter);
    aui_dmx_channel_stop(hdl_channel, NULL);
    aui_dmx_channel_close(&hdl_channel);
    aui_dmx_stop(dmx_hdl, NULL);
    aui_dmx_close(dmx_hdl);
    return 0;
}
int main(int argc, char **argv)
{
	int opt;
	int option_index;
	unsigned long nim_id = 1;
	unsigned long nim_type = 0;
	unsigned long dvbt_type = 0;
	unsigned long freq = 3840;
	unsigned long symb = 27500;
	//unsigned long polar = 0;
        unsigned long  band = 0;
        unsigned long  life = 0;
        unsigned long  mode = 0;

	aui_dmx_id_t dmx_id = AUI_DMX_ID_DEMUX0;
	unsigned short section_pid = 0;
	
	aui_hdl nim_hdl = NULL;
	aui_hdl tsi_hdl = NULL;

	static struct option long_options[] = {
		{ "help",		 no_argument,	0, 'h' },
		{ "nim",         required_argument, 0, 'n' },
		{ "tnim",        required_argument, 0, 't' },
		{ "DVBT",        required_argument, 0, 'T' },
		{ "freq",		 required_argument, 0, 'f' },
		{ "sym",         required_argument, 0, 's' },
		{ "band",        required_argument, 0, 'b' },
		{ "dmx id",      required_argument, 0, 'd' },
        { "spid",        required_argument, 0, 'c' },
        { "life",        required_argument, 0, 'l' },
        { "mode",        required_argument, 0, 'm' },
		{ 0, 0, 0, 0}
	};

	while ((opt = getopt_long (argc, argv, "n:t:T:f:s:b:d:c:l:m:h", long_options, &option_index)) != -1) {

		switch (opt) {
		case 'n':
			nim_id = atoi(optarg);
			break;
		case 't':
			nim_type = atoi(optarg);
			break;	
		case 'T':
			dvbt_type = atoi(optarg);
			break;
		case 'f':
			freq = atoi(optarg);
			break;
		case 's':
			symb = atoi(optarg);
			break;
		case 'b':
			band = atoi(optarg);
			break;	
		case 'd':
			dmx_id = (aui_dmx_id_t)atoi(optarg);
			break;
		case 'c':
			section_pid = atoi(optarg);
			break;
		case 'l':
			life = atoi(optarg);
			break;
        case 'm':
			mode = atoi(optarg);
			break;
		case 'h':
		    print_help(argv[0]);
			return 0;
        default:
            print_help(argv[0]);
            return -1;
		}
	}

	printf("mode: %lu \n", mode);
	if(0==mode) {
	    struct ali_app_nim_init_para init_para_nim;
	    memset(&init_para_nim, 0, sizeof(struct ali_app_nim_init_para));
	    init_para_nim.ul_nim_type = nim_type;
	    init_para_nim.ul_freq = freq;
	    init_para_nim.ul_symb_rate = symb;
	    init_para_nim.ul_freq_band = band;
	    init_para_nim.ul_device = nim_id;
	    init_para_nim.ul_nim_std = dvbt_type;  //added by vedic   value for DVBT   
	    printf("nim_id: %lu, nim_type: %lu, freq: %lu, symb: %lu, band: %lu, dvbt_type: %lu\n", 
	        nim_id, nim_type, freq, symb, band, dvbt_type);
	    if (aui_nim_init(nim_init_cb)) {
	        AUI_PRINTF("\nnim init error\n");
	        return -1;
	    }
		if (nim_connect(&init_para_nim, &nim_hdl)) {
			printf("nim connect error\n");
			goto error_live;
		}

	    struct ali_app_tsi_init_para init_para_tsi;
	    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];

	    if (dmx_id == AUI_DMX_ID_DEMUX0 ||  dmx_id == AUI_DMX_ID_DEMUX1)       
	        init_para_tsi.ul_dmx_idx = (enum aui_tsi_output_id)dmx_id;
	    else if (dmx_id == AUI_DMX_ID_DEMUX2 || dmx_id == AUI_DMX_ID_DEMUX3)
	        init_para_tsi.ul_dmx_idx = (enum aui_tsi_output_id)(dmx_id - 1);
	    init_para_tsi.ul_tsi_id = 0;
	    init_para_tsi.ul_tis_port_idx = AUI_TSI_CHANNEL_0;
	    aui_tsi_config(tsi_cfg);
	    init_para_tsi.ul_hw_init_val = tsi_cfg[nim_id].ul_hw_init_val;
	    init_para_tsi.ul_input_src = tsi_cfg[nim_id].ul_input_src;
	    if (ali_app_tsi_init(&init_para_tsi, &tsi_hdl)) {
	        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
	        goto error_live;
	    }
		printf("tsi channel 0 initialized with DMX 0\n");
	}
    register_section(dmx_id, section_pid, fun_sectionCB);
    if(life == 0) {
    	while (1);
    }    
    AUI_PRINTF("Will stop in %lu seconds!!\n", life);
	sleep(life);
error_live:
	if(0==mode) {
	    ali_app_nim_deinit(nim_hdl);
	    ali_app_tsi_deinit(tsi_hdl);
	    //ali_app_dmx_deinit(dmx_hdl);
	}
    unregister_section();
	return 0;
}
