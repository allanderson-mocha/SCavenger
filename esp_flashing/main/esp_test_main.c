#include "esp_mac.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "driver/i2c.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "cJSON.h"


// === Configuration ===
#define AP_SSID               "ESP32_SETUP"
#define AP_PASS               ""
#define MAX_STA_CONN          4

#define I2C_SLAVE_NUM         I2C_NUM_0
#define I2C_SLAVE_SDA_IO      8
#define I2C_SLAVE_SCL_IO      9
#define I2C_SLAVE_ADDR        0x28
#define I2C_SLAVE_TX_BUF_LEN  128
#define I2C_SLAVE_RX_BUF_LEN  128

#define WIFI_CONNECTED_BIT    BIT0

#define MAX_RESPONSE_SIZE     16384

#define OVERPASS_URL          "https://overpass-api.de/api/interpreter"
extern const uint8_t _binary_isrg_root_x1_pem_start[] asm("_binary_isrg_root_x1_pem_start");

static const char *TAG = "esp_setup";
static EventGroupHandle_t wifi_event_group;
static char user_ssid[32] = {0};
static char user_pass[64] = {0};
static int retry_count = 0;

char coord_list[3][32];
    int coord_index = 0;
int coord_count = 0;


// Forward declaration for the connect task
static void connect_task(void *pv);
static httpd_handle_t start_webserver(void);
void i2c_listener_task(void *arg);
static void AP_config(void);

// ======== Wi-Fi Event Handler ========
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_count < 6) {
            retry_count++;
            ESP_LOGI(TAG, "Retrying connection... (%d/6)", retry_count);
            esp_wifi_connect();
        } else {
            ESP_LOGW(TAG, "Max retries reached. Giving up.");
            AP_config();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        retry_count = 0;
        ESP_LOGI(TAG, "Got IP! Starting I2C listener...");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

        // Check if task already exists (prevent multiple spawns)
        static bool i2c_started = false;
        if (!i2c_started) {
            i2c_started = true;
            xTaskCreate(i2c_listener_task, "i2c_listener", 4096, NULL, 5, NULL);
        }
    }
}



// Write SSID & password into NVS
static esp_err_t save_wifi_credentials(const char* ssid, const char* pass) {
    nvs_handle_t h;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err  = nvs_set_str(h, "ssid", ssid);
    err |= nvs_set_str(h, "pass", pass);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

// Load SSID & password from NVS; returns ESP_ERR_NVS_NOT_FOUND if empty
static esp_err_t load_wifi_credentials(char* ssid, size_t ssid_len, char* pass, size_t pass_len) {
    nvs_handle_t h;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err  = nvs_get_str(h, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(h, "pass", pass, &pass_len);
    nvs_close(h);
    return err;
}

// ======== HTTP Server Handlers ========
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *form =
        "<html><body>"
        "<h3>Configure Wi-Fi</h3>"
        "<form action='/' method='post'>"
        "SSID: <input name='ssid'/><br/>"
        "Password: <input type='password' name='password'/><br/>"
        "<input type='submit' value='Submit'/></form>"
        "</body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, form);
    return ESP_OK;
}


static esp_err_t root_post_handler(httpd_req_t *req)
{
    char buf[128];
    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (len <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[len] = '\0';
    sscanf(buf, "ssid=%31[^&]&password=%63s", user_ssid, user_pass);
    ESP_LOGI(TAG, "Received credentials: SSID='%s', PASS='%s'", user_ssid, user_pass);
    save_wifi_credentials(user_ssid, user_pass);

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Connecting to network...");
    vTaskDelay(pdMS_TO_TICKS(100));

    // Launch connect task
    xTaskCreatePinnedToCore(connect_task, "connect_task",
                            8192, NULL, 5, NULL, tskNO_AFFINITY);
    return ESP_OK;
}


static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &get);
        httpd_uri_t post = {
            .uri = "/",
            .method = HTTP_POST,
            .handler = root_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &post);
    }
    return server;
}


esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR: ESP_LOGE("HTTP_EVENT", "HTTP_EVENT_ERROR"); break;
        case HTTP_EVENT_ON_CONNECTED: ESP_LOGI("HTTP_EVENT", "Connected"); break;
        default: break;
    }
    return ESP_OK;
}

