#ifndef DUNKASM_BUFFER_H
#define DUNKASM_BUFFER_H

#define MAX_BUFFER_SIZE (64 * 1024)

struct dasm_buffer {
	uint16_t *data;
	unsigned int buf_size;
	unsigned int length;
	unsigned int position;
};

int init_buffer(dasm_buffer *buf);
dasm_buffer *new_buffer();
void destroy_buffer(dasm_buffer *buf);
void free_buffer(dasm_buffer *buf);

int append_buffer(dasm_buffer *buf, uint16_t w);
int concat_buffers(dasm_buffer *buf1, dasm_buffer *buf2);
int write_buffer_at(dasm_buffer *buf, uint16_t w, unsigned int pos);

int expand_buffer(dasm_buffer *buf);
int expand_buffer_to_min(dasm_buffer *buf, unsigned int min);

int writeout_buffer(dasm_buffer *buf, FILE *outfile);

int valid_dasm_buffer(const dasm_buffer *buf);
void display_buffer(dasm_buffer *buf, const char *message);

#endif
