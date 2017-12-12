#ifndef BASE64_H
#define BASE64_H

int base64_decode_bis(size_t in_len, const char *in, size_t out_len, unsigned char *out);
int base64_encode_bis(size_t in_len, const unsigned char *in, size_t out_len, char *out);
#endif
