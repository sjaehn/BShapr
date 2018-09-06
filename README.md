# B.Slicer
Repository: bmusic-bslicer

Status: Testing

Description: LV2 audio effect plugin. This plugin slices stereo audio input signals with a length up to 1 bar into up to 16 pieces.
The result will be a step sequencer-like effect.


![screenshot](https://raw.githubusercontent.com/sjaehn/bmusic-bslicer/master/Screenshot.png "Screenshot from B.Slicer")

Dependencies
------------
The development packages of
```
gtk+-2.0
cairo
lv2
```
are required for building BSlicer.

Installation
------------
Once dependencies are satisfied, building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
sudo make install
```
from the directory where you downloaded the repository files.

For installation into an alternative directory (e.g., /usr/local/lib/lv2), modify line 2 in the makefile.

Running
-------
After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Slicer.

If jalv is installed, you can also call it
```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BSlicer
```
to run it stand-alone and connect it to the JACK system.

Limitations
-----------
As this version of B.Slicer is based on GTK2, it will NOT run (directly) in applications that use other versions of GTK.
If you use such an application, load Carla-Patchbay instead. Load and connect B.Slicer in Carla-Patchbay.

Usage
-----
The plugin slices a stereo input, amplifies or silences the individual slices and send the whole sequence to the output. Although this affects only the audio signal, it needs a playback running.

The interface is devided in three parts: step controls, monitor and step shape.

Step controls
* Note = 1/x : Seqence size in 1/x bars (1..8)
* Number of steps : Number of steps in a sequence (1..16)
* Step control : Sets gain for each individual step

Monitor
* On/Off switch
* Scale : Scales the visualization of the input / output signal
* Monitor : Visualization (l + r signal) the input / output signal for a whole sequence

Step shape
* Attack
* Decay
* Monitor : Visualization of a single step

Links
-----
* Preview video: https://www.youtube.com/watch?v=1VD3KsXvyLU




