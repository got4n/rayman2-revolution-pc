#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const char *revo_version(void);

void revo_update_start(void);

#ifdef __cplusplus
}

void revo_update_menu(void);

bool revo_update_badge_wanted(void);
void revo_update_badge(void);
#endif
