# Apollo Launcher

:rotating_light: :construction: This repository is under construction! Please proceed with caution! :construction: :rotating_light:

## Building from Source

Currently, the only way to use this launcher is to build it manually from source. Fortunately, I have provided a script which should handle the building process.

:exclamation: The build process has only been tested on macOS. It *should* work on Linux too, but Windows support is, for now, at your own risk.

To build from source, you will need the following installed on your machine:

- [devkitPro](https://devkitpro.org/wiki/Getting_Started), devkitPPC and libogc, with DEVKITPRO and DEVKITPPC being set in your environment variables.
- [Ninja](https://ninja-build.org/)
- [GRRLIB](https://github.com/GRRLIB/GRRLIB)
- Python 3

To build, simply run the `build.py` file in the root of the repository. It will automatically generate a `build.ninja` file and spawn `ninja` to build it. Artifacts are created in the `dist/` folder.

## Thanks!

A special thank you to:

- [stebler](https://github.com/stblr): for the (now abandoned) [MKW-SP project](https://github.com/mkw-sp/mkw-sp). It's one of the things which originally inspired me to create apollo.
- [Chadderz](https://github.com/Chadderz121): for [BrainSlug](https://github.com/Chadderz121/brainslug-wii). BrainSlug's base is the starting point for Apollo and lots of code will be inspired/used from it (thank you!).
- You: for checking out Apollo, it really means a lot to me.
