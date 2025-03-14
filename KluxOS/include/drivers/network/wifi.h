#ifndef _WIFI_H
#define _WIFI_H

#include <kernel/types.h>
#include <kernel/net/network.h>

// WiFi yapılandırma yapısı
struct wifi_config {
    char ssid[32];
    char password[64];
    uint8_t security_type;  // WPA, WPA2, vb.
    uint8_t channel;
};

// WiFi cihaz yapısı
struct wifi_device {
    struct network_device net_dev;  // Temel ağ cihazı
    struct wifi_config config;
    uint8_t current_state;         // Bağlantı durumu
    int signal_strength;           // Sinyal gücü
};

// WiFi güvenlik tipleri
#define WIFI_SECURITY_NONE  0
#define WIFI_SECURITY_WEP   1
#define WIFI_SECURITY_WPA   2
#define WIFI_SECURITY_WPA2  3

// WiFi durumları
#define WIFI_STATE_DOWN     0
#define WIFI_STATE_SCANNING 1
#define WIFI_STATE_CONNECTING 2
#define WIFI_STATE_CONNECTED 3

// WiFi fonksiyonları
int wifi_init(void);
int wifi_scan(void);
int wifi_connect(const char* ssid, const char* password);
int wifi_disconnect(void);
int wifi_get_signal_strength(void);

#endif 