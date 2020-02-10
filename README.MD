# LIFX ADDON FOR OPENFRAMEWORKS

**Short story**

Around 2016 we had the idea to use several Lifx Color 1000 bulbs for a theater show, the light being emitted by lamps positioned on the stage and completed by "conventional" hallogen ones. At these times, I found no tool that was OK for my needs, so I decided to code this one.

This ofx addon have been running hundreds of hours with very few glitches (most of the time caused by bulbs losing wifi connection) during rehearsals and shows (and still is).

A short demo is available here : https://www.youtube.com/watch?v=M22dPxR8DL8

**Disclaimer**

The source code is provided as-is, it is designed as quick-n-dirty tool intended to be used within a specific environment that might not fit your bill. The code have never been used or even compiled on Linux or Windows (OSX only).

**Requirements**

- OpenFrameworks 0.9.8
- Lifx Color 1000, with non-upgraded firmware from 2017
- A compatible wifi access point : I use very common (and quite old) internet boxes from Bouygues Telecom. Simple access points should work or not, tests are needed, I had no results with Linksys WRT54G with DD-WRT custom firmware
- All the bulbs must be named in a sensible way with the Lifx proprietary Android or IOS app. In my case I used B1, B2, B3 and so on

The whole stuff might run with recent OpenFrameworks and/or Lifx firmware versions, or not.

**Usage**

There's no source code example, but using this addon in your ofx app is quite straightforward

- git clone the repo under the ofx addons directory
- edit the src/defines.h file and set the OFX_TALIFX_BROADCAST_ADDRESS define accordingly to your setup

Somewhere in your app's source code

- drop an `#include "ofxTALifxClient.h"`
- define a var like `ofxtalifx::ofxTALifxClient lifx;`
- in your app's setup method, call `lifx.startThread();`
- in your app's teardown method, call `lifx.stopThread();`

Driving the bulbs

- Have a look at `ofxTALifxClient.cpp` file, around line #260
- With all the calls you can pass a bulb name, a bulbs group name starting with '#' or '*' (all bulbs) as a parameter
- The idea or 'groups' shares nothing with groups as defined in Lifx protocol, it's only a client side way of driving multiple bulbs in one call
- Have a look at `ofxTALifxClient.cpp` file, around line #70 to learn how to create/remove a group, add or remove a bulb to a group


Have fun ! ;-)


*(C) 2016/2017 Christophe Gimenez for TACTIF Compagnie / Bordeaux - France*

*Lifx messages protocol structures adapted from Andrew Kane lifx lib https://github.com/codemaster/lib-lifx*
