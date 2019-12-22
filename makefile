SHELL = /bin/sh

PKG_CONFIG ?= pkg-config
CXX ?= g++
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL) -m644
STRIP ?= strip

PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2

CPPFLAGS += -DPIC
CXXFLAGS += -std=c++11 -fvisibility=hidden -fPIC
LDFLAGS += -shared -Wl,-z,relro,-z,now
STRIPFLAGS += -s --strip-program=$(STRIP)

DSPFLAGS =
GUIFLAGS = -DPUGL_HAVE_CAIRO

DSPFLAGS += `$(PKG_CONFIG) --cflags --libs lv2`
GUIFLAGS += `$(PKG_CONFIG) --cflags --libs lv2 x11 cairo`

BUNDLE = BShapr.lv2
DSP = BShapr
DSP_SRC = ./src/BShapr.cpp
GUI = BShaprGUI
GUI_SRC = ./src/BShaprGUI.cpp
OBJ_EXT = .so
DSP_OBJ = $(DSP)$(OBJ_EXT)
GUI_OBJ = $(GUI)$(OBJ_EXT)
B_OBJECTS = $(addprefix $(BUNDLE)/, $(DSP_OBJ) $(GUI_OBJ))
ROOTFILES = \
	manifest.ttl \
	BShapr.ttl \
	LICENSE

INCFILES = \
	inc/surface.png \
	inc/Shape1.png \
	inc/Shape2.png \
	inc/Shape3.png \
	inc/Shape4.png \
	inc/Level.png \
	inc/Balance.png \
	inc/Width.png \
	inc/Low_pass.png \
	inc/High_pass.png \
	inc/Amplify.png \
	inc/Low_pass_log.png \
	inc/High_pass_log.png \
	inc/Pitch_shift.png \
	inc/Delay.png \
	inc/Doppler_delay.png \
	inc/Distortion.png \
	inc/Decimate.png \
	inc/Bitcrush.png

B_FILES = $(addprefix $(BUNDLE)/, $(ROOTFILES) $(INCFILES))

GUI_INCL = \
	src/MonitorWidget.cpp \
	src/SelectWidget.cpp \
	src/ValueSelect.cpp \
	src/DownClick.cpp \
	src/UpClick.cpp \
	src/ShapeWidget.cpp \
	src/BWidgets/MessageBox.cpp \
	src/BWidgets/TextButton.cpp \
	src/BWidgets/HPianoRoll.cpp \
	src/BWidgets/PianoWidget.cpp \
	src/BWidgets/DrawingSurface.cpp \
	src/BWidgets/PopupListBox.cpp \
	src/BWidgets/ListBox.cpp \
	src/BWidgets/ChoiceBox.cpp \
	src/BWidgets/ItemBox.cpp \
	src/BWidgets/Text.cpp \
	src/BWidgets/UpButton.cpp \
	src/BWidgets/DownButton.cpp \
	src/BWidgets/Button.cpp \
	src/BWidgets/DialValue.cpp \
	src/BWidgets/Dial.cpp \
	src/BWidgets/VSwitch.cpp \
	src/BWidgets/VSlider.cpp \
	src/BWidgets/VScale.cpp \
	src/BWidgets/RangeWidget.cpp \
	src/BWidgets/ValueWidget.cpp \
	src/BWidgets/Knob.cpp \
	src/BWidgets/ImageIcon.cpp \
	src/BWidgets/Icon.cpp \
	src/BWidgets/Label.cpp \
	src/BWidgets/Window.cpp \
	src/BWidgets/Widget.cpp \
	src/BWidgets/BStyles.cpp \
	src/BWidgets/BColors.cpp \
	src/BWidgets/BItems.cpp \
	src/screen.c \
	src/BWidgets/cairoplus.c \
	src/BWidgets/pugl/pugl_x11_cairo.c \
	src/BWidgets/pugl/pugl_x11.c \
	src/BUtilities/to_string.cpp

ifeq ($(shell $(PKG_CONFIG) --exists lv2 || echo no), no)
  $(error LV2 not found. Please install LV2 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists x11 || echo no), no)
  $(error X11 not found. Please install X11 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists cairo || echo no), no)
  $(error Cairo not found. Please install cairo first.)
endif

$(BUNDLE): clean $(DSP_OBJ) $(GUI_OBJ)
	@cp $(ROOTFILES) $(BUNDLE)
	@mkdir -p $(BUNDLE)/inc
	@cp $(INCFILES) $(BUNDLE)/inc

all: $(BUNDLE)

$(DSP_OBJ): $(DSP_SRC)
	@echo -n Build $(BUNDLE) DSP...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< -o $(BUNDLE)/$@ -g $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DSPFLAGS)
	@echo \ done.

$(GUI_OBJ): $(GUI_SRC)
	@echo -n Build $(BUNDLE) GUI...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< $(GUI_INCL) -o $(BUNDLE)/$@ -g $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(GUIFLAGS)
	@echo \ done.

install:
	@echo -n Install $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)/inc
	@$(INSTALL_PROGRAM) -m755 $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(addprefix $(BUNDLE)/, $(ROOTFILES)) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(addprefix $(BUNDLE)/, $(INCFILES)) $(DESTDIR)$(LV2DIR)/$(BUNDLE)/inc
	@echo \ done.

install-strip:
	@echo -n "Install (stripped)" $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)/inc
	@$(INSTALL_PROGRAM) -m755 $(STRIPFLAGS) $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(addprefix $(BUNDLE)/, $(ROOTFILES)) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(addprefix $(BUNDLE)/, $(INCFILES)) $(DESTDIR)$(LV2DIR)/$(BUNDLE)/inc
	@echo \ done.

uninstall:
	@echo -n Uninstall $(BUNDLE)...
	@rm -f $(addprefix $(DESTDIR)$(LV2DIR)/$(BUNDLE)/, $(ROOTFILES) $(INCFILES))
	-@rmdir $(DESTDIR)$(LV2DIR)/$(BUNDLE)/inc
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(GUI_OBJ)
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(DSP_OBJ)
	-@rmdir $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@echo \ done.

clean:
	@rm -rf $(BUNDLE)

.PHONY: all install install-strip uninstall clean
