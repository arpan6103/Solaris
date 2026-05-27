CXX      = clang++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -I/opt/homebrew/include
LIBS     = -L/opt/homebrew/lib -lraylib \
           -framework OpenGL -framework Cocoa -framework IOKit

SRCS = main.cpp Physics.cpp Renderer.cpp Camera.cpp
OBJS = $(SRCS:.cpp=.o)

sim: $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OBJS) -o sim $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f sim $(OBJS)

.PHONY: clean