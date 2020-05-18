# B.Shapr
Description: Beat / envelope shaper LV2 plugin

![screenshot](https://raw.githubusercontent.com/sjaehn/BShapr/master/screenshot.png "Screenshot from B.Shapr")


## Installation

a) Install the bshapr package for your system
* [NixOS](https://github.com/NixOS/nixpkgs/blob/master/pkgs/applications/audio/bshapr/default.ni) by Bart Brouns
* [Debian](https://kx.studio/Repositories:Plugins) by falkTX
* Check https://repology.org/project/bshapr/versions for other systems

b) Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BShapr/releases) of B.Shapr. Or clone or
[download the master](https://github.com/sjaehn/BShapr/archive/master.zip) of this repository.

Step 2: Install pkg-config and the development packages for x11, cairo, and lv2 if not done yet. On
Debian-based systems you may run:
```
sudo apt-get install pkg-config libx11-dev libcairo2-dev lv2-dev
```

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:
```
make
sudo make install
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

### Global

Press the bypass button to bypass the plugin. The dry/wet dial in the top right of the plugin GUI sets the global
mixing ratio of the plugin input : plugin output.

### Shape selector

Select the shape to edit. You can choose between up to four user-defineable shapes. You can remove (-) shapes if not
required or add (+) new shapes (max. 4).

**In the GUI**, the audio input signals are routed through the shapers in the order of their numbers to the audio
output. This means:

```
audio in > shape1 > (shape2 > (shape3 > (shape4))) > audio out.
```

**In the lv2 backend**, you can also build more complex audio routes. You have to follow the rules:

* You can only route in forward direction, shape2 > shape1 will not work
* But you can skip shapes
* sh1_input must be audio in
* All other shape inputs can also take audio in or the output of any previous shape
* At least the last used shape must send to audio out
* All other shape outputs can also directly send to audio out (or to internal)

An example for complex routing would be:

```
audio in > shape1 > shape3 ─┐
      ╰──> shape2 ──────────┤
               ╰──> shape4 ─┴> audio out
```

**Warning: Complex routing made in lv2 backend will be simplified if you add or delete shapes in GUI!**

### Shape editor

Mark and select a node type on the bottom of the editor. Click on the desired position of the shape to set
this node. Mark a node by clicking on it again (or use the selection tool, see below). You can drag a node
(or multiple nodes, if selected) or its handles.

However, dragging the background results only in (vertical) dragging the whole **display** of the envelope. Similarly,
scrolling zooms the display in or out.

In the background of the shape editor a stereo monitor visualizes the input and the output levels. You can change
the zoom by pressing SHIFT key AND scrolling the mouse wheel.

### Toolbar

The toolobar is located below the shape editor. It is divided into five sections.

In section 1 you will find **node tools** to select nodes or to set different types of nodes. Selection of existing nodes
can be done by clicking on a node or by selection of an area. Node types provided are point node, auto smooth
Bezier node, symmetic Bezier node, and asymmetic Bezier node.

Section 2 consists of **edit tools** to apply on selected nodes. This are cut, copy, paste, and delete.

In Section 3 are **shape tools** to reset the shape, and to undo or redo the last edit(s).

Section 4 only consists of the **smooth dial**. This dial sets the smoothing time (in milliseconds). If smoothing is
set (> 0 ms), the shape signal will be linearly smoothed over the given time before applying to the audio signal. The
smoothing time is also shown in the monitor by the thickness of the white horizon line. A long
smoothing time may effectively prevent clicks but also may compensate fast changes. A smoothing value of 20 ms is a
good starting point.

Section 5 are the **grid tools**. You can show the grid and/or snap to the grid.

### Effects

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

## What's new

* Bypass
* Dry / wet mixing
* Send shape to MIDI CC (experimental)

## TODO

* Additional effects (any ideas welcome)
* Report latency (pitch shifter)

## See also

* Tutorial: https://youtu.be/fjhL_rku2BU
* Preview: https://www.youtube.com/watch?v=DxYQJ_XJwbU
* Autotune with B.Shapr: https://youtu.be/c6bUW_dTxGg
