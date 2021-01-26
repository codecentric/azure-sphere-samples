#ifndef CONFIG_H_
#define CONFIG_H_

#define GROVE_SHIELD_BAUDRATE (230400) // max baud rate: 230400

#define ANALOG_READ_CHANNEL (0)

#define DISPLAY_THREAD_MIN_UPDATE_INTERVAL_MS (1000) // minimum time between display updates
#define SENSORS_THREAD_UPDATE_INTERVAL_MS (200)      // time between sensor updates
#define BUTTON_THREAD_UPDATE_INTERVAL_MS (10)        // time intervall in which the buttns are queried for state changes

#define UPDATE_RELAY_BY_TEMPERATURE_THRESHOLD_MIN (15.0f) // set this to the minimum of the configurable temperature threshold
#define UPDATE_RELAY_BY_TEMPERATURE_THRESHOLD_MAX (25.0f) // set this to the maximum of the configurable temperature threshold

#endif //CONFIG_H_
