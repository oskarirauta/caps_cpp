CAPS_DIR?=./caps
INCLUDES += -I$(CAPS_DIR)/include

CAPS_OBJS:= \
	objs/cap-ng_lookup_table.o \
	objs/cap-ng_cap-ng.o \
	objs/caps_cap.o \
	objs/caps_caps.o \
	objs/caps_caps_set.o

ifdef DROP_AMBIENT
	CAPS_OBJS += \
		objs/cap-ng_libdrop_ambient.o
endif




objs/cap-ng_lookup_table.o: $(CAPS_DIR)/src/cap-ng/lookup_table.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/cap-ng_libdrop_ambient.o: $(CAPS_DIR)/src/cap-ng/libdrop_ambient.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/cap-ng_cap-ng.o: $(CAPS_DIR)/src/cap-ng/cap-ng.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/caps_cap.o: $(CAPS_DIR)/src/cap.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/caps_caps.o: $(CAPS_DIR)/src/caps.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/caps_caps_set.o: $(CAPS_DIR)/src/caps_set.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;
