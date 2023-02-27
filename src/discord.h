#ifndef R_DISCORD_H
#define R_DISCORD_H

#include "discord_game_sdk.h"

#define Discord_ActivityManager app.core->get_activity_manager(app.core)

struct Application {
    struct IDiscordCore* core;
};

extern struct Application app;
extern struct IDiscordActivityEvents Discord_activityEvents;

void Discord_init();

bool Discord_connected();

void Discord_update();

void DISCORD_CALLBACK Discord_onActivityJoin(void* event_data, const char* secret);
void DISCORD_CALLBACK Discord_onActivitySpectate(void* event_data, const char* secret);
void DISCORD_CALLBACK Discord_onActivityJoinRequest(void* event_data, struct DiscordUser* user);
void DISCORD_CALLBACK Discord_onActivityInvite(void* event_data, enum EDiscordActivityActionType type, struct DiscordUser* user, struct DiscordActivity* activity);

#endif