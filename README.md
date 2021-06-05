# B.Shapr
Description: Beat / envelope shaper LV2 plugin

![screenshot](https://raw.githubusercontent.com/sjaehn/BShapr/master/doc/screenshot.png "Screenshot from B.Shapr")


## Installation

a) Install the bshapr package for your system
* [Arch](https://git.archlinux.org/svntogit/community.git/tree/trunk?h=packages/bshapr)
* [NixOS](https://github.com/NixOS/nixpkgs/blob/master/pkgs/applications/audio/bshapr/default.nix) by Bart Brouns
* [openSUSE](https://software.opensuse.org/package/BShapr)
* [Ubuntu](https://packages.ubuntu.com/source/hirsute/bshapr) by Erich Eickmeyer, trebmuh
* Check https://repology.org/project/bshapr/versions for other systems

b) Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BShapr/releases) of B.Shapr. Or clone or
[download the master](https://github.com/sjaehn/BShapr/archive/master.zip) of this repository.

Step 2: Install pkg-config and the development packages for x11, cairo, and lv2 if not done yet. If you
don't have already got the build tools (compilers, make, libraries) then install them too.

On Debian-based systems you may run:
```
sudo apt-get install build-essential
sudo apt-get install pkg-config libx11-dev libcairo2-dev lv2-dev
```

On Arch-based systems you may run:
```
sudo pacman -S base-devel
sudo pacman -S pkg-config libx11 cairo lv2
```

Step 3: Building and installing into the default lv2 directory (/usr/local/lib/lv2/) is easy using `make` and
`make install`. Simply call:
```
make
sudo make install
```

**Optional:** Standard `make` and `make install` parameters are supported. Compiling using `make CPPFLAGS+=-O3`
is recommended to improve the plugin performance. Alternatively, you may build a debugging version using
`make CPPFLAGS+=-g`. For installation into an alternative directory (e.g., /usr/lib/lv2/), change the
variable `PREFIX` while installing: `sudo make install PREFIX=/usr`. If you want to freely choose the
install target directory, change the variable `LV2DIR` (e.g., `make install LV2DIR=~/.lv2`).


## Running

After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Shapr.

If jalv is installed, you can also call it using one of the graphical jalv executables (like
jalv.gtk, or jalv.gtk3, or jalv.qt4, or jalv.qt5, depending on what is installed), like

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BShapr
```

to run it stand-alone and connect it to the JACK system.

If you are interested in the CV version of this plugin call

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BShapr-cv
```

Notes:

* **Jack transport is required to get information about beat and bar position (not required for seconds mode)**

## Usage

B.Shapr is an envelope plugin for time or beat position-dependent effects.
The user can define up to four different envelope shapes by drawing Bezier curves. Each of these envelope
shapes can be connected to different target effects, such as amplification, balance, stereo width,
filters, pitch shift, delay, and distortion effects and can be combined together.

### Global

Press the bypass button to bypass the plugin. The dry/wet dial in the top right of the plugin GUI sets the global
mixing ratio of the plugin input : plugin output.

### Shape selector

Select the shape to edit. You can choose between up to four user-definable shapes. You can remove (-) shapes if not
required or add (+) new shapes (max. 4).

**In the GUI**, the audio input signals are routed through the shapers in the order of their numbers to the audio
output. This means:

```
audio in > shape1 > (shape2 > (shape3 > (shape4))) > audio out.
```

**Deprecated:** Complex non-linear routing of the shapers by manual change of "shx_input" and "shx_output" in the
LV2 backend will not be supported in future versions and will be removed until the first official release of B.Shapr.

**Deprecated:** Constant value input will not be supported in future versions and will be removed until the first
official release of B.Shapr. Think about to use the "send" effects instead.

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
Bezier node, symmetric Bezier node, and asymmetric Bezier node.

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
* Send (shape to CV out or MIDI CC)

### MIDI control

B.Shapr can optionally be controlled by a MIDI device. Once switched to B.Shapr's MIDI trigger mode, you can select
and deselect the keys to which B.Shapr shall respond. This takes effect to all four shapers.

The MIDI Thru option allows to forward an incoming MIDI signal. Be careful not to create dead loops in combination
with MIDI IN!

### Sequence size

On the bottom of the widget, you can set the length of the whole shape sequence between 1 and 16 seconds,
beats or bars. Change the value by dragging, scrolling or clicking on its up and down arrows and select
a base.

## What's new

* Smooth parameters instead of shape

## TODO

* Additional effects (any ideas welcome)
* Report latency (pitch shifter)

## See also

* Tutorial: https://youtu.be/fjhL_rku2BU
* Preview: https://www.youtube.com/watch?v=DxYQJ_XJwbU
* Autotune with B.Shapr: https://youtu.be/c6bUW_dTxGg
