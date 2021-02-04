#include "state.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define STORE_SIZE sizeof(store_t)
#define STATE_SIZE sizeof(state_t)

pthread_mutex_t mutex;

state_t *ReduceState(state_t *oldState, int action, void *payload)
{
    state_t *newState = (state_t *)malloc(STATE_SIZE);
    memcpy(newState, oldState, STATE_SIZE);

    if (action == COUNTDOWN_ELAPSED)
    {
        newState->countdownEnabled = false;
        newState->vibrationEnabled = true;
        newState->motorsEnabled = true;
        newState->loadingIndicatorEnabled = true;
    }
    else if (action == VIBRATED)
    {
        newState->vibrationEnabled = false;
    }
    else if (action == LOADING_INDICATOR_SHOWN)
    {
        newState->loadingIndicatorEnabled = false;
        newState->coffeeRequestEnabled = true;
    }
    else if (action == COFFEE_REQUEST_SHOWN)
    {
        newState->coffeeRequestEnabled = false;
        newState->countdownEnabled = true;
    }
    else if (action == MAX_MOTOR_SPEED_REACHED)
    {
        newState->motorsEnabled = false;
    }
    else if (action == TEMPERATURE_CHANGED)
    {
        newState->temperature = *((float *)payload);
    }
    else if (action == HUMIDITY_CHANGED)
    {
        newState->humidity = *((float *)payload);
    }

    return newState;
}

store_t *CreateStore(state_t *initialState)
{
    store_t *store = malloc(STORE_SIZE);
    store->state = malloc(STATE_SIZE);
    memcpy(store->state, initialState, STATE_SIZE);

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        return NULL;
    }

    return store;
}

void DestroyStore(store_t *store) {
    pthread_mutex_destroy(&mutex);
    free(store->state);
    free(store);
}

void Dispatch(store_t *store, int action, void *payload)
{
    pthread_mutex_lock(&mutex);
    state_t *oldState = store->state;
    store->state = ReduceState(oldState, action, payload);
    free(oldState);
    pthread_mutex_unlock(&mutex);
}

bool CountdownEnabled(store_t *store) {
    pthread_mutex_lock(&mutex);
    bool countdownEnabled = store->state->countdownEnabled;
    pthread_mutex_unlock(&mutex);
    return countdownEnabled;
}

bool CoffeeRequestEnabled(store_t *store) {
    pthread_mutex_lock(&mutex);
    bool coffeeRequestEnabled = store->state->coffeeRequestEnabled;
    pthread_mutex_unlock(&mutex);
    return coffeeRequestEnabled;
}

bool LoadingIndicatorEnabled(store_t *store) {
    pthread_mutex_lock(&mutex);
    bool loadingIndicatorEnabled = store->state->loadingIndicatorEnabled;
    pthread_mutex_unlock(&mutex);
    return loadingIndicatorEnabled;
}

bool VibrationEnabled(store_t *store) {
    pthread_mutex_lock(&mutex);
    bool vibrationEnabled = store->state->vibrationEnabled;
    pthread_mutex_unlock(&mutex);
    return vibrationEnabled;
}

bool MotorsEnabled(store_t *store) {
    pthread_mutex_lock(&mutex);
    bool motorsEnabled = store->state->motorsEnabled;
    pthread_mutex_unlock(&mutex);
    return motorsEnabled;
}

float Temperature(store_t *store) {
    pthread_mutex_lock(&mutex);
    bool temperature = store->state->temperature;
    pthread_mutex_unlock(&mutex);
    return temperature;
}

float Humidity(store_t *store) {
    pthread_mutex_lock(&mutex);
    bool humidity = store->state->humidity;
    pthread_mutex_unlock(&mutex);
    return humidity;
}
