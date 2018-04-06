
-include ../local.mk
ifneq ($(MAKECMDGOALS),clean)
ifndef CPPUTEST_PATH
$(error Requred variable `CPPUTEST_PATH` not defined. Set `CPPUTEST_PATH` in local.mk)
endif
endif

CXX = g++
CXXFLAGS += --std=c++11 -Wall -Wextra -pedantic -I../src
LDFLAGS +=
DEBUG_CXXFLAGS = -g -O0 -DDEBUG

TARGET = all
SRC = main.cpp pf_test.cpp pf_pq_test.cpp
TEST_OBJ := $(SRC:.cpp=.o)
DEP := $(SRC:.cpp=.d)

# object files from ../src
OBJDIR = ../src
OBJ = $(OBJDIR)/polyfuse.o $(OBJDIR)/pf_pq.o $(OBJDIR)/pf_accum.o \
	  $(OBJDIR)/pf_topic.o $(OBJDIR)/util.o

.PHONY: test_all
test_all: $(TARGET)

$(TARGET): CXXFLAGS := $(CXXFLAGS) $(DEBUG_CXXFLAGS)
$(TARGET): $(OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(TEST_OBJ) $(DEP)

-include $(DEP)
