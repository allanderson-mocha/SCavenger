// === Full working ESP32 code: AP -> STA -> User Coordinates -> Real Cafe API Query ===

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
#include "esp_http_client.h"
#include "cJSON.h"

#define AP_SSID "SCavenger"
#define AP_PASS ""
#define MAX_STA_CONN 4
#define WIFI_CONNECTED_BIT BIT0
#define MAX_RESPONSE_SIZE 16384

extern const uint8_t _binary_isrg_root_x1_pem_start[] asm("_binary_isrg_root_x1_pem_start");

static const char *TAG = "esp_setup";
static EventGroupHandle_t wifi_event_group;
static httpd_handle_t server = NULL;
static char user_ssid[32] = {0};
static char user_pass[64] = {0};

static char cafes_html[2048] = {0};

// Forward declarations
static void start_wifi_ap(void);
static void connect_to_wifi(const char *ssid, const char *pass);
static void query_overpass_api(double lat, double lon);
static esp_err_t coords_get_handler(httpd_req_t *req);
static esp_err_t coords_post_handler(httpd_req_t *req);

// --- Wi-Fi Event Handler ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    static int retries = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retries < 6) {
            retries++;
            ESP_LOGW(TAG, "Wi-Fi disconnected, retrying... attempt %d/6", retries);
            esp_wifi_connect();
        } else {
            ESP_LOGW(TAG, "Failed to connect after retries, going back to AP mode...");
            retries = 0;
            start_wifi_ap();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        retries = 0;
        ESP_LOGI(TAG, "Got IP! Switching web page...");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

        if (server) {
            httpd_unregister_uri_handler(server, "/", HTTP_GET);
            httpd_uri_t coords_get = {.uri = "/", .method = HTTP_GET, .handler = coords_get_handler, .user_ctx = NULL};
            httpd_register_uri_handler(server, &coords_get);

            httpd_uri_t coords_post = {.uri = "/coords", .method = HTTP_POST, .handler = coords_post_handler, .user_ctx = NULL};
            httpd_register_uri_handler(server, &coords_post);
        }
    }
}

// --- Web Handlers ---
static esp_err_t wifi_get_handler(httpd_req_t *req)
{
    const char *page = "<html><body><h3>Enter Wi-Fi Credentials</h3>"
                        "<form method='post'>"
                        "SSID: <input name='ssid'><br>"
                        "Password: <input name='password' type='password'><br>"
                        "<input type='submit' value='Connect'></form></body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, page);
    return ESP_OK;
}

static esp_err_t wifi_post_handler(httpd_req_t *req)
{
    char buf[128];
    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (len <= 0) return ESP_FAIL;
    buf[len] = '\0';

    sscanf(buf, "ssid=%31[^&]&password=%63s", user_ssid, user_pass);
    ESP_LOGI(TAG, "Received SSID: %s", user_ssid);

    connect_to_wifi(user_ssid, user_pass);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, "Connecting to Wi-Fi...");
    return ESP_OK;
}

static esp_err_t coords_get_handler(httpd_req_t *req)
{
    const char *page = "<html><body><h3>Enter GPS Coordinates</h3>"
                        "<form action='/coords' method='post'>"
                        "Latitude: <input name='lat'><br>"
                        "Longitude: <input name='lon'><br>"
                        "<input type='submit' value='Search'></form></body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, page);
    return ESP_OK;
}

static esp_err_t coords_post_handler(httpd_req_t *req)
{
    char buf[128];
    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (len <= 0) return ESP_FAIL;
    buf[len] = '\0';

    double lat = 0, lon = 0;
    sscanf(buf, "lat=%lf&lon=%lf", &lat, &lon);
    ESP_LOGI(TAG, "Received coords: %.6f, %.6f", lat, lon);

    query_overpass_api(lat, lon);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, cafes_html);
    return ESP_OK;
}

// --- Overpass API Query ---
static void query_overpass_api(double lat, double lon)
{
    const char *url = "https://overpass-api.de/api/interpreter";

    char post_data[512];
    snprintf(post_data, sizeof(post_data),
            "[out:json];node(around:2000,%.6f,%.6f)[\"amenity\"=\"cafe\"];out;",
        lat, lon);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = (const char *)_binary_isrg_root_x1_pem_start,
        .timeout_ms = 10000,
        .buffer_size = 8192,
        .buffer_size_tx = 8192,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
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
    
    int total_read = esp_http_client_read_response(client, buffer, MAX_RESPONSE_SIZE - 1);
    buffer[total_read] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        snprintf(cafes_html, sizeof(cafes_html), "<html><body><h3>API Error</h3></body></html>");
        goto cleanup;
    }

    cJSON *elements = cJSON_GetObjectItem(root, "elements");
    if (!cJSON_IsArray(elements)) {
        snprintf(cafes_html, sizeof(cafes_html), "<html><body><h3>No Cafes Found</h3></body></html>");
        goto cleanup;
    }

    snprintf(cafes_html, sizeof(cafes_html), "<html><body>");
    int count = cJSON_GetArraySize(elements);
    int limit = count < 5 ? count : 5;

    ESP_LOGI(TAG, "Found %d cafes", count);


    for (int i = 0; i < limit; i++) {
        cJSON *item = cJSON_GetArrayItem(elements, i);
        cJSON *name = cJSON_GetObjectItem(item, "tags");
        const char *cafe_name = "Unknown Cafe";

        if (name) {
            cJSON *n = cJSON_GetObjectItem(name, "name");
            if (cJSON_IsString(n)) {
                cafe_name = n->valuestring;
            }
        }

        cJSON *lat_item = cJSON_GetObjectItem(item, "lat");
        cJSON *lon_item = cJSON_GetObjectItem(item, "lon");

        if (lat_item && lon_item) {
            char line[256];
            snprintf(line, sizeof(line),
                    "<p>%.6f, %.6f</p>",
                    lat_item->valuedouble, lon_item->valuedouble);

            strlcat(cafes_html, line, sizeof(cafes_html));
        }
    }

    strlcat(cafes_html, "</body></html>", sizeof(cafes_html));

cleanup:
    cJSON_Delete(root);
    free(buffer);
    esp_http_client_cleanup(client);
}

// --- Wi-Fi Setup ---
static void start_wifi_ap(void)
{
    ESP_LOGI(TAG, "Starting AP mode");

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
}

static void connect_to_wifi(const char *ssid, const char *pass)
{
    ESP_LOGI(TAG, "Connecting to Wi-Fi SSID: %s", ssid);

    wifi_config_t wifi_cfg = {0};
    strncpy((char*)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid) - 1);
    strncpy((char*)wifi_cfg.sta.password, pass, sizeof(wifi_cfg.sta.password) - 1);

    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    esp_wifi_start();
}

// --- Server Startup ---
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t serv = NULL;

    if (httpd_start(&serv, &config) == ESP_OK) {
        httpd_uri_t root_get = {.uri = "/", .method = HTTP_GET, .handler = wifi_get_handler, .user_ctx = NULL};
        httpd_register_uri_handler(serv, &root_get);

        httpd_uri_t wifi_post = {.uri = "/", .method = HTTP_POST, .handler = wifi_post_handler, .user_ctx = NULL};
        httpd_register_uri_handler(serv, &wifi_post);
    }
    return serv;
}

// --- App Main ---
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_event_group = xEventGroupCreate();

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    start_wifi_ap();
    server = start_webserver();
}
