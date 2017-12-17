#ifndef BASE64_H
#define BASE64_H

int base64_decode(unsigned int in_len, const char *in, int out_len, unsigned char *out);
int base64_encode(unsigned int in_len, const unsigned char *in, int out_len, char *out);
#endif