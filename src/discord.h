#ifndef R_DISCORD_H
#define R_DISCORD_H

#include "discord_game_sdk.h"


struct Application {
    struct IDiscordCore* core;
};

extern struct Application app;

void Discord_init();

bool Discord_connected();

void Discord_update();

#endif