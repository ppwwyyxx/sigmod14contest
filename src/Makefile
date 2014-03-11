
# $File: Makefile
# $Date: Tue Mar 11 10:46:43 2014 +0800
# $Date: Tue Mar 11 10:46:43 2014 +0800

OBJ_DIR = build
TARGET = ../main

INCLUDE_DIR = -Iinclude -Ithird-party
DEFINES = -DDEBUG -DGOOGLE_DENSE_HASH

OPTFLAGS = -Wall -Wextra -Wconversion -Wno-unused-result -Wno-unused-local-typedefs
# OPTFLAGS += -g -O0
OPTFLAGS += -O3

CONCURFLAGS = -fopenmp -pthread
CXXFLAGS := $(INCLUDE_DIR)
CXXFLAGS += -std=c++0x $(CONCURFLAGS)
CXXFLAGS += $(DEFINES) $(OPTFLAGS)
LDFLAGS += $(CONCURFLAGS) -Wl,--wrap=memcpy -lprofiler

GCC = gcc
CC = g++
SHELL = bash
ccSOURCES = $(shell find . -name "*.cpp" | sed 's/^\.\///g')
OBJS = $(addprefix $(OBJ_DIR)/,$(ccSOURCES:.cpp=.o))
DEPFILES = $(OBJS:.o=.d)

.PHONY: all clean run rebuild

all: $(TARGET)

sinclude $(DEPFILES)

$(OBJ_DIR)/%.o: %.cpp
	@echo "[cpp] $< ..."
	@$(CC) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)/memcpy.o: lib/memcpy.c
	@echo "[memcpy.o] ..."
	@$(GCC) -c $< -o $@


$(OBJ_DIR)/%.d: %.cpp  Makefile
	@mkdir -p $(dir $@)
	@echo "[dep] $< ..."
	@$(CC) $(CXXFLAGS) $(INCLUDE_DIR) $(DEFINES) -MM -MT "$(OBJ_DIR)/$(<:.cpp=.o) $(OBJ_DIR)/$(<:.cpp=.d)" "$<"  > "$@"


$(TARGET): $(OBJS) $(OBJ_DIR)/memcpy.o
	@echo "Linking ..."
	@$(CC) $^ -o $@ $(LDFLAGS)
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

