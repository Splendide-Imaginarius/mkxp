#include "discord.h"
#include "util/debugwriter.h"
#include "sharedstate.h"
#include "filesystem/filesystem.h"
#include <cstdio>

struct Application app;
struct IDiscordActivityEvents Discord_activityEvents;
bool _discord_connected;

void Discord_init() {
    // Don't forget to memset or otherwise initialize your classes!
    memset(&app, 0, sizeof(Application));

    struct DiscordCreateParams params;
    params.client_id = DISCORD_APPID;
    params.flags = DiscordCreateFlags_NoRequireDiscord;
    params.event_data = &app;
    params.activity_events = &Discord_activityEvents;

    EDiscordResult result = DiscordCreate(DISCORD_VERSION, &params, &app.core);
    _discord_connected = (result == DiscordResult_Ok);

    std::string path = "%s/Game.exe" + mkxp_fs::getDefaultGameRoot();
    Discord_ActivityManager->register_command(Discord_ActivityManager, path.c_str());

    Discord_activityEvents.on_activity_invite = &Discord_onActivityInvite;
    Discord_activityEvents.on_activity_join = &Discord_onActivityJoin;
    Discord_activityEvents.on_activity_join_request = &Discord_onActivityJoinRequest;
    Discord_activityEvents.on_activity_spectate = &Discord_onActivitySpectate;
}

bool Discord_connected() {
    return _discord_connected;
}

void Discord_update() {
    app.core->run_callbacks(app.core);
}