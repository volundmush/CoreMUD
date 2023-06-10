# CoreMUD

## WARNING: Early Alpha!
Pardon our dust, this project is still in its infancy. It runs, but if you're not a developer intent on sprucing up, it may not have much for you just yet.

## CONTACT INFO
**Name:** Volund

**Email:** volundmush@gmail.com

**PayPal:** volundmush@gmail.com

**Discord:** volundmush

**Discord Channel:** https://discord.gg/Sxuz3QNU8U

**Patreon:** https://www.patreon.com/volund

**Home Repository:** https://github.com/volundmush/CoreMUD

## TERMS AND CONDITIONS

MIT license. In short: go nuts, but give credit where credit is due.

Please see the included LICENSE.txt for the legalese.

## INTRO
MUDs and their brethren are the precursors to our modern MMORPGs, and are still a blast to play - in addition to their other uses, such as educative game design: all the game logic, none of the graphics!

Writing one from scratch isn't easy though, so this library aims to take away a great deal of the boilerplate pain.

CoreMUD is a library that aims to provide a clean and straightforward approach to creating a MUD server backend which uses the [Thermite](https://github.com/volundmush/thermite) project as a networking portal.

This library isn't a MUD. It's not a MUSH, or a MUX, or a MOO, or MUCK on its own, though. In truth, it doesn't DO very much. That's a good thing! See, it doesn't make (many) decisions for the developers it's meant for, making it easy to build virtually ANY kind of text-based multiplayer game atop of it.

## FEATURES
* Asynchronous Networking courtesy of Boost ASIO
* Robust Telnet handling via [Thermite](https://github.com/volundmush/thermite)
* Hyper-extendable codebase where all the guts are easily replaced.

## OKAY, BUT HOW DO I USE IT?
Glad you asked! A full wiki / tutorial is in the works, but for now, here's the basics:

1. Install Rust and Cargo. You can find instructions [here](https://www.rust-lang.org/tools/install).
2. Download the [Thermite](https://github.com/volundmush/thermite) project code and build it. There's not currently an official launch process, just use cargo run.
3. Download the CoreMUD library and add it to your project for static linking. The easiest way to do this is probably to use [CPM](https://github.com/cpm-cmake/CPM.cmake) and if you look at this project's CMakeLists.txt you'll see exactly how.
4. Write your C++ code, including CoreMUD files like you would any statically linked library. The general startup process is to set the global variables in core::config, then call core::setup() and core::startup().

If you examine the source code, you'll notice that virtually everything can be replaced thanks to copious usage of std::function and even vectors of std::function. These are used to replace specific calls with your own logic, or compose large operations to happen at certain points in time.

Major things every game will need to do:
* Replace core::config::makeConnection with their own function which instantiates a subclass of core::Connection.
* Adjust the core::config options to your liking.
* Register systems to be called during the heartbeat, or replace the heartbeat entirely.
* Register shutdown/restart routines.

An actual game is going to need far more than just that, but that's the basics.


## FAQ
__Q:__ This is cool! How can I help?  
__A:__ [Patreon](https://www.patreon.com/volund) support is always welcome. If you can code and have cool ideas or bug fixes, feel free to fork, edit, and pull request! Join our [discord](https://discord.gg/Sxuz3QNU8U) to really get cranking away though.

__Q:__ I found a bug! What do I do?  
__A:__ Post it on this GitHub's Issues tracker. I'll see what I can do when I have time. ... or you can try to fix it yourself and submit a Pull Request. That's cool too.

## Special Thanks
* The [Evennia](https://github.com/evennia/evennia) Project. A lot of my style has evolved from efforts with Evennia, the dual-process idea for Portal+Server is definitely from them.
* All of my Patrons on Patreon.
* Anyone who contributes to this project or my other ones.