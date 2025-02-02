TARGET = a.out

SRCS = src/main.cpp src/boardmanage.cpp src/gamelogic.cpp

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
