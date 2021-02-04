#pragma once

#include <stdbool.h>

#define VIBRATED 1
#define COUNTDOWN_ELAPSED 2
#define MAX_MOTOR_SPEED_REACHED 3
#define TEMPERATURE_CHANGED 4
#define HUMIDITY_CHANGED 5
#define LOADING_INDICATOR_SHOWN 6
#define COFFEE_REQUEST_SHOWN 7

typedef struct
{
    bool countdownEnabled;
    bool coffeeRequestEnabled;
    bool loadingIndicatorEnabled;
    bool vibrationEnabled;
    bool motorsEnabled;
    float temperature;
    float humidity;
} state_t;

typedef struct {
    state_t *state;
} store_t;

#define INITIAL_COUNTDOWN 10
#define UNKNOWN_VALUE -1

store_t* CreateStore(state_t *initialState);
void DestroyStore(store_t *store);

void Dispatch(store_t* store, int action, void *payload);

bool CountdownEnabled(store_t *store);
bool CoffeeRequestEnabled(store_t *store);
bool LoadingIndicatorEnabled(store_t *store);
bool VibrationEnabled(store_t *store);
bool MotorsEnabled(store_t *store);
float Temperature(store_t *store);
float Humidity(store_t *store);