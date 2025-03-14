#ifndef WIFI_H
#define WIFI_H

#include <kernel/types.h>

#define WIFI_STATE_DOWN       0
#define WIFI_STATE_READY      1
#define WIFI_STATE_SCANNING   2
#define WIFI_STATE_CONNECTING 3
#define WIFI_STATE_CONNECTED  4


#define WIFI_SECURITY_NONE     0
#define WIFI_SECURITY_WEP      1
#define WIFI_SECURITY_WPA_PSK  2
#define WIFI_SECURITY_WPA2_PSK 3


typedef struct {
    char ssid[32];
    char password[64];
    uint8_t security_type;
} wifi_connection_params_t;


struct wifi_device;

typedef struct {
    int (*scan)(struct wifi_device* dev);
    int (*connect)(struct wifi_device* dev, wifi_connection_params_t* params);
    int (*disconnect)(struct wifi_device* dev);
} wifi_driver_t;


typedef struct {
    char ssid[32];
    char password[64];
} wifi_config_t;


typedef struct wifi_device {
    uint8_t current_state;
    uint8_t signal_strength;
    uint8_t ip_address[4];
    uint8_t connected_channel;
    uint32_t connection_timeout;
    uint32_t connection_start_time;
    uint8_t networks_found;
    wifi_config_t config;
    wifi_driver_t* driver;
} wifi_device_t;


typedef struct network_device {
    uint8_t type;
    uint8_t mac_address[6];
} network_device_t;


extern struct network_device* current_device;


int wifi_init(void);
int wifi_scan(void);
int wifi_connect(const char* ssid, const char* password);
int wifi_disconnect(void);

#endif // WIFI_H 