CXX      = clang++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -I/opt/homebrew/include
LIBS     = -L/opt/homebrew/lib -lraylib \
           -framework OpenGL -framework Cocoa -framework IOKit

sim: main.cpp Vec3.h Body.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) main.cpp -o sim $(LIBS)

clean:
	rm -f sim

.PHONY: clean
