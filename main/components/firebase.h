#ifndef FIREBASE_H
#define FIREBASE_H
#include <stdbool.h>
/* Initialize Firebase client and reset database values */
void firebase_init(void);

/* Poll Firebase for commands (relay control, login/logout) */
void firebase_read_streams(void);

/* Write a float value to Firebase (path: e.g., "dev1/I") */
void firebase_set_float(const char *path, float value);

/* Write a boolean value to Firebase */
void firebase_set_bool(const char *path, bool value);

/* Read a float value from Firebase, returns true on success */
bool firebase_get_float(const char *path, float *output);

/* Batch update using JSON PATCH (path: "" for root) */
void firebase_patch_json(const char *path, const char *json);

/* Upload all device data (dev1/dev2) */
void firebase_update_data(void);

/* Save user energy consumption (uid, device1 energy, device2 energy) */
void firebase_store_user_energy(const char *uid, float e1, float e2);

#endif