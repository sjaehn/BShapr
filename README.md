# B.Shapr
Description: Beat / envelope shaper LV2 plugin

![screenshot](https://raw.githubusercontent.com/sjaehn/BShapr/master/screenshot.png "Screenshot from B.Shapr")


## Installation

Build your own binaries in the following three steps.

Step 1: Clone or download this repository.

Step 2: Install the development packages for x11, cairo, and lv2 if not done yet.

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
* **Don't forget to select an input for each shape you use and an output (see usage/input and usage/output)**

## Usage

+++ See tutorial: https://youtu.be/fjhL_rku2BU +++

B.Shapr is an envelope plugin for time or beat position-dependent effects.
The user can define up to four different envelope shapes by drawing Bezier curves. Each of these envelope
shapes can be connected to different target effects, such as level, balance, or stereo width, and can be
combined together.

### Sequence size

On the bottom of the widget, you can set the lenght of the whole shape sequence between 1 and 16 seconds,
beats or bars. Change the value by dragging, scrolling or clicking on its up and down arrows and select
a base.

### MIDI control

Not active yet.

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

In the background of the shape editor a stereo monitor visualizes the input and the output levels. You can change
the zoom by pressing SHIFT key AND scrolling the mouse wheel.

### Method

Select a method that will be applied on the input signal by the use of the shape. You can choose between:

* Level
* Amplification
* Stereo balance
* Stereo width
* Low pass filter (linear or log)
* High pass filter (linear or log)
* Pitch shift
* Delay (const. pitch)
* Doppler delay

### Output

Select the output for this shape. Mark "audio" if you want to send the output directly to audio out. Otherwise
the signal will only be used internally.

## TODO

* Additional methods (any ideas welcome)
* Display additional configuration parameters for methods
* Copy & paste nodes

## See also

* Tutorial: https://youtu.be/fjhL_rku2BU
* Preview: https://www.youtube.com/watch?v=DxYQJ_XJwbU
* Autotune with B.Shapr: https://youtu.be/c6bUW_dTxGg
