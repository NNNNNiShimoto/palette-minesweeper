TARGET = a.out

SRCS = main.cpp

OBJS = $(SRCS:.cpp=.o)

CXX = g++
CXXFLAGS = -std=c++14 -O2 #-Wall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
