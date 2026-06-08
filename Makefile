CXX := clang++
CXXFLAGS := -std=c++17 -g -Wall
LDFLAGS :=
LIBS := -lSDL2

ifeq ($(shell uname -s),Darwin)
	CXXFLAGS += -I/opt/homebrew/include -I/usr/local/include
	LDFLAGS += -L/opt/homebrew/lib -L/usr/local/lib
	SDL2_PREFIX := $(shell brew --prefix sdl2 2>/dev/null || echo /opt/homebrew/opt/sdl2)
endif

TARGET := nano8
SRCS := main.cpp opcodes.cpp

APP := NANO8.app
APP_EXEC := $(APP)/Contents/MacOS/$(TARGET)
APP_FRAMEWORKS := $(APP)/Contents/Frameworks
SDL2_DYLIB := libSDL2-2.0.0.dylib

.PHONY: all run app clean

all: $(TARGET)

$(TARGET): $(SRCS) opcodes.h
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET) $(LIBS)

app: $(TARGET)
	cp $(TARGET) $(APP_EXEC)
	mkdir -p $(APP_FRAMEWORKS)
	rm -f $(APP_FRAMEWORKS)/$(SDL2_DYLIB)
	cp $(SDL2_PREFIX)/lib/$(SDL2_DYLIB) $(APP_FRAMEWORKS)/
	install_name_tool -id @executable_path/../Frameworks/$(SDL2_DYLIB) $(APP_FRAMEWORKS)/$(SDL2_DYLIB)
	install_name_tool -change $(SDL2_PREFIX)/lib/$(SDL2_DYLIB) @executable_path/../Frameworks/$(SDL2_DYLIB) $(APP_EXEC)
	codesign --force --sign - $(APP_FRAMEWORKS)/$(SDL2_DYLIB)
	codesign --force --sign - $(APP_EXEC)
	codesign --force --deep --sign - $(APP)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
