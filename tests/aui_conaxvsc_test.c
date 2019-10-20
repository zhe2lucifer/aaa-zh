/*
 * Conax Virtual Smart Card test module
 * Copyright(C) 2014 ALi Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>

#include <aui_conaxvsc.h>


#define FD_INVALID     -1
#define MAX_PID_CNT    32

#define VSC_PI_HOST_VER             0x10
#define VSC_PI_CAT                  0x11
#define VSC_PI_EMM                  0x12
#define VSC_PI_ECM                  0x14
#define VSC_PI_RET_CHAN_CONFIG      0x1B
#define VSC_PI_CA_STATUS_SELECT     0x1C
#define VSC_PI_PIN_IN               0x1D
#define VSC_PI_REQ_BLOCK_SIZE       0x65
#define VSC_PI_REQ_CARD_NUMBER      0x66
#define VSC_PI_REQ_SEQUENCE_NUMBER  0x67
#define VSC_PI_RESET_SESSION        0x69
#define VSC_PI_CASS_VER             0x20
#define VSC_PI_CA_DESC_EMM          0x22
#define VSC_PI_SESSION_INFO         0x23
#define VSC_PI_CW                   0x25
#define VSC_PI_CA_SYS_ID            0x28
#define VSC_PI_CRYPTO_BLOCK_SIZE    0x73
#define VSC_PI_CARD_NUMBER          0x74
#define VSC_PI_SEQUENCE_NUMBER      0x75
#define VSC_PI_HOST_DATA            0x80

#define VSC_PI_REQ_VSC_VERSION      0xC1
#define VSC_PI_VALIDATION_NUMBER    0xD0
#define VSC_PI_VSC_VERSION          0xD1



char pfile[1028] = {'\0'};
char sfile[1028] = {'\0'};

struct vsc_command {
	uint8_t data[256];
	int len;
};

static int trimwhitespace(char *in, char *out)
{
	char *s, *d;

	s = in;
	d = out;

	while (*s != 0) {
		if (!isspace(*s) && *s != '\"') {
			*d = *s;
			d++;
		}
		s++;
	}

	return (d - out);
}

static int hexstring_to_bytearray(const char *hexstr, unsigned char *buf, int len)
{
    int i = 0;
    const char *pos = hexstr;

    /* WARNING: no sanitization or error-checking whatsoever */
    for(i = 0; i < len; i++) {
        sscanf(pos, "%2hhx", &buf[i]);

        pos += 2;
    }

    return 0;
}

static uint8_t *vsc_find_next_token(uint8_t *buf, uint8_t tag, int *len)
{
	uint8_t *p = buf;
	int PI;
	int PL;
	uint8_t *PV;

	while (*len >= 2) {
		PI = p[0];
		PL = p[1];
		PV = &p[2];

		if (PI == tag)
			break;

		*len -= (PL + 2);
		p = &PV[PL];
	}

	return  p;
}

int vsc_parse_validation_number(uint8_t *buf, int size)
{
	int ret = EINVAL;
	uint8_t *p = buf;
	int PI, PL;
	uint8_t *PV;
	int i;

	(void) PI;

	while (size > 2) {
		p = vsc_find_next_token(p, VSC_PI_VALIDATION_NUMBER, &size);

		if (p[0] != VSC_PI_VALIDATION_NUMBER)
			break;

		if (size < 40) {
			printf("VSC_PI_VALIDATION_NUMBER parameter too short:%d", size);
			ret = EINVAL;
			break;
		}

		ret = 0;

		PI = p[0];
		PL = p[1];
		PV = &p[2];

		printf("validation_number: \n");
		for (i = 0; i < PL; i++)
			printf("%02x", PV[i]);
		printf("\n");

		break;
	}

	return ret;
}

