#include "binding-util.h"
#include "sharedstate.h"
#include "util/debugwriter.h"
#include "discord.h"

typedef struct Discord_rubyCallback {
    VALUE proc;
} Discord_rubyCallback;

VALUE Discord_module;
VALUE Discord_user_rb;

/* Discord */

/*  update_activity   */
/*  args:             */
/*    activity: Hash  */
RB_METHOD(updateActivity) {
    if (!Discord_connected())
        return Qfalse;

    VALUE hActivity;
    rb_scan_args(argc, argv, "1", &hActivity);

    struct DiscordActivity activity;
    memset(&activity, 0, sizeof(activity));

    // game details/state
    VALUE details = rb_hash_aref(hActivity, ID2SYM(rb_intern("details")));
    if (details != Qnil)
        sprintf(activity.details, "%s", rb_string_value_cstr(&details));
    VALUE state = rb_hash_aref(hActivity, ID2SYM(rb_intern("state")));
    if (state != Qnil)
        sprintf(activity.state, "%s", rb_string_value_cstr(&state));

    // timestamps
    VALUE startTimestamp = rb_hash_aref(hActivity, ID2SYM(rb_intern("start_timestamp")));
    if (startTimestamp != Qnil)
        activity.timestamps.start = NUM2INT(startTimestamp);
    VALUE endTimestamp = rb_hash_aref(hActivity, ID2SYM(rb_intern("end_timestamp")));
    if (endTimestamp != Qnil)
        activity.timestamps.start = NUM2INT(endTimestamp);

    // images
    // large image
    VALUE largeImage = rb_hash_aref(hActivity, ID2SYM(rb_intern("large_image")));
    if (largeImage != Qnil)
        sprintf(activity.assets.large_image, "%s", rb_string_value_cstr(&largeImage));
    VALUE largeImageText = rb_hash_aref(hActivity, ID2SYM(rb_intern("large_image_text")));
    if (largeImageText != Qnil)
        sprintf(activity.assets.large_text, "%s", rb_string_value_cstr(&largeImageText));
    // small image
    VALUE smallImage = rb_hash_aref(hActivity, ID2SYM(rb_intern("small_image")));
    if (smallImage != Qnil)
        sprintf(activity.assets.small_image, "%s", rb_string_value_cstr(&smallImage));
    VALUE smallImageText = rb_hash_aref(hActivity, ID2SYM(rb_intern("small_image_text")));
    if (smallImageText != Qnil)
        sprintf(activity.assets.small_text, "%s", rb_string_value_cstr(&smallImageText));

    // party details
    VALUE partyId = rb_hash_aref(hActivity, ID2SYM(rb_intern("party_id")));
    if (partyId != Qnil)
        sprintf(activity.party.id, "%s", rb_string_value_cstr(&partyId));
    VALUE partySize = rb_hash_aref(hActivity, ID2SYM(rb_intern("party_size")));
    if (partySize != Qnil)
        activity.party.size.current_size = NUM2INT(partySize);
    VALUE partyMax = rb_hash_aref(hActivity, ID2SYM(rb_intern("party_max")));
    if (partyMax != Qnil)
        activity.party.size.max_size = NUM2INT(partyMax);

    // activity secrets
    VALUE joinSecret = rb_hash_aref(hActivity, ID2SYM(rb_intern("join_secret")));
    if (joinSecret != Qnil)
        sprintf(activity.secrets.join, "%s", rb_string_value_cstr(&joinSecret));
    VALUE spectateSecret = rb_hash_aref(hActivity, ID2SYM(rb_intern("spectate_secret")));
    if (spectateSecret != Qnil)
        sprintf(activity.secrets.spectate, "%s", rb_string_value_cstr(&spectateSecret));

    Discord_ActivityManager->update_activity(Discord_ActivityManager, &activity, &app, NULL);

    return Qtrue;
}

void activityManager_callback(void* callback_data, enum EDiscordResult result) {
    if (callback_data == NULL)
        return;

    Discord_rubyCallback* callback = (Discord_rubyCallback*)callback_data;

    // call the block
    rb_proc_call(callback->proc, rb_ary_new_from_args(1, INT2NUM(result)));
    free(callback);
}

RB_METHOD(sendRequestReply) {
    VALUE id, response;
    Discord_rubyCallback* block = (Discord_rubyCallback*)malloc(sizeof(Discord_rubyCallback));
    rb_scan_args(argc, argv, "2&", &id, &response, &block->proc);

    void* callback_data = NULL;
    if (rb_block_given_p())
        callback_data = (void*)block;

    Discord_ActivityManager->send_request_reply(Discord_ActivityManager, NUM2LL(id), (EDiscordActivityJoinRequestReply)NUM2INT(response), callback_data, activityManager_callback);

    return Qtrue;
}

/*  update_activity     */
/*  args:               */
/*    event:    Symbol  */
/*    callback: proc    */
RB_METHOD(addEventCallback) {
    if (!Discord_connected())
        return Qfalse;

    VALUE sym, proc;
    rb_scan_args(argc, argv, "2", &sym, &proc);

    VALUE events = rb_cv_get(Discord_module, "@@events");
    VALUE eventArr = rb_hash_aref(events, sym);
    if (eventArr == Qnil)
        rb_raise(rb_eArgError, "Attempted to add callback to nonexistent Discord event.");

    rb_ary_push(eventArr, proc);

    return Qtrue;
}

