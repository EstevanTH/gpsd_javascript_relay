# GPSD to JavaScript relay
A small tool producing JS &amp; JSON output based on GPS data provided by a GPSD server

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
#### Customizable refresh rate
Because we do not have the same needs and the same GPSD server!
In addition, the program adds the field `relaytime` (milliseconds timestamp) in JSON and JS objects when a GPSD response is received.
#### Support for GPSD servers through TCP, UDP and symmetric UDP
For a maximum compatibility and the largest choose, support is provided for all common protocols!
#### Support for IPv6 and IPv4
This program has full IPv6 compatibility thanks to Qt!
