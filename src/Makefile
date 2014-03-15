# $File: Makefile
# $Date: Sat Mar 15 10:03:34 2014 +0800

OBJ_DIR = build
TARGET = ./main

BUILD ?= develop

INCLUDE_DIR = -Iinclude -Ithird-party
DEFINES += -DGOOGLE_HASH
DEFINES += -DUSE_THREAD

OPTFLAGS = -Wno-unused-result -Wno-unused-local-typedefs
#OPTFLAGS += -g -O0
OPTFLAGS += -O3

ifeq ($(BUILD), submit)
OPTFLAGS = -O3
DEFINES += -DUSE_THREAD
else
DEFINES += -DDEBUG
LDFLAGS += -lprofiler
OPTFLAGS += -Wall -Wextra -Wconversion
endif

CONCURFLAGS = -fopenmp -pthread
CXXFLAGS = $(INCLUDE_DIR)
CXXFLAGS += -std=c++0x $(CONCURFLAGS)
CXXFLAGS += $(DEFINES) $(OPTFLAGS)
LDFLAGS += $(CONCURFLAGS)


CC = gcc
CXX ?= g++
SHELL = bash
ccSOURCES = $(shell find . -name "*.cpp" | sed 's/^\.\///g')
OBJS = $(addprefix $(OBJ_DIR)/,$(ccSOURCES:.cpp=.o))
DEPFILES = $(OBJS:.o=.d)

.PHONY: all clean run rebuild

all: $(TARGET)

sinclude $(DEPFILES)

$(OBJ_DIR)/%.o: %.cpp
	@echo "[cpp] $< ..."
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)/memcpy.o: lib/memcpy.c
	@echo "[memcpy.o] ..."
	@$(CC) -c $< -o $@


$(OBJ_DIR)/%.d: %.cpp  Makefile
	@mkdir -p $(dir $@)
	@echo "[dep] $< ..."
	@$(CXX) $(CXXFLAGS) $(INCLUDE_DIR) $(DEFINES) -MM -MT "$(OBJ_DIR)/$(<:.cpp=.o) $(OBJ_DIR)/$(<:.cpp=.d)" "$<"  > "$@"


#$(TARGET): $(OBJS) $(OBJ_DIR)/memcpy.o
$(TARGET): $(OBJS)
	@echo "Linking ..."
	@$(CXX) $^ -o $@ $(LDFLAGS)
	@echo "done."


clean:
	@rm -rf $(OBJ_DIR)

rebuild:
	@+make clean
	@+make

TEST_DATA = ../data/1k

run: $(TARGET)
	./$(TARGET) $(TEST_DATA) $(TEST_DATA)-queries.txt

# test memory usage
mem: $(TARGET)
	../memusg ./$(TARGET) $(TEST_DATA) $(TEST_DATA)-queries.txt

