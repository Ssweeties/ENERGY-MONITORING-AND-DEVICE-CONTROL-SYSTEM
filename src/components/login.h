#ifndef LOGIN_H
#define LOGIN_H

#include <stdbool.h>
void login_handle(void);
bool check_valid_uid(const char *uid);
bool login_load_user_energy(const char *uid, float *e1, float *e2);

#endif