static int vsc_read_perso_file(char *path, struct vsc_command *cmds, int max_size)
{
	int in;
	int ret = 0, len, total;
	struct stat st;
	char *buf = NULL;
	char *tmp = NULL;
	char *saveptr;
	char *pch;
	int i = 0;

	memset(cmds, 0x00, max_size * sizeof(struct vsc_command));

	in = open(path, O_RDONLY);
	if (in == FD_INVALID) {
		AUI_PRINTF("open %s failed err=%s", path, strerror(errno));
		return -1;
	}

	ret = fstat(in, &st);
	if (ret) {
		AUI_PRINTF("fstat %s failed err=%s", path, strerror(errno));
		return -1;
	}

	total = st.st_size;

	buf = malloc(total + 1);
	if (!buf) {
		AUI_PRINTF("malloc failed");
		goto exit_close;
	}

	tmp = buf;

	len = read(in, buf, total);
	if (len != total) {
		AUI_PRINTF("readv error ret=%d err=%s", len, strerror(errno));
		goto exit_free;
	}

	buf[total] = '\0';

	/* remove all white spaces to make parsing easier */
	total = trimwhitespace(buf, buf);
	buf[total-2] = '\0';

	/* locate start of persoCommands */
	buf = strstr(buf, "[") + 1;

	/* split into list of perso commands */
	pch = strtok_r(buf, ",", &saveptr);
	while (pch != NULL && i < max_size) {
		hexstring_to_bytearray(&pch[0], cmds[i].data, strlen(&pch[0]) - 0);
		cmds[i].len = (strlen(pch) - 0) / 2;
		pch = strtok_r(NULL, ",", &saveptr);
		i++;
	}

	free(tmp);
	close(in);
	return 0;

exit_free:
	free(tmp);
exit_close:
	close(in);
	return -1;
}

int do_config_write(char *infile)
{
	struct iovec iov[3];
	int in;
	int ret = 0, len, total;
	aui_hdl hdl;
	aui_conaxvsc_attr attr;
	aui_conaxvsc_store *store = NULL;

	store = calloc(sizeof(aui_conaxvsc_store), 1);
	if (!store) {
		AUI_PRINTF("calloc failed\n");
		return ENOMEM;
	}

	in = open(infile, O_RDONLY);
	if (in == FD_INVALID) {
		ret = errno;
		printf("open %s failed err=%s\n", infile, strerror(errno));
		return ret;
	}

	iov[0].iov_base = store->p_uc_data;
	iov[0].iov_len = AUI_CONAXVSC_STORE_DATA_LEN;
	iov[1].iov_base = store->p_uc_key;
	iov[1].iov_len = AUI_CONAXVSC_STORE_KEY_LEN;
	iov[2].iov_base = store->p_uc_hash;
	iov[2].iov_len = AUI_CONAXVSC_STORE_HASH_LEN;

	total = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;

	len = readv(in, iov, 3);
	if (len != total) {
		AUI_PRINTF("readv error ret=%d err=%s\n", len, strerror(errno));
		ret = EIO;
		goto exit;
	}

	memset(&attr, 0, sizeof(aui_conaxvsc_attr));
	attr.uc_dev_idx = 0;
	attr.p_vsc_store = store;
	aui_conaxvsc_open(&attr, &hdl);

exit:
	close(in);
	return ret;
}

