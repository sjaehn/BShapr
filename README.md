# B.Shapr
Description: Beat / envelope shaper LV2 plugin

Installation
------------
Build your own binaries in the following three steps.

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
After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Shapr.

If jalv is installed, you can also call it

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BShapr
```

to run it stand-alone and connect it to the JACK system. 

Notes: 

* **Jack transport is required to get information about beat / position**

Usage
-----
... coming soon!




