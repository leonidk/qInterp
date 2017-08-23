DEBUG := 0

# Detect OS and CPU
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
machine := $(shell sh -c "$(CC) -dumpmachine || echo unknown")

CXX := clang++
CXXFLAGS := -std=c++14 -Wno-narrowing -Iinclude
ifeq ($(DEBUG),0)
CXXFLAGS += -Ofast -march=native
else
CXXFLAGS += -O0 -g
endif

ifeq ($(uname_S),Darwin)
CXXFLAGS += -I/usr/local/include
GLFW3_FLAGS := -lglfw -framework OpenGL
else
GLFW3_FLAGS := `pkg-config --cflags --libs glfw3 gl`
endif

LDFLAGS := -lpthread -lrealsense
INCLUDES := $(wildcard include/*.hpp)
INCLUDES += $(wildcard include/*.h)
OBJECTS  = $(notdir $(basename $(wildcard */*.cpp)))
BINARIES := $(addprefix bin/, $(OBJECTS))
OBJECTS := $(addprefix obj/, $(addsuffix .o, $(OBJECTS)))

all: bin/qInterp

clean:
	rm -rf obj
	rm -rf bin

obj bin:
	mkdir -p bin
	mkdir -p obj

obj/%.o: */%.cpp $(INCLUDES) | obj
	$(CXX) $< $(CXXFLAGS) -c -o $@

bin/qInterp: obj/imshow.o obj/Main.o | bin
	$(CXX) $^ $(GLFW3_FLAGS) $(LDFLAGS) -o $@
