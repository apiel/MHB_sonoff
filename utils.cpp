#include <string.h>

char * str_extract(char * str, int start, int end, char * ret)
{
    char *str_start, *str_end;
    if (start > 0) str_start = strchr(str, start)+1;
    else str_start = str;
    str_end = strchr(str_start, end);
    int len = str_end - str_start;
    // if (len > strlen(ret)) len = strlen(ret);  // it could be out of memory?
    memcpy(ret, str_start, len);
    ret[len] = '\0';

    return str_end;
}

void char_replace(char * str, char search, char replace)
{
    for(int pos = strlen(str); pos > 0; pos--) {
        if (str[pos] == search) str[pos] = replace;
    }
}

int char_to_int(char * str)
{
    int value = 0;
    for(int pos = 0, end = strlen(str); pos < end; pos++) {
        if (str[pos] >= '0' && str[pos] <= '9') {
            value = value * 10 + str[pos] - '0';
        } else {
            break;
        }
    }
    return value;
}
