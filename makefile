SHELL = /bin/sh

PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2

PKG_CONFIG ?= pkg-config
CXX ?= g++
CPPFLAGS += -DPIC
CXXFLAGS += -std=c++11 -fvisibility=hidden -fPIC
LDFLAGS += -shared

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
FILES = manifest.ttl BShapr.ttl surface.png Shape1.png Shape2.png Shape3.png Shape4.png LICENSE
B_FILES = $(addprefix $(BUNDLE)/, $(FILES))

GUI_INCL = \
	src/SelectWidget.cpp \
	src/ValueSelect.cpp \
	src/DownClick.cpp \
	src/UpClick.cpp \
	src/ShapeWidget.cpp \
	src/BWidgets/DrawingSurface.cpp \
	src/BWidgets/PopupListBox.cpp \
	src/BWidgets/ListBox.cpp \
	src/BWidgets/ChoiceBox.cpp \
	src/BWidgets/ItemBox.cpp \
	src/BWidgets/Text.cpp \
	src/BWidgets/UpButton.cpp \
	src/BWidgets/DownButton.cpp \
	src/BWidgets/Button.cpp \
	src/BWidgets/DisplayDial.cpp \
	src/BWidgets/Dial.cpp \
	src/BWidgets/DisplayVSlider.cpp \
	src/BWidgets/VSlider.cpp \
	src/BWidgets/VScale.cpp \
	src/BWidgets/HSwitch.cpp \
	src/BWidgets/DisplayHSlider.cpp \
	src/BWidgets/HSlider.cpp \
	src/BWidgets/HScale.cpp \
	src/BWidgets/RangeWidget.cpp \
	src/BWidgets/ValueWidget.cpp \
	src/BWidgets/Knob.cpp \
	src/BWidgets/ImageIcon.cpp \
	src/BWidgets/Label.cpp \
	src/BWidgets/FocusWidget.cpp \
	src/BWidgets/Window.cpp \
	src/BWidgets/Widget.cpp \
	src/BWidgets/BStyles.cpp \
	src/BWidgets/BColors.cpp \
	src/BWidgets/BEvents.cpp \
	src/BWidgets/BValues.cpp \
	src/screen.c \
	src/BWidgets/cairoplus.c \
	src/BWidgets/pugl/pugl_x11_cairo.c \
	src/BWidgets/pugl/pugl_x11.c

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
	@cp $(FILES) $(BUNDLE)

all: $(BUNDLE)

$(DSP_OBJ): $(DSP_SRC)
	@echo -n Build $(BUNDLE) DSP...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DSPFLAGS)
	@echo \ done.

$(GUI_OBJ): $(GUI_SRC)
	@echo -n Build $(BUNDLE) GUI...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< $(GUI_INCL) -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(GUIFLAGS)
	@echo \ done.

install:
	@echo -n Install $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@install -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@install -m755 $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@install -m644 $(B_FILES) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)
	@echo \ done.

.PHONY: all

clean:
	@rm -rf $(BUNDLE)