void query_overpass_api(double lat, double lon, const char *query_term)
{
    ESP_LOGI(TAG, "Connected to Wi-Fi! Querying Overpass...");

    const char *overpass_url = "https://overpass-api.de/api/interpreter";

    char post_data[512];
    snprintf(post_data, sizeof(post_data),
        "[out:json];node[\"amenity\"=\"%s\"](around:500,%f,%f);out;",
        query_term, lat, lon);

    esp_http_client_config_t config = {
    .url = overpass_url,
    .method = HTTP_METHOD_POST,
    .cert_pem = (const char *)_binary_isrg_root_x1_pem_start,
    .timeout_ms = 10000,
    .event_handler = _http_event_handler,
    .buffer_size = 2048,
    .buffer_size_tx = 2048,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept-Encoding", "identity");

    esp_err_t err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_http_client_open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    int written = esp_http_client_write(client, post_data, strlen(post_data));
    if (written <= 0) {
        ESP_LOGE(TAG, "Failed to write request body");
        esp_http_client_cleanup(client);
        return;
    }

    // 🟢 Important: finalize the request so we can read status & response
    err = esp_http_client_fetch_headers(client);
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to fetch headers: %s", esp_err_to_name(err));
    }

    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP status = %d", status_code);

    char *buffer = malloc(MAX_RESPONSE_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Out of memory");
        esp_http_client_cleanup(client);
        return;
    }

    int total_read = 0, read_len;
    do {
        read_len = esp_http_client_read(client, buffer + total_read, MAX_RESPONSE_SIZE - total_read - 1);
        if (read_len > 0) {
            total_read += read_len;
        }
    } while (read_len > 0 && total_read < MAX_RESPONSE_SIZE - 1);

    if (total_read >= MAX_RESPONSE_SIZE - 1) {
        ESP_LOGW(TAG, "Response was likely truncated (hit buffer limit)");
    }

    buffer[total_read] = '\0';

    ESP_LOGI(TAG, "Received body (%d bytes)", total_read);
    buffer[total_read] = '\0';  // Null-terminate

    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        free(buffer);
        esp_http_client_cleanup(client);
        return;
    }

    cJSON *elements = cJSON_GetObjectItem(root, "elements");
    if (!cJSON_IsArray(elements)) {
        ESP_LOGW(TAG, "No 'elements' array found in JSON");
        cJSON_Delete(root);
        free(buffer);
        esp_http_client_cleanup(client);
        return;
    }

    int count = cJSON_GetArraySize(elements);
    int limit = count < 3 ? count : 3;


    int coord_count = 0;
    for (int i = 0; i < limit; i++) {
        cJSON *item = cJSON_GetArrayItem(elements, i);
        cJSON *lat = cJSON_GetObjectItem(item, "lat");
        cJSON *lon = cJSON_GetObjectItem(item, "lon");
    
        if (cJSON_IsNumber(lat) && cJSON_IsNumber(lon)) {
            snprintf(coord_list[coord_count], sizeof(coord_list[coord_count]),
                     "%.6f,%.6f", lat->valuedouble, lon->valuedouble);
            coord_count++;
        }
    }
    coord_index = 0; 


    cJSON_Delete(root);
    free(buffer);

}


