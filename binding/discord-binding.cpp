#include "binding-util.h"
#include "sharedstate.h"
#include "util/debugwriter.h"
#include "discord.h"

VALUE sym_details;
VALUE sym_state;
VALUE sym_startTimestamp;
VALUE sym_endTimestamp;
VALUE sym_largeImage;
VALUE sym_largeImageText;
VALUE sym_smallImage;
VALUE sym_smallImageText;
VALUE sym_partyId;
VALUE sym_partySize;
VALUE sym_partyMax;
VALUE sym_joinSecret;
VALUE sym_spectateSecret;

RB_METHOD(updateActivity) {
    if (!Discord_connected())
        return Qfalse;

    VALUE hActivity;
    rb_scan_args(argc, argv, "1", &hActivity);

    struct DiscordActivity activity;
    memset(&activity, 0, sizeof(activity));

    // game details/state
    VALUE details = rb_hash_aref(hActivity, sym_details);
    if (details != Qnil)
        sprintf(activity.details, "%s", rb_string_value_cstr(&details));
    VALUE state = rb_hash_aref(hActivity, sym_state);
    if (state != Qnil)
        sprintf(activity.state, "%s", rb_string_value_cstr(&state));

    // timestamps
    VALUE startTimestamp = rb_hash_aref(hActivity, sym_startTimestamp);
    if (startTimestamp != Qnil)
        activity.timestamps.start = NUM2INT(startTimestamp);
    VALUE endTimestamp = rb_hash_aref(hActivity, sym_endTimestamp);
    if (endTimestamp != Qnil)
        activity.timestamps.start = NUM2INT(endTimestamp);

    // images
    // large image
    VALUE largeImage = rb_hash_aref(hActivity, sym_largeImage);
    if (largeImage != Qnil)
        sprintf(activity.assets.large_image, "%s", rb_string_value_cstr(&largeImage));
    VALUE largeImageText = rb_hash_aref(hActivity, sym_largeImageText);
    if (largeImageText != Qnil)
        sprintf(activity.assets.large_text, "%s", rb_string_value_cstr(&largeImageText));
    // small image
    VALUE smallImage = rb_hash_aref(hActivity, sym_smallImage);
    if (smallImage != Qnil)
        sprintf(activity.assets.small_image, "%s", rb_string_value_cstr(&smallImage));
    VALUE smallImageText = rb_hash_aref(hActivity, sym_smallImageText);
    if (smallImageText != Qnil)
        sprintf(activity.assets.small_text, "%s", rb_string_value_cstr(&smallImageText));

    // party details
    VALUE partyId = rb_hash_aref(hActivity, sym_partyId);
    if (partyId != Qnil)
        sprintf(activity.party.id, "%s", rb_string_value_cstr(&partyId));
    VALUE partySize = rb_hash_aref(hActivity, sym_partySize);
    if (partySize != Qnil)
        activity.party.size.current_size = NUM2INT(partySize);
    VALUE partyMax = rb_hash_aref(hActivity, sym_partyMax);
    if (partyMax != Qnil)
        activity.party.size.max_size = NUM2INT(partyMax);

    // activity secrets
    VALUE joinSecret = rb_hash_aref(hActivity, sym_joinSecret);
    if (joinSecret != Qnil)
        sprintf(activity.secrets.join, "%s", rb_string_value_cstr(&joinSecret));
    VALUE spectateSecret = rb_hash_aref(hActivity, sym_spectateSecret);
    if (spectateSecret != Qnil)
        sprintf(activity.secrets.spectate, "%s", rb_string_value_cstr(&spectateSecret));

    app.core->get_activity_manager(app.core)->update_activity(app.core->get_activity_manager(app.core), &activity, &app, NULL);

    return Qtrue;
}

void DiscordBindingInit() {
    // create symbols for rpc hash
    sym_details = ID2SYM(rb_intern("details"));
    sym_state = ID2SYM(rb_intern("state"));
    sym_startTimestamp = ID2SYM(rb_intern("start_imestamp"));
    sym_endTimestamp = ID2SYM(rb_intern("end_timestamp"));
    sym_largeImage = ID2SYM(rb_intern("large_image"));
    sym_largeImageText = ID2SYM(rb_intern("large_image_text"));
    sym_smallImage = ID2SYM(rb_intern("small_image"));
    sym_smallImageText = ID2SYM(rb_intern("small_image_text"));
    sym_partyId = ID2SYM(rb_intern("party_id"));
    sym_partySize = ID2SYM(rb_intern("party_size"));
    sym_partyMax = ID2SYM(rb_intern("party_max"));
    sym_joinSecret = ID2SYM(rb_intern("join_secret"));
    sym_spectateSecret = ID2SYM(rb_intern("spectate_secret"));

    // bindings
    VALUE module = rb_define_module("Discord");
    
    _rb_define_module_function(module, "update_activity", updateActivity);
}