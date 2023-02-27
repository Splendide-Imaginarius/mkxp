#include "discord.h"
#include "util/debugwriter.h"
#include "sharedstate.h"
#include <cstdio>

struct Application app;
bool _discord_connected;

void Discord_init() {
    // Don't forget to memset or otherwise initialize your classes!
    memset(&app, 0, sizeof(Application));

    Debug() << "help";

    struct DiscordCreateParams params;
    params.client_id = DISCORD_APPID;
    params.flags = DiscordCreateFlags_NoRequireDiscord;
    params.event_data = &app;

    EDiscordResult result = DiscordCreate(DISCORD_VERSION, &params, &app.core);
    _discord_connected = (result == DiscordResult_Ok);
}

bool Discord_connected() {
    return _discord_connected;
}

void Discord_update() {
    app.core->run_callbacks(app.core);
}