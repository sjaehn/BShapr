# B.Shapr
Description: Beat / envelope shaper LV2 plugin

![screenshot](https://raw.githubusercontent.com/sjaehn/BShapr/master/screenshot.png "Screenshot from B.Shapr")


## Installation

Build your own binaries in the following three steps.

Step 1: Clone or download this repository.

Step 2: Satisfy dependencies for libx11-dev, libcairo2-dev, lv2-dev, and pkg-config, if not done yet.

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
make
sudo make install PREFIX=/usr
```
from the directory where you downloaded the repository files. For installation into an
alternative directory (e.g., /usr/local/lib/lv2/), change the variable `PREFIX` while installing:

```
sudo make install PREFIX=/usr/local
```


## Running

After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Shapr.

If jalv is installed, you can also call it

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BShapr
```

to run it stand-alone and connect it to the JACK system.

Notes:

* **Jack transport is required to get information about beat and bar position (not required for seconds mode)**

## Usage

B.Shapr is an envelope plugin for time or beat position-dependent effects.
The user can define up to four different envelope shapes by drawing Bezier curves. Each of these envelope
shapes can be connected to different target effects, such as amplification, balance, stereo width,
filters, pitch shift, delay, and distortion effecs and can be combined together.

### Shape selector

Select between the four user-defineable shapes.

### Shape editor

Mark and select a node type on the bottom of the editor. Click on the desired position of the shape to set
this node (or to delete it if the deletion tool is marked). Mark a node by clicking on it. You can drag a node
or its handles.

However, dragging the background results only in dragging the whole **display** of the envelope. Similarly,
scrolling zooms the display in or out.

In the background of the shape editor a stereo monitor visualizes the input and the output levels. You can change
the zoom by pressing SHIFT key AND scrolling the mouse wheel.

### Effect

Select a effect that will be applied on the input signal by the use of the shape. You can choose between:

* Level
* Amplification
* Stereo balance
* Stereo width
* Low pass filter (linear or log)
* High pass filter (linear or log)
* Pitch shift
* Delay (const. pitch)
* Doppler delay
* Distortion
* Decimate
* Bitcrush

### MIDI control

B.Shapr can optionally be controlled by a MIDI device. Once switched to B.Shapr's MIDI mode, you can select and
deselect the keys to which B.Shapr shall respond. This takes effect to all four shapers.

### Sequence size

On the bottom of the widget, you can set the lenght of the whole shape sequence between 1 and 16 seconds,
beats or bars. Change the value by dragging, scrolling or clicking on its up and down arrows and select
a base.

### What's new

* Usage simplified
* * No routing in GUI
* * Routing can only be done in LV2 backend now
* New GUI
* Additional configuration parameters for effects (optional dials)
* New effects
* * Distortion
* * * Hardclip
* * * Softclip
* * * Foldback
* * * Overdrive
* * * Fuzz
* * Decimate
* * Bitcrush

## TODO

* Additional effects (any ideas welcome)
* Copy & paste nodes

## See also

* Tutorial: https://youtu.be/fjhL_rku2BU
* Preview: https://www.youtube.com/watch?v=DxYQJ_XJwbU
* Autotune with B.Shapr: https://youtu.be/c6bUW_dTxGg
