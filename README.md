# GPSD to JavaScript relay
A small tool producing JS &amp; JSON output based on GPS data provided by a GPSD server

The program is available in :us: English and :fr: French.

**:dart: Beta stage:** Please try hard the program, do unexpected actions, try various GPSD servers, etc.

:blue_book: Find more information on [the Wiki of this project](/wiki)!

## Introduction
This is my first mini-project in C++. As a beginner I would like to meet other C++ developers and fix bugs.

The aim of this program is to provide access to GPS data as JavaScript and JSON output.

Output is available through files or through the built-in mini HTTP server.

Only the TPV class from [the GPSD protocol](http://www.catb.org/gpsd/gpsd_json.html) is available.

This project uses [the Qt framework](http://doc.qt.io/qt-5/), which provides an amazing easy-to-use multi-platform availability.

## Features
#### Built-in mini HTTP server
The easiest way to use some AJAX is with an HTTP server, with the lowest possible delay!
#### Connection to multiple GPSD servers
If you have several GPS devices, I thought about you!
#### Multiple hosts for a single GPSD server
If you have several networks, no problem!
#### Auto-reconnect when connection is lost
The GPSD server crashed? The WiFi does not work well? Do not worry!
#### Auto-start for HTTP server and GPSD clients
Well, clicking buttons every time is boring, isn't it?
#### Auto-save configuration
Filling fields is a pain, so your configuration is always saved on exit!
#### Hide in system tray
If you use the program for a long period of time, you'll love it.
#### Bunch of command-line parameters
- `--confignoload` - run program with a new fresh configuration
- `--confignosave` - do not save configuration on exit
- `--minimized` - minimize in tray on startup
- `--noautostart` - do not auto-start HTTP server and GPSD clients
- `--traynomessages` - disable messages from tray icon

#### Customizable refresh rate
Because we do not have the same needs and the same GPSD server!

In addition, the program adds the field `relaytime` (milliseconds timestamp) in JSON and JS objects when a GPSD response is received.
#### Support for GPSD servers through TCP, UDP and symmetric UDP
For a maximum compatibility and the largest choose, support is provided for all common protocols!
#### Support for IPv6 and IPv4
This program has full IPv6 compatibility thanks to Qt!

## Preview
![Interface Status](http://puu.sh/u8QPd/4f619d3a25.png)
![Interface Servers](http://puu.sh/u8QTM/29f411cef9.png)
![Interface Targets](http://puu.sh/u8QUG/ffef1ab94e.png)
![Interface About](http://puu.sh/u8QVo/176f40c5f8.png)
![HTTP JavaScript](http://puu.sh/u8QWj/3504b8afdd.png)
![HTTP JSON](http://puu.sh/u8QX7/4fb5d0776d.png)
