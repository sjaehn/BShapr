# B.Shapr
Description: Beat / envelope shaper LV2 plugin

![screenshot](https://raw.githubusercontent.com/sjaehn/BShapr/master/screenshot.png "Screenshot from B.Shapr")


## Installation

Build your own binaries in the following three steps.

Step 1: Clone or download this repository.

Step 2: Install the development packages for x11, cairo, and lv2 if not done yet.

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:

```
sudo make install
```

from the directory where you downloaded the repository files.

For installation into an alternative directory (e.g., /usr/local/lib/lv2), modify line 2 in the makefile.

## Running

After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Shapr.

If jalv is installed, you can also call it

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BShapr
```

to run it stand-alone and connect it to the JACK system.

Notes:

* **Jack transport is required to get information about beat / position**
* **Don't forget to select an input for each shape you use and an output (see usage/input and usage/output)**

## Usage

B.Shapr is an envelope plugin for time or beat position-dependent effects.
The user can define up to four different envelope shapes by drawing Bezier curves. Each of these envelope
shapes can be connected to different target effects, such as level, balance, or stereo width, and can be
combined together.

### Monitor

The Monitor in the upper part of the GUI visualizes the input and the output levels. You can change the zoom
by scrolling the mouse wheel.

### Sequence size

You can set the lenght of the whole shape sequence between 1 and 16 seconds, beats or bars. Change the value
by dragging, scrolling or clicking on its up and down arrows and select a base.

### Shape selector

Select between the four user-defineable shapes.

### Input

Select one input for this shape. You can choose between the audio input, a constant signal, or a previous shape.
Use the dial to set the level of the input signal.

### Shape editor

Mark and select a node type on the bottom of the editor. Click on the desired position of the shape to set
this node (or to delete it if the deletion tool is marked). Mark a node by clicking on it. You can drag a node
or its handles.

However, dragging the background results only in dragging the whole **display** of the envelope. Similarly,
scrolling zooms the display in or out.

### Target

Select a target effect for this shape. E.g., if you select balance, the input signal will be panned according
to the shape envelope.

### Output

Select the output for this shape. Mark "audio" if you want to send the output directly to audio out. Otherwise
the signal will only be used internally.

## TODO

* Stereo monitor
* GUI performance
* Optimize dynamic pitch and delay methods
* Additional target effects (any ideas welcome)

## See also

* Preview: https://www.youtube.com/watch?v=DxYQJ_XJwbU
* Autotune with B.Shapr: https://youtu.be/c6bUW_dTxGg
