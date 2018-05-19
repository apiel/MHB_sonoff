#include "../upnp_utils.cpp"
#include <gtest/gtest.h>

const char * req_false_state = "Host: 192.168.0.66\n\r"
"Accept: */*\n\r"
"Content-type: application/x-www-form-urlencoded\n\r"
"Content-Length: 13\n\r\n\r"
"{\"on\": false}";

const char * req_false_state2 = "Host: 192.168.0.66\n\r"
"Accept: */*\n\r"
"Content-type: application/x-www-form-urlencoded\n\r"
"Content-Length: 13\n\r\n\r"
"{\"on\": false}\n\r";

const char * req_true_state = "Host: 192.168.0.66\n\r"
"Accept: */*\n\r"
"Content-type: application/x-www-form-urlencoded\n\r"
"Content-Length: 12\n\r\n\r"
"{\"on\": true}";

const char * req_true_state2 = "Host: 192.168.0.66\n\r"
"Accept: */*\n\r"
"Content-type: application/x-www-form-urlencoded\n\r"
"Content-Length: 12\n\r\n\r"
"{\"on\": true}\n\r";

TEST(upnp_utils_get_requested_state, try_false)
{
	ASSERT_EQ("false", upnp_utils_get_requested_state((char *)req_false_state));
    ASSERT_EQ("false", upnp_utils_get_requested_state((char *)req_false_state2));
}

TEST(upnp_utils_get_requested_state, try_true)
{
	ASSERT_EQ("true", upnp_utils_get_requested_state((char *)req_true_state));
    ASSERT_EQ("true", upnp_utils_get_requested_state((char *)req_true_state2));
}
