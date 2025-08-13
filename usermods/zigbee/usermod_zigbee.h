#include "wled.h"

/*
 * Zigbee Usermod for WLED
 * 
 * This usermod enables WLED control via Zigbee protocol using ESP-Zigbee-SDK.
 * Compatible with ESP32-C6 which has native IEEE 802.15.4 radio support.
 * 
 * Author: WLED Community
 * 
 * Features:
 * - Zigbee 3.0 device support
 * - On/Off light control
 * - Brightness control (0-255)
 * - Color control (RGB)
 * - WLED integration for effects and settings
 */

#ifdef ESP32
#if defined(CONFIG_IDF_TARGET_ESP32C6) && defined(CONFIG_ZB_ENABLED)

// Include ESP-Zigbee-SDK headers (available in ESP-IDF 5.x for ESP32-C6)
#include "esp_zigbee_core.h"
#include "esp_zigbee_cluster.h"
#include "esp_zigbee_attribute.h"

// Zigbee definitions
#define ZIGBEE_ENDPOINT_ID 1
#define ZIGBEE_DEVICE_ID ESP_ZB_HA_DIMMABLE_LIGHT_DEVICE_ID
#define ZIGBEE_DEVICE_VERSION 1
#define ZIGBEE_POWER_SOURCE ESP_ZB_ZED_POWER_SOURCE_BATTERY

class ZigbeeUsermod : public Usermod {
private:
    bool enabled = false;
    bool initDone = false;
    bool zigbeeStarted = false;
    
    // Configuration variables
    String deviceName = "WLED";
    uint8_t endpointId = ZIGBEE_ENDPOINT_ID;
    
    // Zigbee state variables
    bool zigbeeOnOff = false;
    uint8_t zigbeeBrightness = 0;
    uint16_t zigbeeColorX = 0;
    uint16_t zigbeeColorY = 0;
    
    // String constants for config
    static const char _name[];
    static const char _enabled[];
    static const char _deviceName[];
    static const char _endpointId[];
    
    // Zigbee callback handlers
    static void zigbeeActionHandler(esp_zb_core_action_t action, esp_zb_core_action_handler_args_t *args);
    static esp_err_t zigbeeAttributeHandler(const esp_zb_zcl_set_attr_value_message_t *message);
    
    // Helper functions
    void updateWLEDFromZigbee();
    void updateZigbeeFromWLED();
    esp_err_t createZigbeeDevice();
    
public:
    void setup() override;
    void loop() override;
    void onStateChange(uint8_t mode) override;
    
    // Config functions
    void addToConfig(JsonObject& root) override;
    bool readFromConfig(JsonObject& root) override;
    void addToJsonInfo(JsonObject& root) override;
    
    // Usermod info
    uint16_t getId() override { return USERMOD_ID_ZIGBEE; }
    
    // Control functions
    inline void enable(bool enable) { enabled = enable; }
    inline bool isEnabled() { return enabled; }
};

// Static member definitions
const char ZigbeeUsermod::_name[] PROGMEM = "Zigbee";
const char ZigbeeUsermod::_enabled[] PROGMEM = "enabled";
const char ZigbeeUsermod::_deviceName[] PROGMEM = "deviceName";
const char ZigbeeUsermod::_endpointId[] PROGMEM = "endpointId";

// Global instance for callback access
ZigbeeUsermod* zigbeeInstance = nullptr;

void ZigbeeUsermod::setup() {
    Serial.println(F("Zigbee Usermod: Starting setup"));
    
    if (!enabled) {
        Serial.println(F("Zigbee Usermod: Disabled"));
        return;
    }
    
    // Store global instance for callbacks
    zigbeeInstance = this;
    
    Serial.println(F("Zigbee Usermod: ESP32-C6 with Zigbee support detected"));
    
    // Initialize ESP-Zigbee-SDK
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    
    esp_err_t err = esp_zb_platform_config(&config);
    if (err != ESP_OK) {
        Serial.printf("Zigbee Usermod: Platform config failed: %s\n", esp_err_to_name(err));
        return;
    }
    
    // Create Zigbee device
    if (createZigbeeDevice() == ESP_OK) {
        // Set attribute change callback
        esp_zb_core_action_handler_register(zigbeeActionHandler);
        
        // Start Zigbee stack
        err = esp_zb_start(false);
        if (err == ESP_OK) {
            zigbeeStarted = true;
            Serial.println(F("Zigbee Usermod: Initialized successfully"));
        } else {
            Serial.printf("Zigbee Usermod: Start failed: %s\n", esp_err_to_name(err));
        }
    } else {
        Serial.println(F("Zigbee Usermod: Failed to create device"));
    }
    
    initDone = true;
}

