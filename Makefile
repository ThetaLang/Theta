CXX = g++
RM = rm -f
CPPFLAGS = -std=c++11

TARGET = theta

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CPPFLAGS) $(TARGET).cpp -o $(TARGET)

clean: 
	$(RM) $(TARGET)