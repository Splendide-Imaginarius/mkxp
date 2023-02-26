#include "discord.h"
#include "util/debugwriter.h"
#include "sharedstate.h"
#include <cstdio>

struct Application app;

void Discord_init() {
    // Don't forget to memset or otherwise initialize your classes!
    memset(&app, 0, sizeof(Application));

    Debug() << "help";

    struct DiscordCreateParams params;
    params.client_id = DISCORD_APPID;
    params.flags = DiscordCreateFlags_NoRequireDiscord;
    params.event_data = &app;

    DiscordCreate(DISCORD_VERSION, &params, &app.core);

    struct DiscordActivity activity;
    memset(&activity, 0, sizeof(activity));
    sprintf(activity.details, "the mkxp-z with discord integration");

    app.core->get_activity_manager(app.core)->update_activity(app.core->get_activity_manager(app.core), &activity, &app, NULL);
}

bool Discord_connected() {
    return true;
}

void Discord_update() {
    app.core->run_callbacks(app.core);
}