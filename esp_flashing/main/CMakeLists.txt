idf_component_register(
    SRCS 
       "esp_test_main.c"
       
    PRIV_REQUIRES
        spi_flash
        nvs_flash
        esp_netif
        esp_wifi
        esp_http_server
        driver
        esp_http_client
        json
    INCLUDE_DIRS
        "."
    EMBED_TXTFILES
        "isrg_root_x1.pem"
)
