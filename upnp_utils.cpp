#include <string.h>

#include "utils.h"

// Host: 192.168.0.66
// Accept: */*
// Content-type: application/x-www-form-urlencoded
// Content-Length: 13

// {"on": false}

char * upnp_utils_get_requested_state(char * data)
{
    // printf("upnp_utils_get_requested_state: %s\n", data);
    // char * state = data + strlen(data) - 9; // "false}" or " true}" // true false not done correctly, to fix

    static char state[32];
    str_extract(data, '{', '}', state);

    // printf("the state: %s\n", state);
    return state;
}