void ZigbeeUsermod::loop() {
    if (!enabled || !initDone || !zigbeeStarted) return;
    
    // Periodically sync WLED state to Zigbee attributes
    static unsigned long lastSync = 0;
    if (millis() - lastSync > 1000) { // Sync every second
        updateZigbeeFromWLED();
        lastSync = millis();
    }
}

void ZigbeeUsermod::onStateChange(uint8_t mode) {
    if (!enabled || !initDone || !zigbeeStarted) return;
    
    // Update Zigbee attributes when WLED state changes
    updateZigbeeFromWLED();
}

esp_err_t ZigbeeUsermod::createZigbeeDevice() {
    // Create endpoint list
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    if (!ep_list) {
        Serial.println(F("Zigbee: Failed to create endpoint list"));
        return ESP_FAIL;
    }
    
    // Create cluster list for the endpoint
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
    if (!cluster_list) {
        Serial.println(F("Zigbee: Failed to create cluster list"));
        return ESP_FAIL;
    }
    
    // Basic cluster (mandatory)
    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(NULL);
    if (basic_cluster) {
        esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    }
    
    // On/Off cluster
    esp_zb_attribute_list_t *on_off_cluster = esp_zb_on_off_cluster_create(NULL);
    if (on_off_cluster) {
        esp_zb_cluster_list_add_on_off_cluster(cluster_list, on_off_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    }
    
    // Level Control cluster
    esp_zb_attribute_list_t *level_cluster = esp_zb_level_cluster_create(NULL);
    if (level_cluster) {
        esp_zb_cluster_list_add_level_cluster(cluster_list, level_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    }
    
    // Color Control cluster
    esp_zb_attribute_list_t *color_cluster = esp_zb_color_control_cluster_create(NULL);
    if (color_cluster) {
        esp_zb_cluster_list_add_color_control_cluster(cluster_list, color_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    }
    
    // Create endpoint with clusters
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = endpointId,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ZIGBEE_DEVICE_ID,
        .app_device_version = ZIGBEE_DEVICE_VERSION
    };
    
    esp_err_t err = esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint_config);
    if (err != ESP_OK) {
        Serial.printf("Zigbee: Failed to add endpoint: %s\n", esp_err_to_name(err));
        return err;
    }
    
    err = esp_zb_device_register(ep_list);
    if (err != ESP_OK) {
        Serial.printf("Zigbee: Failed to register device: %s\n", esp_err_to_name(err));
        return err;
    }
    
    return ESP_OK;
}

void ZigbeeUsermod::zigbeeActionHandler(esp_zb_core_action_t action, esp_zb_core_action_handler_args_t *args) {
    if (!zigbeeInstance) return;
    
    switch (action) {
        case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
            zigbeeAttributeHandler(&(args->set_attr_value));
            break;
        case ESP_ZB_CORE_CMD_DEFAULT_RESP_CB_ID:
            Serial.println(F("Zigbee: Command response received"));
            break;
        default:
            break;
    }
}

esp_err_t ZigbeeUsermod::zigbeeAttributeHandler(const esp_zb_zcl_set_attr_value_message_t *message) {
    if (!zigbeeInstance) return ESP_ERR_INVALID_STATE;
    
    Serial.printf("Zigbee: Attribute change - Endpoint: %d, Cluster: 0x%04x, Attribute: 0x%04x\n", 
                  message->info.dst_endpoint, message->info.cluster, message->attribute.id);
    
    switch (message->info.cluster) {
        case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
                zigbeeInstance->zigbeeOnOff = *(bool*)message->attribute.data.value;
                Serial.printf("Zigbee: On/Off = %s\n", zigbeeInstance->zigbeeOnOff ? "ON" : "OFF");
                zigbeeInstance->updateWLEDFromZigbee();
            }
            break;
            
        case ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID) {
                zigbeeInstance->zigbeeBrightness = *(uint8_t*)message->attribute.data.value;
                Serial.printf("Zigbee: Brightness = %d\n", zigbeeInstance->zigbeeBrightness);
                zigbeeInstance->updateWLEDFromZigbee();
            }
            break;
            
        case ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_X_ID) {
                zigbeeInstance->zigbeeColorX = *(uint16_t*)message->attribute.data.value;
                Serial.printf("Zigbee: Color X = %d\n", zigbeeInstance->zigbeeColorX);
                zigbeeInstance->updateWLEDFromZigbee();
            } else if (message->attribute.id == ESP_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_Y_ID) {
                zigbeeInstance->zigbeeColorY = *(uint16_t*)message->attribute.data.value;
                Serial.printf("Zigbee: Color Y = %d\n", zigbeeInstance->zigbeeColorY);
                zigbeeInstance->updateWLEDFromZigbee();
            }
            break;
    }
    
    return ESP_OK;
}

