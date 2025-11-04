CXX = g++
CXXFLAGS = -O2 -std=c++14 -Wall
TARGET = code
SOURCES = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) *.o *.dat

.PHONY: all clean
