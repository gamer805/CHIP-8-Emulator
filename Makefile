CXX := clang++
CXXFLAGS := -std=c++17 -g -Wall
LDFLAGS :=
LIBS := -lSDL2

ifeq ($(shell uname -s),Darwin)
	CXXFLAGS += -I/opt/homebrew/include -I/usr/local/include
	LDFLAGS += -L/opt/homebrew/lib -L/usr/local/lib
endif

TARGET := main
SRCS := main.cpp opcodes.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) opcodes.h
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
