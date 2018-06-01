#include "config.h"
#include "upnp_utils.h"
#include "relay.h"

#include <unistd.h>
#include <string.h>
#include <lwip/udp.h>
#include <lwip/igmp.h>
#include <lwip/api.h>
#include <espressif/esp_common.h>

#include "upnp.h"
// #include "action.h"

#define UPNP_MCAST_GRP  ("239.255.255.250")
#define UPNP_MCAST_PORT (1900)

struct HUEitems
{
    const char * name;
    Relay * relay;
};

// need define RELAY_NAME
#define RELAY_NAME "bed light"
#define RELAY_2_NAME "table light"

struct HUEitems hueItems[] = {
#if defined(RELAY_NAME) && defined(PIN_RELAY)
    { "table light", &Relay1},
#endif
#if defined(RELAY_2_NAME) && defined(PIN_RELAY_2)
    { "kitchen light", &Relay2},
#endif
};

uint8_t hueItems_count = sizeof(hueItems) / sizeof(hueItems[0]);


static const char* get_my_ip(void)
{
    static char ip[16] = "0.0.0.0";
    ip[0] = 0;
    struct ip_info ipinfo;
    (void) sdk_wifi_get_ip_info(STATION_IF, &ipinfo);
    snprintf(ip, sizeof(ip), IPSTR, IP2STR(&ipinfo.ip));
    return (char*) ip;
}

/**
  * @brief This function joins a multicast group witht he specified ip/port
  * @param group_ip the specified multicast group ip
  * @param group_port the specified multicast port number
  * @param recv the lwip UDP callback
  * @retval udp_pcb* or NULL if joining failed
  */
static struct udp_pcb* mcast_join_group(char *group_ip, uint16_t group_port, void (* recv)(void * arg, struct udp_pcb * upcb, struct pbuf * p, const ip4_addr * addr, short unsigned int port))
{
    bool status = false;
    struct udp_pcb *upcb;

    printf("Joining mcast group %s:%d\n", group_ip, group_port);
    do {
        upcb = udp_new();
        if (!upcb) {
            printf("Error, udp_new failed");
            break;
        }
        udp_bind(upcb, IP_ADDR_ANY, group_port);
        struct netif* netif = sdk_system_get_netif(STATION_IF);
        if (!netif) {
            printf("Error, netif is null");
            break;
        }
        if (!(netif->flags & NETIF_FLAG_IGMP)) {
            netif->flags |= NETIF_FLAG_IGMP;
            igmp_start(netif);
        }
        ip_addr_t ipgroup;
        ipaddr_aton(group_ip, &ipgroup);
        err_t err = igmp_joingroup(&netif->ip_addr, &ipgroup);
        if(ERR_OK != err) {
            printf("Failed to join multicast group: %d", err);
            break;
        }
        status = true;
    } while(0);

    if (status) {
        printf("Join successs\n");
        udp_recv(upcb, recv, upcb);
    } else {
        if (upcb) {
            udp_remove(upcb);
        }
        upcb = NULL;
    }
    return upcb;
}

static void send(struct udp_pcb *upcb, const ip_addr_t * addr, u16_t port)
{
    struct pbuf *p;
    char msg[500];

    snprintf(msg, sizeof(msg),
        "HTTP/1.1 200 OK\r\n"
        "NT: urn:schemas-upnp-org:device:basic:1\r\n"
        "SERVER: node.js/0.10.28 UPnP/1.1\r\n"
        "ST: urn:schemas-upnp-org:device:basic:1\r\n"
        "USN: uuid:Socket-1_0-221438K0100073::urn:Belkin:device:**\r\n"
        "LOCATION: http://%s:80/api/setup.xml\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "CACHE-CONTROL: max-age=1800\r\n"
        "EXT: \r\n"
        "DATE: Fri, 21 Jul 2017 17:58:41 GMT\r\n\r\n", get_my_ip());

    p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg)+1, PBUF_RAM);

    if (!p) {
        printf("Failed to allocate transport buffer\n");
    } else {
        memcpy(p->payload, msg, strlen(msg)+1);
        err_t err = udp_sendto(upcb, p, addr, port);
        if (err < 0) {
            printf("Error sending message: %s (%d)\n", lwip_strerr(err), err);
        }
        // else {
        //     printf("Sent message '%s'\n", msg);
        // }
        pbuf_free(p);
    }
}

/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
static void receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t * addr, short unsigned int port)
{
    if (p) {
        // printf("Msg received port:%d len:%d\n", port, p->len);
        // uint8_t *buf = (uint8_t*) p->payload;
        // printf("Msg received port:%d len:%d\nbuf: %s\n", port, p->len, buf);

        send(upcb, addr, port);

        pbuf_free(p);
    }
}

/**
  * @brief Initialize the upnp server
  * @retval true if init was succcessful
  */
