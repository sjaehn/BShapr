BUNDLE = BSlicer.lv2
INSTALL_DIR = /usr/lib/lv2
CC = g++
TK = src/BWidgets/DrawingSurface.cpp src/BWidgets/DialWithValueDisplay.cpp src/BWidgets/VSliderWithValueDisplay.cpp src/BWidgets/HSliderWithValueDisplay.cpp src/BWidgets/Dial.cpp src/BWidgets/VSlider.cpp src/BWidgets/HSlider.cpp src/BWidgets/RangeWidget.cpp src/BWidgets/ValueWidget.cpp src/BWidgets/Label.cpp src/BWidgets/Widget.cpp src/BWidgets/BStyles.cpp src/BWidgets/BColors.cpp src/BWidgets/BEvents.cpp src/BWidgets/BValues.cpp src/BWidgets/cairoplus.c src/BWidgets/pugl/pugl_x11.c

$(BUNDLE): clean BSlicer.so BSlicer_GUI.so
	cp manifest.ttl BSlicer.ttl surface.png LICENSE $(BUNDLE)
	
all: $(BUNDLE)

BSlicer.so: ./src/BSlicer.cpp
	mkdir -p $(BUNDLE)
	$(CC) $< -o $(BUNDLE)/$@ -shared -fPIC -DPIC `pkg-config --cflags --libs lv2`
	
BSlicer_GUI.so: ./src/BSlicer_GUI.cpp
	mkdir -p $(BUNDLE)	
	$(CC) $< $(TK) -o $(BUNDLE)/$@ -shared -DPUGL_HAVE_CAIRO -fPIC -DPIC `pkg-config --cflags --libs lv2 x11 cairo`

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)
	
.PHONY: all

clean:
	rm -rf $(BUNDLE)
