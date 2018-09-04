# B.Slicer
Repository: bmusic-bslicer

Status: Testing

Description: LV2 audio effect plugin. This plugin slices stereo audio input signals with a length up to 1 bar into up to 16 pieces.
The result will be a step sequencer-like effect.

Dependencies
------------
```
gtk+-2.0
cairo
lv2
```

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
