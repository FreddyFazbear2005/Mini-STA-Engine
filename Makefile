CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic

TARGET = sta 

SRCS = main.cpp \
			 TimingGraph.cpp \
			 STAEngine.cpp \
			 NetlistParser.cpp \
			 Cell.cpp \
			 LibraryParser.cpp

OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< 

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
