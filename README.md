# mkxp-zr

This is a fork of mkxp-z, which is a fork of the RGSS reimplementation mkxp originally intended to run Pokémon Essentials games (which were notorious for their dependency on a lot of Windows API calls). Original its only purpose was to add Discord Game SDK support directly to mkxp for the 2023 Relic Castle Winter Jam entry [Pivot](https://reliccastle.com/threads/6203/), but I found the ability to add new ways of interacting with the underlying engine behind RGSS extremely useful for my own games as well.

It supports Windows, and probably Linux and x86/ARM macOS as well.

## Building
mkxp-z's build system is fairly easy to follow, however there are a few steps. Thankfully, these steps have already been outlined far better than I ever could, right [here (Windows/Linux)](https://roza-gb.gitbook.io/mkxp-z/compilation) and [here (macOS)](https://roza-gb.gitbook.io/mkxp-z/building-xcode/compilation-macos-xcode).
Obviously, nothing implemented by this fork is covered in the mkxp-z gitbook or wiki. There are a few options for configuring those things:

| Option | Type | Description | Default |
| --- | --- | --- | --- |
| discord_path | string | Path to the [Discord Game SDK](https://discord.com/developers/docs/game-sdk/sdk-starter-guide) for building with Discord support | '' |
| discord_appid | string | Your game's [Discord application ID](https://discord.com/developers/applications) | '' |
| windows_resource_directory | string | Name of Windows EXE resource directory (for changing the EXE icon etc) | 'windows' |

## Midi music

mkxp doesn't come with a soundfont by default, so you will have to supply it yourself (set its path in the config). Playback has been tested and should work reasonably well with all RTP assets.

You can use this public domain soundfont: [GMGSx.sf2](https://www.dropbox.com/s/qxdvoxxcexsvn43/GMGSx.sf2?dl=0)

## macOS Controller Support

Binding controller buttons on macOS is slightly different depending on which version you are running. Binding specific buttons requires different versions of the operating system:

+ **Thumbstick Button (L3/R3, LS/RS, L↓/R↓)**: macOS Mojave 10.14.1+
+ **Start/Select (Options/Share, Menu/Back, Plus/Minus)**: macOS Catalina 10.15+
+ **Home (Guide, PS)**: macOS Big Sur 11.0+

Technically, while SDL itself might support these buttons, the keybinding menu had to be rewritten in Cocoa in a hurry, as switching away from native OpenGL broke the original keybinding menu. (ANGLE is used instead, to prevent crashing on Apple Silicon releases of macOS, and to help mkxp switch to Metal)

## Fonts

In the RMXP version of RGSS, fonts are loaded directly from system specific search paths (meaning they must be installed to be available to games). Because this whole thing is a giant platform-dependent headache, Ancurio decided to implement the behavior Enterbrain thankfully added in VX Ace: loading fonts will automatically search a folder called "Fonts", which obeys the default searchpath behavior (ie. it can be located directly in the game folder, or an RTP).

If a requested font is not found, no error is generated. Instead, a built-in font is used. By default, this font is Liberation Sans.

## What doesn't work
* wma audio files
* Creating Bitmaps with sizes greater than your hardware's texture size limit (either 16384 on an average modern GPU or 32768 for modern dedicated GPUs).^

^ There is an exception to this, called *mega surface*. When a Bitmap bigger than the texture limit is created from a file, it is not stored in VRAM, but regular RAM. Its sole purpose is to be used as a tileset bitmap. Any other operation to it (besides blitting to a regular Bitmap) will result in an error.
