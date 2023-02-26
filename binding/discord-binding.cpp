#include "binding-util.h"
#include "sharedstate.h"
#include "util/debugwriter.h"
#include "discord.h"

RB_METHOD(runCallbacks) {

    // run discord callbacks
    //shState->discord()->core->run_callbacks(shState->discord()->core);

    Debug() << "this would run the discord callbacks";

    return Qnil;
}

void DiscordBindingInit() {
    VALUE module = rb_define_module("Discord");
    
    _rb_define_module_function(module, "run_callbacks", runCallbacks);
}