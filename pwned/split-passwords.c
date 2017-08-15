#include <stdio.h>
#include <string.h>

#define HASH_LEN 40
#define PRFX_LEN 2
#define BUF_LINES 512

int main(int argc, char *argv[]) {

  char prefix[] = { '\0', '\0', '\0' };
  char buf[(HASH_LEN - PRFX_LEN + 2) * BUF_LINES + 1];
  int buffer_len = (HASH_LEN - PRFX_LEN + 2) * BUF_LINES + 1;
  int buffer_idx = 0;
  memset(buf, 0, buffer_len);

  char input[HASH_LEN + 3];
  FILE * out = NULL;
  
  while(1) {
    char * ret = fgets(input, HASH_LEN + 3, stdin);
    if (!ret) {
      return 0;
    }

    if (strlen(input) < HASH_LEN + 2) {
      fprintf(stderr, "Received hash of length less than %d: %s\n", HASH_LEN, input);
      if (feof(stdin)) {
	return 0;
      } else {
	continue;
      }
    }

    if (!*prefix || prefix[0] != input[0] || prefix[1] != input[1]) {
      if (out) {
	if (buffer_idx > 0) {
	  fputs(buf, out);
	  memset(buf, 0, buffer_idx);
	  buffer_idx = 0;
	}
	fclose(out);
	out = NULL;
      }

      prefix[0] = input[0];
      prefix[1] = input[1];

      out = fopen(prefix, "wb");
      if (!out) {
	fprintf(stderr, "Unable to open file %s\n", prefix);
	return 0;
      }
    }

    if (buffer_idx + HASH_LEN - PRFX_LEN + 1 >= buffer_len) {
      fputs(buf, out);
      memset(buf, 0, buffer_idx);
      buffer_idx = 0;
    }

    ret = strncpy(buf + buffer_idx, input + PRFX_LEN, HASH_LEN - PRFX_LEN);
    buffer_idx += HASH_LEN - PRFX_LEN;
    buf[buffer_idx++] = '\n';
  }

  return 0;
}
