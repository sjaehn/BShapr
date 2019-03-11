# B.Slizzr
Repository: BSlizzr

Description: LV2 audio effect plugin. This plugin slices stereo audio input signals with a length up to 1 bar into up to 16 pieces. Each slice can be levelled up or down. The result can be a step sequencer-like effect.

![screenshot](https://raw.githubusercontent.com/sjaehn/bmusic-bslicer/master/Screenshot.png "Screenshot from B.Slicer")

Installation
------------
a) Download and unpack the most recent release binaries from
https://github.com/sjaehn/bmusic-bslicer/releases to your lv2 directory (e.g., /usr/lib/lv2/).

b) Build your own binaries in the following three steps.

Step 1: Clone or download this repository.

Step 2: Install the development packages for x11, cairo, and lv2 if not done yet.

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
sudo make install
```
from the directory where you downloaded the repository files.

For installation into an alternative directory (e.g., /usr/local/lib/lv2), modify line 2 in the makefile.

Running
-------
After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Slizzr.

If jalv is installed, you can also call it
```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BSlizzr
```
to run it stand-alone and connect it to the JACK system.

What's new
-----------
* Compatibility with negative speed (reverse playing)
* .ttl files updated

Usage
-----
The plugin slices a stereo input, amplifies or silences the individual slices and send the whole sequence to the output. Although this affects only the audio signal, it needs a playback running.

The interface is divided into three parts: step controls, monitor and step shape.

Step controls
* Sequences per bar : Number of sequences in one bar (1..8)
* Number of steps : Number of steps in one sequence (1..16)
* Step control : Coefficient for sound reduction for each individual step

Monitor
* On/Off switch: Switches monitor and monitor <-> plugin communication on/off. Reduces CPU load.
* Scale : Scales the visualization of the input / output signal
* Monitor : Visualization (l + r signal) the input / output signal for a whole sequence

Step shape
* Attack
* Decay
* Monitor : Visualization of a single step

Links
-----
* Preview video: https://www.youtube.com/watch?v=1VD3KsXvyLU




