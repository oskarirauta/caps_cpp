all: world

CXX?=g++
CXXFLAGS?=--std=c++17 -Wall -fPIC -g

OBJS_LIST_CAPS:= \
	objs/list_caps.o

OBJS_SET_CAPS:= \
	objs/set_caps.o

OBJS_SET_USER:= \
	objs/set_user.o

OBJS_SHOW_CAPS:= \
	objs/show_caps.o

CAPS_DIR:=.
include $(CAPS_DIR)/Makefile.inc

world: list_caps set_caps set_user show_caps

$(shell mkdir -p objs)

objs/set_user.o: set_user.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/set_caps.o: set_caps.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/list_caps.o: list_caps.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/show_caps.o: show_caps.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

list_caps: $(OBJS_LIST_CAPS) $(CAPS_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

set_caps: $(OBJS_SET_CAPS) $(CAPS_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

set_user: $(OBJS_SET_USER) $(CAPS_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

show_caps: $(OBJS_SHOW_CAPS) $(CAPS_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

.PHONY: clean
clean:
	@rm -rf objs list_caps set_caps set_user show_caps