void i2c_listener_task(void *arg) {
    uint8_t buf[64];  // For coordinate input or NEXT command
    int len;

    ESP_LOGW("I2C", "Beginning Listening");

    while (1) {
        len = i2c_slave_read_buffer(I2C_SLAVE_NUM, buf, sizeof(buf) - 1, pdMS_TO_TICKS(1000));
        if (len > 0) {
            buf[len] = '\0';
            ESP_LOGI("I2C", "Received from master: %s", (char*)buf);

            if (strncmp((char*)buf, "NEXT", 4) == 0) {
                if (coord_index < coord_count) {
                    const char *resp = coord_list[coord_index++];
                    i2c_slave_write_buffer(I2C_SLAVE_NUM,
                                           (const uint8_t*)resp, strlen(resp),
                                           pdMS_TO_TICKS(100));
                    ESP_LOGI("I2C", "Sent to master: %s", resp);
                } else {
                    const char *done = "DONE";
                    i2c_slave_write_buffer(I2C_SLAVE_NUM,
                                           (const uint8_t*)done, strlen(done),
                                           pdMS_TO_TICKS(100));
                }
            } else {
                
                double lat = 0.0, lon = 0.0;
                if (sscanf((char*)buf, "%lf,%lf", &lat, &lon) == 2) {
                    ESP_LOGI("I2C", "Parsed coords: lat=%.6f, lon=%.6f", lat, lon);
                    query_overpass_api(lat, lon, "cafe");  // Hardcoded query term
                    const char *ack = "OK";
                    i2c_slave_write_buffer(I2C_SLAVE_NUM,
                                           (const uint8_t*)ack, strlen(ack),
                                           pdMS_TO_TICKS(100));
                } else {
                    const char *err = "ERR";
                    i2c_slave_write_buffer(I2C_SLAVE_NUM,
                                           (const uint8_t*)err, strlen(err),
                                           pdMS_TO_TICKS(100));
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


// ======== Task for Station Connection & I2C Setup ========
static void connect_task(void *pv)
{
    // Stop AP mode
    esp_wifi_stop();
    ESP_LOGI(TAG, "AP stopped, switching to STA mode");

    // Initialize default station
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_netif_create_default_wifi_sta();
    esp_wifi_init(&init_cfg);

    // Register event handlers for station
    esp_event_handler_instance_t inst1, inst2;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        wifi_event_handler, NULL, &inst1);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        wifi_event_handler, NULL, &inst2);

    // Configure station credentials
    wifi_config_t sta_cfg = {
        .sta = {
            .ssid = "",
            .password = "",
            .bssid_set = false
        }
    };
    strncpy((char*)sta_cfg.sta.ssid, user_ssid, sizeof(sta_cfg.sta.ssid) - 1);
    strncpy((char*)sta_cfg.sta.password, user_pass, sizeof(sta_cfg.sta.password) - 1);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);
    ESP_LOGI(TAG, "Connecting with SSID: '%s', PASS: '%s'", sta_cfg.sta.ssid, sta_cfg.sta.password);

    esp_wifi_start();
    ESP_LOGI(TAG, "Station started, connecting to '%s'...", user_ssid);

    // Wait for connection
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                        pdFALSE, pdTRUE, portMAX_DELAY);
    
    //query_overpass_api(34.0522, -118.2437, "restaurant");

    // I2C slave setup
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave = {
            .slave_addr = I2C_SLAVE_ADDR,
            .addr_10bit_en = 0
        }
    };
    i2c_param_config(I2C_SLAVE_NUM, &i2c_cfg);
    i2c_driver_install(I2C_SLAVE_NUM, I2C_MODE_SLAVE,
                       I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);

    vTaskDelete(NULL);
}

static void AP_config(void) {
    ESP_LOGI(TAG, "Starting AP for configuration");
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&init_cfg);
    wifi_config_t ap_cfg = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_OPEN
        }
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    esp_wifi_start();
    ESP_LOGI(TAG, "Access Point '%s' started", AP_SSID);

    start_webserver();
}


void app_main(void)
{
    // NVS init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // TCP/IP & event loop
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_event_group = xEventGroupCreate();

    // Try loading saved credentials
    if(load_wifi_credentials(user_ssid, sizeof(user_ssid),
                              user_pass, sizeof(user_pass)) == ESP_OK &&
        strlen(user_ssid) > 0) 
    {
        ESP_LOGI(TAG, "Loaded saved SSID: %s", user_ssid);
        // kick off STA connect task directly
        xTaskCreate(connect_task, "connect_task", 4096, NULL, 5, NULL);
        // wait a bounded time for WIFI_CONNECTED_BIT...
        EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                    WIFI_CONNECTED_BIT, pdFALSE, pdTRUE,
                    pdMS_TO_TICKS(30000));  // wait up to 30s
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Reconnected via saved credentials");
            return;  // your connect_task also sets up I2C TX buffer
        }
        // Clear invalid creds
        ESP_LOGW(TAG, "Reconnection failed, clearing saved creds");
        nvs_handle_t h;
        nvs_open("storage", NVS_READWRITE, &h); 
        nvs_erase_key(h,"ssid");
        nvs_erase_key(h,"pass");
        nvs_commit(h);
        nvs_close(h);
    }

    // Fallback to AP mode for configuration
    AP_config();
}
