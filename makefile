BUNDLE = BSlicer.lv2
INSTALL_DIR = /usr/lib/lv2


$(BUNDLE): clean BSlicer.so BSlicer_GUI.so
	cp manifest.ttl BSlicer.ttl surface.jpeg LICENSE $(BUNDLE)
	
all: $(BUNDLE)

BSlicer.so: ./src/BSlicer.cpp
	mkdir -p $(BUNDLE)
	rm -f $(BUNDLE)/$@
	g++ $< -o $(BUNDLE)/$@ -shared -fPIC -DPIC `pkg-config --cflags --libs lv2`
	
BSlicer_GUI.so: ./src/BSlicer_GUI.cpp
	mkdir -p $(BUNDLE)	
	rm -f $(BUNDLE)/$@
	g++ $< -o $(BUNDLE)/$@ -shared -fPIC -DPIC `pkg-config --cflags --libs lv2 gtk+-2.0`

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)
	rm -rf $(BUNDLE)
	
.PHONY: all

clean:
	rm -rf $(BUNDLE)