void ZigbeeUsermod::updateWLEDFromZigbee() {
    // Update WLED state based on Zigbee attributes
    if (zigbeeOnOff) {
        bri = zigbeeBrightness;
        if (bri == 0) bri = 255; // Ensure some brightness when turning on
    } else {
        bri = 0;
    }
    
    // Convert CIE XY to RGB (simplified conversion)
    if (zigbeeColorX > 0 || zigbeeColorY > 0) {
        // This is a simplified conversion - a proper implementation would use CIE XY to RGB conversion
        float x = zigbeeColorX / 65535.0f;
        float y = zigbeeColorY / 65535.0f;
        
        // Basic approximation for demo purposes
        col[0] = (uint8_t)(x * 255);
        col[1] = (uint8_t)(y * 255);
        col[2] = (uint8_t)((1.0f - x - y) * 255);
    }
    
    // Apply changes to strip
    stateUpdated(CALL_MODE_NOTIFICATION);
}

void ZigbeeUsermod::updateZigbeeFromWLED() {
    if (!zigbeeStarted) return;
    
    // Update Zigbee attributes based on WLED state
    bool currentOnOff = (bri > 0);
    uint8_t currentBrightness = bri;
    
    if (currentOnOff != zigbeeOnOff) {
        zigbeeOnOff = currentOnOff;
        // Update Zigbee On/Off attribute
        esp_zb_zcl_set_attribute_val(endpointId, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
                                     ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, 
                                     &zigbeeOnOff, false);
    }
    
    if (currentBrightness != zigbeeBrightness) {
        zigbeeBrightness = currentBrightness;
        // Update Zigbee Level attribute
        esp_zb_zcl_set_attribute_val(endpointId, ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, 
                                     ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID, 
                                     &zigbeeBrightness, false);
    }
}

void ZigbeeUsermod::addToConfig(JsonObject& root) {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enabled)] = enabled;
    top[FPSTR(_deviceName)] = deviceName;
    top[FPSTR(_endpointId)] = endpointId;
}

bool ZigbeeUsermod::readFromConfig(JsonObject& root) {
    JsonObject top = root[FPSTR(_name)];
    bool configComplete = !top.isNull();
    
    configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled, false);
    configComplete &= getJsonValue(top[FPSTR(_deviceName)], deviceName, "WLED");
    configComplete &= getJsonValue(top[FPSTR(_endpointId)], endpointId, (uint8_t)ZIGBEE_ENDPOINT_ID);
    
    return configComplete;
}

void ZigbeeUsermod::addToJsonInfo(JsonObject& root) {
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");
    
    JsonArray zigbeeInfo = user.createNestedArray(FPSTR(_name));
    if (enabled && zigbeeStarted) {
        zigbeeInfo.add("Connected");
        zigbeeInfo.add("");
    } else if (enabled) {
        zigbeeInfo.add("Initializing");
        zigbeeInfo.add("");
    } else {
        zigbeeInfo.add("Disabled");
        zigbeeInfo.add("");
    }
}

// Instantiate the usermod
ZigbeeUsermod zigbeeUsermod;

// Register the usermod
REGISTER_USERMOD(zigbeeUsermod);

#else // Not ESP32-C6 or Zigbee not enabled

// Stub implementation for non-ESP32-C6 platforms
class ZigbeeUsermod : public Usermod {
private:
    static const char _name[];
    
public:
    void setup() override {
        Serial.println(F("Zigbee Usermod: Only supported on ESP32-C6 with Zigbee enabled"));
    }
    
    void addToJsonInfo(JsonObject& root) override {
        JsonObject user = root["u"];
        if (user.isNull()) user = root.createNestedObject("u");
        
        JsonArray zigbeeInfo = user.createNestedArray(FPSTR(_name));
        zigbeeInfo.add("Not supported");
        zigbeeInfo.add("ESP32-C6 required");
    }
    
    uint16_t getId() override { return USERMOD_ID_ZIGBEE; }
};

const char ZigbeeUsermod::_name[] PROGMEM = "Zigbee";

ZigbeeUsermod zigbeeUsermod;

// Register the usermod  
REGISTER_USERMOD(zigbeeUsermod);

#endif // CONFIG_IDF_TARGET_ESP32C6 && CONFIG_ZB_ENABLED
#endif // ESP32