bool upnp_server_init(void)
{
    printf("Upnp server init\n\r");
    struct udp_pcb *upcb = mcast_join_group((char*)UPNP_MCAST_GRP, UPNP_MCAST_PORT, receive_callback);
    return (upcb != NULL);
}

void upnp_task(void *pvParameters)
{
    bool ok = false;
    printf("Upnp task\n\r");

    while (1) {
        if (!ok) ok = upnp_server_init();
        vTaskDelay(1000);
    }
}

char * upnp_setup_response()
{
    return (char *)
        "<?xml version=\"1.0\"?>"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
        "<specVersion>"
        "<major>1</major>"
        "<minor>0</minor>"
        "</specVersion>"
        "<URLBase>http://192.168.42.102:8082/</URLBase>"
        "<device>"
        "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
        "<friendlyName>Amazon-Echo-HA-Bridge (192.168.42.102)</friendlyName>"
        "<manufacturer>Royal Philips Electronics</manufacturer>"
        "<manufacturerURL>http://www.armzilla..com</manufacturerURL>"
        "<modelDescription>Hue Emulator for Amazon Echo bridge</modelDescription>"
        "<modelName>Philips hue bridge 2012</modelName>"
        "<modelNumber>929000226503</modelNumber>"
        "<modelURL>http://www.armzilla.com/amazon-echo-ha-bridge</modelURL>"
        "<serialNumber>01189998819991197253</serialNumber>"
        "<UDN>uuid:88f6698f-2c83-4393-bd03-cd54a9f8595</UDN>"
        "<serviceList>"
        "<service>"
        "<serviceType>(null)</serviceType>"
        "<serviceId>(null)</serviceId>"
        "<controlURL>(null)</controlURL>"
        "<eventSubURL>(null)</eventSubURL>"
        "<SCPDURL>(null)</SCPDURL>"
        "</service>"
        "</serviceList>"
        "<presentationURL>index.html</presentationURL>"
        "<iconList>"
        "<icon>"
        "<mimetype>image/png</mimetype>"
        "<height>48</height>"
        "<width>48</width>"
        "<depth>24</depth>"
        "<url>hue_logo_0.png</url>"
        "</icon>"
        "<icon>"
        "<mimetype>image/png</mimetype>"
        "<height>120</height>"
        "<width>120</width>"
        "<depth>24</depth>"
        "<url>hue_logo_3.png</url>"
        "</icon>"
        "</iconList>"
        "</device>"
        "</root>";
}

char * upnp_config_response()
{
    static char item[1024];
    // static char response[sizeof(item)* sizeof(hueItems) / sizeof(hueItems[0])];
    static char response[4000];
    strcpy(response, "{\"lights\":{");
    for (int index = 0; index < hueItems_count; index++) {
        if (index > 0) {
            strcat(response, ",");
        }
        snprintf(item, sizeof(item),
            "\"abcd1234ef0%d\": {\"name\": \"%s\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"abcd1234ef0%d\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}}",
            index, hueItems[index].name, index);
        strcat(response, item);
    }
    strcat(response, "}}");

    printf("-->> config response: %s\n", response);
    return response;

    // return
    //     "{\"lights\":{"
    //     "\"5102d46c-50d5-4bc7-a180-38623e4bbb00\": {\"name\": \"windows lights\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}},"
    //     "\"abcd1234ef1\": {\"name\": \"bed lights\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}},"
    //     "\"5102d46c-50d5-4bc7-a180-38623e4bbb02\": {\"name\": \"room lights\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}},"
    //     "\"5102d46c-50d5-4bc7-a180-38623e4bbb03\": {\"name\": \"kitchen lights\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}},"
    //     "\"5102d46c-50d5-4bc7-a180-38623e4bbb04\": {\"name\": \"toilet lights\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}},"
    //     "\"5102d46c-50d5-4bc7-a180-38623e4bbb05\": {\"name\": \"wall lights\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}},"
    //     "\"5102d46c-50d5-4bc7-a180-38623e4bbb06\": {\"name\": \"store\", \"state\": {\"on\": false, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}}"
    //     "}}";
}

char theResponse[512];

