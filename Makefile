TARGET = bf_vm

SRCS  = $(shell find . -type f -name "*.cpp")
HEADS = $(shell find . -type f -name "*.hpp")
OBJS = $(SRCS:.cpp=.o)

CXXFLAGS = -O3 -Wall -Wextra -std=c++20 $(INCLUDES)
LDFLAGS = -lm

all: clean $(TARGET)

$(TARGET): $(OBJS) $(HEADS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

run: all
	@./$(TARGET)

.PHONY: depend clean
depend:
	$(CXX) $(INCLUDES) -MM $(SRCS) > $(DEPS)
	@sed -i -E "s/^(.+?).o: ([^ ]+?)\1/\2\1.o: \2\1/g" $(DEPS)

clean:
	$(RM) $(OBJS) $(TARGET)

-include $(DEPS)