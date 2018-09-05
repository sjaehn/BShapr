BUNDLE = BSlicer.lv2
INSTALL_DIR = /usr/lib/lv2


$(BUNDLE): BSlicer.so BSlicer_GUI.so manifest.ttl BSlicer.ttl surface.jpeg LICENCE
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp $^ $(BUNDLE)
	
all: $(BUNDLE)

BSlicer.so: ./src/BSlicer.cpp
	rm -f $@
	g++ $< -o $@ -shared -fPIC -DPIC `pkg-config --cflags --libs lv2`
	
BSlicer_GUI.so: ./src/BSlicer_GUI.cpp	
	rm -f $@
	g++ $< -o $@ -shared -fPIC -DPIC `pkg-config --cflags --libs lv2 gtkmm-2.4`

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)
	
.PHONY: all

clean:
	rm -rf $(BUNDLE) BSlicer.so