char * upnp_state_response(char * request)
{
    char strIndex[2]; // duplicate code from next function
    strncpy(strIndex, request + strlen(request) - 2, 2); // duplicate code from next function
    unsigned int index = atoi(strIndex); // duplicate code from next function

    printf("return state (%d)[%s]: %s\n", index, strIndex, request);
    // return (char *)
    //     "HTTP/1.1 200 OK\r\n"
    //     "Content-type: application/json\r\n\r\n"
    //     "{\"name\": \"generic\", \"state\": {\"on\": true, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}}";

    // sprintf(response, "HTTP/1.1 200 OK\r\n"
    sprintf(theResponse, "HTTP/1.1 200 OK\r\n"
        "Content-type: application/json\r\n\r\n"
    // sprintf(theResponse, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nDate: Fri, 01 Jun 2018 22:17:37 GMT\r\nConnection: keep-alive\r\nTransfer-Encoding: chunked\r\n\r\n"
        // "{\"name\": \"generic\", \"state\": {\"on\": %s, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"modelid\": \"LCT001\", \"manufacturername\": \"Philips\", \"uniqueid\": \"5102d46c-50d5-4bc7-a180-38623e4bbb08\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}}",
        // "{\"state\": {\"on\": %s, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"ct\": 313, \"alert\": \"none\", \"colormode\": \"ct\", \"ct\": 365, \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"name\": \"%s\", \"modelid\": \"LCT004\", \"manufacturername\": \"Philips\", \"uniqueid\": \"abcd1234ef0%d\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}}",
        "{\"state\": {\"on\": %s, \"bri\": 254, \"hue\": 15823, \"sat\": 88, \"effect\": \"none\", \"alert\": \"none\", \"colormode\": \"ct\", \"ct\": 365, \"reachable\": true, \"xy\": [0.4255, 0.3998]}, \"type\": \"Extended color light\", \"name\": \"%s\", \"modelid\": \"LCT004\", \"manufacturername\": \"Philips\", \"uniqueid\": \"abcd1234ef0%d\", \"swversion\": \"65003148\", \"pointsymbol\": {\"1\": \"none\", \"2\": \"none\", \"3\": \"none\", \"4\": \"none\", \"5\": \"none\", \"6\": \"none\", \"7\": \"none\", \"8\": \"none\"}}",
        hueItems[index].relay->relay_status() == RELAY_ON ? "true" : "false", hueItems[index].name, index);

    printf("the response: %s\n", theResponse);
    return theResponse;
}

char * upnp_update_state(char * request, char * data)
{
    char strIndex[2];
    strncpy(strIndex, request + strlen(request) - 8, 2);
    unsigned int index = atoi(strIndex);
    char * state = upnp_utils_get_requested_state(data);
    if (index < hueItems_count) {
        printf("change state: %d :: %s :: %s\n", index, hueItems[index].name, state);
        char params[512];
        if (strcmp(state, "\"on\": true") == 0) { // upnp_utils_get_requested_state here we use, == "on": true
            strcpy(params, hueItems[index].relay->get_id());
            (* hueItems[index].relay)(ACTION_RELAY_ON);
        } else { // upnp_utils_get_requested_state here we use, == "on": false
            strcpy(params, hueItems[index].relay->get_id());
            (* hueItems[index].relay)(ACTION_RELAY_OFF);
        } // we could upnp_utils_get_requested_state check "bri": 90 for sonoff bulb
        // reducer(hueItems[index].action, params);
        printf("here we go, we need to update state: %s then: %s\n", hueItems[index].name, params);
    }
    return state;
}

// {"bri":127} // not supported yet, need to respond with bri instead of on
// {"on": true}

// Host: 192.168.0.66
// Accept: */*
// Content-type: application/x-www-form-urlencoded
// Content-Length: 13

// {"on": false}

char * upnp_update_state_response(char * request, char * data)
{
    // printf("update state data(%s): %s\n", request, data);
    // char * state = data + strlen(data) - 9; // "false}" or " true}" // true false not done correctly, to fix
    char * state = upnp_update_state(request, data);

    char * light = request + 40;
    static char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-type: application/json\r\n\r\n"
        // "[{\"success\":{\"/lights/abcd1234ef01/state/on\":true}}]");
        "[{\"success\":{\"%s/on\":%s}}]", light, strcmp(state, "\"on\": true") == 0 ? "true" : "false");
    printf("upnp state response: %s\n", response);

    return response;
}

char * upnp_action(char * request, char * data)
{
    char * response = NULL;

    // printf("Upnp action: %s\ndata %s\n", request, data);
    printf("the request: %s\n", request);
    if (strcmp(request, "setup.xml") == 0) {
       response = upnp_setup_response();
    } else if (strcmp(request, "config.json") == 0
    || strcmp(request, "S6QJ3NqpQzsR6ZFzOBgxSRJPW58C061um8oP8uhf") == 0) {
        response = upnp_config_response();
    } else {
        char isLight[47];
        strncpy(isLight, request, 47);
        // printf("isLight: %s\n", isLight);
        if (strcmp(isLight, "S6QJ3NqpQzsR6ZFzOBgxSRJPW58C061um8oP8uhf/lights") == 0) {
            // printf("yes it is a light action\n");
            if (strcmp(request + strlen(request) - 5, "state") == 0) {
                response = upnp_update_state_response(request, data);
            } else {
                response = upnp_state_response(request);
                printf("------------ %s\n", response);
            }
        }
    }

    return response;
}
