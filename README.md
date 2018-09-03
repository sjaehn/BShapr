# B.Slicer
Repository: bmusic-bslicer

Status: **Experimental**

Description: LV2 audio effect plugin. This plugin slices stereo audio input signals with a length up to 1 bar into up to 16 pieces.
The result will be a step sequencer-like effect.

Dependencies
------------
```
glibmm-2.4
sigc++-2.0
gtkmm-2.4
cairomm-1.0
lv2
```

Installation
------------
Once deps are satisfied, building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
sudo make install
```
from the directory where you downloaded the repository files.

For installation into an alternative directory, modify line 2 in the makefile.

Running
-------
After the installation Ardour, Carla, and any other LV2 host should
automatically detect B.Slicer.

Limitations
-----------
As this version of B.Slicer is based on GTK2, it will NOT run (directly) in applications that use other versions of GTK.
If you use such an application, load Carla-Patchbay instead. Load and connect B.Slicer in Carla-Patchbay.