/* Discord::User */

RB_METHOD(userInitialize) {
    // get args
    VALUE id, username, discriminator, avatar, bot;
    rb_scan_args(argc, argv, "5", &id, &username, &discriminator, &avatar, &bot);
    // set attributes

    rb_iv_set(self, "@id", id);
    rb_iv_set(self, "@username", username);
    rb_iv_set(self, "@discriminator", discriminator);
    rb_iv_set(self, "@avatar", avatar);
    rb_iv_set(self, "@bot", bot);

    return self;
}


void DiscordBindingInit() {
    Discord_module = rb_define_module("Discord");

    VALUE Discord_ActivityJoinRequestReply = rb_define_module_under(Discord_module, "ActivityJoinRequestReply");
    rb_define_const(Discord_ActivityJoinRequestReply, "NO", INT2NUM(DiscordActivityJoinRequestReply_No));
    rb_define_const(Discord_ActivityJoinRequestReply, "YES", INT2NUM(DiscordActivityJoinRequestReply_Yes));
    rb_define_const(Discord_ActivityJoinRequestReply, "IGNORE", INT2NUM(DiscordActivityJoinRequestReply_Ignore));

    // DiscordUser wrapper
    Discord_user_rb = rb_define_class_under(Discord_module, "User", rb_cObject);
    _rb_define_method(Discord_user_rb, "initialize", userInitialize);
    rb_define_attr(Discord_user_rb, "id", 1, 1);
    rb_define_attr(Discord_user_rb, "username", 1, 1);
    rb_define_attr(Discord_user_rb, "discriminator", 1, 1);
    rb_define_attr(Discord_user_rb, "avatar", 1, 1);
    rb_define_attr(Discord_user_rb, "bot", 1, 1);


    // Activities event handlers
    VALUE events = rb_hash_new();
    rb_hash_aset(events, ID2SYM(rb_intern("on_activity_join")), rb_ary_new());
    rb_hash_aset(events, ID2SYM(rb_intern("on_activity_spectate")), rb_ary_new());
    rb_hash_aset(events, ID2SYM(rb_intern("on_activity_join_request")), rb_ary_new());
    rb_hash_aset(events, ID2SYM(rb_intern("on_activity_invite")), rb_ary_new());
    rb_cv_set(Discord_module, "@@events", events);

    
    // module functions
    _rb_define_module_function(Discord_module, "update_activity", updateActivity);
    _rb_define_module_function(Discord_module, "send_request_reply", sendRequestReply);
    _rb_define_module_function(Discord_module, "add_event_callback", addEventCallback);
}

// create a ruby Discord::User from a pointer to a C DiscordUser struct
VALUE _rb_discord_user_new_cstruct(struct DiscordUser* user) {
    VALUE argv[] = {
        LL2NUM(user->id),
        rb_str_new_cstr(user->username),
        rb_str_new_cstr(user->discriminator),
        rb_str_new_cstr(user->avatar),
        rb_bool_new(user->bot)
    };

    return rb_class_new_instance(5, argv, Discord_user_rb);
}

void DISCORD_CALLBACK Discord_onActivityJoin(void* event_data, const char* secret) {
    VALUE events = rb_cv_get(Discord_module, "@@events");
    VALUE onActivityJoinEvents = rb_hash_aref(events, ID2SYM(rb_intern("on_activity_join")));
    if (onActivityJoinEvents != Qnil) {
        for (long i = 0l; i < rb_array_len(onActivityJoinEvents); i++) {
            rb_proc_call(rb_ary_entry(onActivityJoinEvents, i), rb_ary_new_from_args(1, rb_str_new_cstr(secret)));
        }
    }
}

void DISCORD_CALLBACK Discord_onActivitySpectate(void* event_data, const char* secret) {
    VALUE events = rb_cv_get(Discord_module, "@@events");
    VALUE onActivitySpectate = rb_hash_aref(events, ID2SYM(rb_intern("on_activity_spectate")));
    if (onActivitySpectate != Qnil) {
        for (long i = 0l; i < rb_array_len(onActivitySpectate); i++) {
            rb_proc_call(rb_ary_entry(onActivitySpectate, i), rb_ary_new_from_args(1, rb_str_new_cstr(secret)));
        }
    }
}

void DISCORD_CALLBACK Discord_onActivityJoinRequest(void* event_data, struct DiscordUser* user) {
    VALUE events = rb_cv_get(Discord_module, "@@events");
    VALUE onActivityJoinRequest = rb_hash_aref(events, ID2SYM(rb_intern("on_activity_join_request")));
    if (onActivityJoinRequest != Qnil) {
        for (long i = 0l; i < rb_array_len(onActivityJoinRequest); i++) {
            rb_proc_call(rb_ary_entry(onActivityJoinRequest, i), rb_ary_new_from_args(1, _rb_discord_user_new_cstruct(user)));
        }
    }
}

void DISCORD_CALLBACK Discord_onActivityInvite(void* event_data, enum EDiscordActivityActionType type, struct DiscordUser* user, struct DiscordActivity* activity) {

}