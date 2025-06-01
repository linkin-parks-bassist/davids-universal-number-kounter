#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "dunkasm.h"

#define INIT_BUFSIZE 1024

int init_buffer(dasm_buffer *buf)
{
	if (buf == NULL)
		return BAD_ARGUMENTS;
	
	buf->buf_size = INIT_BUFSIZE;
	buf->data = malloc(INIT_BUFSIZE * sizeof(uint16_t));
	
	if (buf->data == NULL) {
		return MEMORY_FAILURE;
	}
	
	for (int i = 0; i < INIT_BUFSIZE; i++) {
		buf->data[i] = 0;
	}
	
	buf->position = 0;
	buf->length = 0;
	
	return SUCCESS;
}

dasm_buffer *new_buffer()
{
	dasm_buffer *res = malloc(sizeof(dasm_buffer));
	
	if (res == NULL)
		return NULL;
	
	init_buffer(res);
	
	return res;
}


void destroy_buffer(dasm_buffer *buf)
{
	if (buf == NULL)
		return;
	
	if (buf->data != NULL)
		free(buf->data);
}


void free_buffer(dasm_buffer *buf)
{
	if (buf == NULL)
		return;
	
	destroy_buffer(buf);
	
	free(buf);
}

int append_buffer(dasm_buffer *buf, uint16_t w)
{
	if (!buf)
		return BAD_ARGUMENTS;
	
	if (buf->position >= buf->buf_size) {
		if (expand_buffer(buf))
			return MEMORY_FAILURE;
	}
	
	buf->data[buf->position] = w;
	buf->position++;
	
	if (buf->position > buf->length)
		buf->length = buf->position;

	
	return SUCCESS;
}

int write_buffer_at(dasm_buffer *buf, uint16_t w, unsigned int pos)
{
	if (buf == NULL)
		return BAD_ARGUMENTS;
	
	if (buf->buf_size <= pos) {
		if (expand_buffer_to_min(buf, pos))
			return MEMORY_FAILURE;
	}
	
	buf->data[pos] = w;
	
	if (pos > buf->length)
		buf->length = pos;
	
	return SUCCESS;
}

int concat_buffers(dasm_buffer *buf1, dasm_buffer *buf2)
{
	if (buf1 == NULL || buf2 == NULL)
		return BAD_ARGUMENTS;
	
	if (buf1->data == NULL || buf2->data == NULL)
		return BAD_ARGUMENTS;
	
	unsigned int new_length = buf1->length + buf2->length;
	
	if (buf1->buf_size <= new_length) {
		if (expand_buffer_to_min(buf1, new_length))
			return MEMORY_FAILURE;
	}
	
	memcpy(&(buf1->data[buf1->length]), buf2->data, buf2->length * sizeof(uint16_t));
	
	buf1->length = new_length;
	buf1->position = new_length;
	
	return SUCCESS;
}

int expand_buffer(dasm_buffer *buf)
{
	if (buf == NULL)
		return BAD_ARGUMENTS;
	
	if (buf->data == NULL) {
		buf->buf_size = INIT_BUFSIZE;
		buf->data = malloc(sizeof(uint16_t) * INIT_BUFSIZE);
	} else {
		if (buf->buf_size == 0)
			buf->buf_size = INIT_BUFSIZE;
		else
			buf->buf_size = 2 * buf->buf_size;
		buf->data = realloc(buf->data, buf->buf_size);
	}
	
	return SUCCESS;
}

int expand_buffer_to_min(dasm_buffer *buf, unsigned int min)
{
	if (buf == NULL)
		return BAD_ARGUMENTS;
	
	if (buf->data == NULL) {
		buf->buf_size = min * 2;
		buf->data = malloc(sizeof(uint16_t) * buf->buf_size);
	} else {
		if (buf->buf_size == 0) {
			buf->buf_size = min * 2;
		} else {
			while (buf->buf_size <= min)
				buf->buf_size = 2 * buf->buf_size;
		}
		buf->data = realloc(buf->data, buf->buf_size);
		
		if (buf->data == NULL)
			return MEMORY_FAILURE;
	}
	
	return SUCCESS;
}

int writeout_buffer(dasm_buffer *buf, FILE *outfile, int format)
{
	if (buf == NULL || outfile == NULL)
		return BAD_ARGUMENTS;
	
	if (buf->data == NULL)
		return BAD_ARGUMENTS;
	
	if (format == OUTPUT_FORMAT_BINARY)
	{
		int return_value = fwrite(buf->data, sizeof(uint16_t), buf->length, outfile);
		
		if (return_value != buf->length)
		{
			return 2;
		}
	}
	else if (format == OUTPUT_FORMAT_V3_HEX)
	{	
		fputs("v3.0 hex words addressed", outfile);
		
		for (int i = 0; i < buf->length; i++)
		{
			if (i % 8 == 0)
				fprintf(outfile, "\n%04x:", i);
			
			fprintf(outfile, " %05x", buf->data[i]);
		}
	}
	
	return SUCCESS;
}


int valid_dasm_buffer(const dasm_buffer *buf)
{
	if (buf == NULL)
		return 0;
	
	if (buf->data == NULL)
		return 0;
	
	if (buf->position > buf->length)
		return 0;
	
	return 1;
}

void display_buffer(dasm_buffer *buf, const char *message)
{
	printf("Printing buffer");
	if (message != NULL) {
		printf(" %s", message);
	}
	putc('\n', stdout);
	
	if (!valid_dasm_buffer(buf)) {
		printf("It's busted...\n");
		return;
	}
	
	for (int i = 0; i < buf->length; i++) {
		if (i % 8 == 0) {
			printf("\n	0x%04x |  ", i);
		}
		printf("0x%04x ", buf->data[i]);
	}
	printf("\n\nBuffer length: 0x%04x\n", buf->length);
	
	printf("\n\n");
}