static void vsc_store_data_fun_cb(aui_conaxvsc_store *store, void *user_data)
{
	struct iovec iov[3];
	int in;
	int len, total;
	char *path = (char *) user_data;

	if (!path || !strlen(path)) {
		printf("store file path not set\n");
		return;
	}

	printf("writing store data to file: %s\n", path);

	in = creat(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (in == FD_INVALID) {
		printf("open %s failed err=%s\n", path, strerror(errno));
		return;
	}

	iov[0].iov_base = store->p_uc_data;
	iov[0].iov_len = AUI_CONAXVSC_STORE_DATA_LEN;
	iov[1].iov_base = store->p_uc_key;
	iov[1].iov_len = AUI_CONAXVSC_STORE_KEY_LEN;
	iov[2].iov_base = store->p_uc_hash;
	iov[2].iov_len = AUI_CONAXVSC_STORE_HASH_LEN;

	total = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;

	len = writev(in, iov, 3);
	if (len != total) {
		printf("writev error ret=%d err=%s\n", len, strerror(errno));
		goto exit;
	}

exit:
	close(in);
}

int do_personalization(char *outfile, struct vsc_command *cmds, unsigned int cnt)
{
	aui_hdl hdl;
	aui_conaxvsc_attr attr;
	aui_conaxvsc_store_callback_attr cbattr;
	aui_conaxvsc_tx_buf *command;
	aui_conaxvsc_tx_buf *response;
	int sw1, sw2;
	unsigned int i = 0;

	int ret = 0;

	command = (aui_conaxvsc_tx_buf *) malloc(sizeof(aui_conaxvsc_tx_buf));
	if (!command) {
		printf("malloc failed\n");
		return -1;
	}


	response = (aui_conaxvsc_tx_buf *) malloc(sizeof(aui_conaxvsc_tx_buf));
	if (!response) {
		printf("malloc failed\n");
		return -1;
	}

	memset(&attr, 0, sizeof(aui_conaxvsc_attr));
	attr.uc_dev_idx = 0;
	aui_conaxvsc_open(&attr, &hdl);

	memset(&cbattr, 0, sizeof(aui_conaxvsc_store_callback_attr));
	cbattr.pv_user_data = outfile;
	cbattr.p_fn_conaxvsc_store_cb = vsc_store_data_fun_cb;
	aui_conaxvsc_register_store_callback(hdl, &cbattr);

	/* send commands */
	for (i = 0; i < cnt; i++) {
		if (!cmds[i].len)
			break;

		memcpy(command->p_uc_data, cmds[i].data, cmds[i].len);
		command->n_size = cmds[i].len;

		ret = aui_conaxvsc_cmd_transfer(hdl, 0, command, response,
			&sw1, &sw2);
		if (ret || sw1 != 0x90 || sw2 != 0x00) {
			printf("aui_conaxvsc_cmd_transfer %d failed. ret=%d sw=%02x%02x\n",
				i, ret, sw1, sw2);
			goto exit;
		}

		/* TODO: implement response processing here to
		 * obtain validation number from vsc */
		if (response->n_size) {
			vsc_parse_validation_number(response->p_uc_data, response->n_size);
		}
	}
	sleep(2);

exit:
	aui_conaxvsc_unregister_store_callback(hdl);
	aui_conaxvsc_close(hdl);

	free(command);
	free(response);

	return ret;
}

int decypt_file(char *infile, char *outfile)
{
	(void) infile;
	(void) outfile;
	return 0;
}

void print_help(char *sz_appname)
{
	printf("Usage %s [OPTIONS]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t--perso\t\t<file> in Conax Personalizer JSON format\n");
	//printf("\t\t\t\tresulting store data file is created in 00000.store\n");
	printf("\t--config\t<file> store data upload from file\n");
	printf("\t--help\t\t-h Print this help\n");
}

int parse_input(int argc, char **argv)
{
	int c;
	int option_index = 0;

	char *short_options = "s:i:o:f:p:e:h";

	struct option long_options[] = {
		{"config", required_argument, 0, '0'},
		{"perso", required_argument, 0, '1'},
		{"help", required_argument, 0, 'h'},
	};

	while ((c = getopt_long_only(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
		case '1':
			strcpy(pfile, optarg);
			break;
		case '0':
			strcpy(sfile, optarg);
			break;
		case 'h':
		default:
			return -1;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;

	ret = parse_input(argc, argv);
	if (ret) {
		print_help(argv[0]);
		exit(-1);
	}

	if (strlen(pfile)) {
		struct vsc_command *cmds;
		int maxsize = 128;

		cmds = malloc(maxsize * sizeof(struct vsc_command));
		if (!cmds) {
			printf("malloc failed\n");
			return -1;
		}

		ret = vsc_read_perso_file(pfile, cmds, maxsize);
		if (ret < 0) {
			printf("vsc_read_perso_file failed\n");
			return -1;
		}

		printf("Start VSC personalisation: %s\n", pfile);
		do_personalization("00000.store", cmds, maxsize);
		return 0;
	}
	else if (strlen(sfile)) {
		printf("Start VSC store config upload: %s\n", sfile);
		do_config_write(sfile);
		return 0;
	}
	else {
		printf("Permitted modes are --perso and --config\n");
		print_help(argv[0]);
		exit(-1);
	}

	return 0;
}
