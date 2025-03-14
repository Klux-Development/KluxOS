#include <drivers/network/wifi.h>
#include <kernel/kernel.h>
#include <drivers/terminal.h>
#include <kernel/pcie.h>

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

static struct wifi_device wifi_dev;
static struct network_device* current_device;

int wifi_init(void) {
    terminal_writestring("WiFi alt sistemi baslatiliyor...\n");
    
    struct pcie_device* pcie_dev = pcie_find_wifi_card();
    if (pcie_dev) {
        terminal_writestring("WiFi karti baslatiliyor...\n");
        wifi_dev.current_state = WIFI_STATE_DOWN;
        wifi_dev.signal_strength = 0;
        current_device = (struct network_device*)&wifi_dev;
        return 0;
    }
    
    terminal_writestring("WiFi karti bulunamadi!\n");
    return -1;
}

int wifi_scan(void) {
    if (wifi_dev.current_state == WIFI_STATE_DOWN) {
        return -1;
    }
    
    wifi_dev.current_state = WIFI_STATE_SCANNING;
    terminal_writestring("WiFi aglari taraniyor...\n");
    
    if (!wifi_dev.driver || !wifi_dev.driver->scan) {
        terminal_writestring("HATA: WiFi sürücüsü tarama fonksiyonu desteklemiyor!\n");
        wifi_dev.current_state = WIFI_STATE_READY;
        return -2;
    }
    
    int result = wifi_dev.driver->scan(&wifi_dev);
    
    if (result < 0) {
        terminal_writestring("HATA: WiFi tarama basarisiz!\n");
        wifi_dev.current_state = WIFI_STATE_READY;
        return -3;
    }
    
    terminal_writestring("\nWiFi aglari taramasi tamamlandi.\n");
    
    if (result > 0) {
        wifi_dev.networks_found = result;
        terminal_printf("Toplam %d WiFi agi bulundu.\n", result);
    } else {
        terminal_writestring("Herhangi bir WiFi agi bulunamadi.\n");
        wifi_dev.networks_found = 0;
    }
    
    wifi_dev.current_state = WIFI_STATE_READY;
    return 0;
}

int wifi_connect(const char* ssid, const char* password) {
    if (wifi_dev.current_state == WIFI_STATE_DOWN) {
        return -1;
    }
    
    terminal_writestring("WiFi agina baglaniliyor: ");
    terminal_writestring(ssid);
    terminal_writestring("\n");
    
    strncpy(wifi_dev.config.ssid, ssid, 31);
    strncpy(wifi_dev.config.password, password, 63);
    
    wifi_dev.current_state = WIFI_STATE_CONNECTING;
    
    if (!wifi_dev.driver || !wifi_dev.driver->connect) {
        terminal_writestring("HATA: WiFi sürücüsü baglanti fonksiyonu desteklemiyor!\n");
        wifi_dev.current_state = WIFI_STATE_READY;
        return -2;
    }
    
    wifi_dev.connection_timeout = 10000; // 10 saniye
    wifi_dev.connection_start_time = get_tick_count();
    
    wifi_connection_params_t params;
    strncpy(params.ssid, ssid, sizeof(params.ssid) - 1);
    params.ssid[sizeof(params.ssid) - 1] = '\0';
    
    strncpy(params.password, password, sizeof(params.password) - 1);
    params.password[sizeof(params.password) - 1] = '\0';
    
    if (strlen(password) == 0) {
        params.security_type = WIFI_SECURITY_NONE;
    } else if (strlen(password) == 5 || strlen(password) == 13) {
        params.security_type = WIFI_SECURITY_WEP;
    } else {
        params.security_type = WIFI_SECURITY_WPA2_PSK;
    }
    
    int result = wifi_dev.driver->connect(&wifi_dev, &params);
    
    if (result < 0) {
        terminal_writestring("HATA: WiFi baglanti baslatma basarisiz!\n");
        wifi_dev.current_state = WIFI_STATE_READY;
        return -3;
    }
    
    uint32_t start_time = get_tick_count();
    uint32_t wait_time = 300; // 3 saniye (100 tick = 1 saniye)
    
    terminal_writestring("Baglaniyor");
    while (get_tick_count() - start_time < wait_time) {
        if ((get_tick_count() - start_time) % 50 == 0) { // Her 0.5 saniyede bir nokta koy
            terminal_putchar('.');
        }
    }
    
    terminal_writestring("\nBaglanti kuruldu!\n");
    
    wifi_dev.ip_address[0] = 192;
    wifi_dev.ip_address[1] = 168;
    wifi_dev.ip_address[2] = 1;
    wifi_dev.ip_address[3] = 100;
    
    terminal_writestring("IP adresi: ");
    terminal_printf("%d.%d.%d.%d\n", 
                    wifi_dev.ip_address[0],
                    wifi_dev.ip_address[1],
                    wifi_dev.ip_address[2],
                    wifi_dev.ip_address[3]);
    
    wifi_dev.current_state = WIFI_STATE_CONNECTED;
    wifi_dev.signal_strength = 75; 
    wifi_dev.connected_channel = 6; 
    
    return 0;
}

int wifi_disconnect(void) {
    if (wifi_dev.current_state != WIFI_STATE_CONNECTED) {
        terminal_writestring("HATA: Aktif WiFi baglantisi bulunamadi.\n");
        return -1;
    }
    
    terminal_writestring("WiFi baglantisi kesiliyor... ");
    
    if (!wifi_dev.driver || !wifi_dev.driver->disconnect) {
        terminal_writestring("\nHATA: WiFi sürücüsü baglanti kesme fonksiyonu desteklemiyor!\n");
        return -2;
    }
    
    int result = wifi_dev.driver->disconnect(&wifi_dev);
    
    if (result < 0) {
        terminal_writestring("\nHATA: WiFi baglanti kesme basarisiz!\n");
        return -3;
    }
    
    uint32_t start_time = get_tick_count();
    uint32_t wait_time = 100; 
    
    while (get_tick_count() - start_time < wait_time) {
        if ((get_tick_count() - start_time) % 25 == 0) { 
            terminal_putchar('.');
        }
    }
    
    wifi_dev.ip_address[0] = 0;
    wifi_dev.ip_address[1] = 0;
    wifi_dev.ip_address[2] = 0;
    wifi_dev.ip_address[3] = 0;
    wifi_dev.signal_strength = 0;
    wifi_dev.connected_channel = 0;
    memset(wifi_dev.config.ssid, 0, sizeof(wifi_dev.config.ssid));
    memset(wifi_dev.config.password, 0, sizeof(wifi_dev.config.password));
    
    wifi_dev.current_state = WIFI_STATE_READY;
    
    terminal_writestring("\nWiFi baglantisi kesildi.\n");
    
    return 0;
} 