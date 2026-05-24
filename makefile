CXX      = g++
CXXFLAGS = -std=c++11 -Wall -O2
LIBS     = -lglfw -lGLEW -lGL -lassimp

# GLM: prefer the system package (apt install libglm-dev);
# fall back to the copy that CMake FetchContent downloaded.
GLM_SYSTEM := $(shell find /usr/include/glm -name glm.hpp 2>/dev/null | head -1)
GLM_LOCAL  := $(shell find out/build -name glm.hpp 2>/dev/null | head -1)

ifneq ($(GLM_SYSTEM),)
  # System install found — no extra -I needed
else ifneq ($(GLM_LOCAL),)
  # Strip the trailing /glm/glm.hpp to get the directory that CONTAINS glm/.
  GLM_DIR   := $(patsubst %/glm/glm.hpp,%,$(GLM_LOCAL))
  CXXFLAGS  += -I$(GLM_DIR)
  $(info NOTE: using bundled GLM at $(GLM_DIR). Run: sudo apt install libglm-dev)
else
  $(error GLM not found. Run: sudo apt install libglm-dev)
endif

# Every .cpp in the root directory (excluding the old Windows stub).
ROOT_SRCS  := $(filter-out Test.cpp, $(wildcard *.cpp))

# Shape template .cpp files are #included directly by their .h headers
# (C++ template instantiation requires the definition to be visible at the
# call site). Compiling them as separate translation units causes duplicate
# symbol errors. Only non-template Shape files need separate compilation.
# Check: grep -l '#include.*\.cpp"' Shapes/*.h lists all header-included ones.
SHAPE_SRCS := Shapes/light.cpp

SRCS       := $(ROOT_SRCS) $(SHAPE_SRCS)

OBJS   := $(SRCS:.cpp=.o)
TARGET := MiniGolf

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Build successful: ./$(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Shapes/%.o: Shapes/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)