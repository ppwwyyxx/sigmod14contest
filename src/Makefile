
# $File: Makefile
# $Date: Fri Mar 07 10:11:39 2014 +0800

OBJ_DIR = build
TARGET = ../main

INCLUDE_DIR = -Iinclude
DEFINES = -DDEBUG

OPTFLAGS = -Wall -Wextra -Wconversion -Wno-unused-result
#OPTFLAGS += -g
OPTFLAGS += -O3

CXXFLAGS := $(INCLUDE_DIR)
CXXFLAGS += -fopenmp -pthread -std=c++0x
CXXFLAGS += $(DEFINES) $(OPTFLAGS)
LDFLAGS += -fopenmp -pthread

GCC = gcc
CC = g++
SHELL = bash
ccSOURCES = $(shell find . -name "*.cpp" | sed 's/^\.\///g')
OBJS = $(addprefix $(OBJ_DIR)/,$(ccSOURCES:.cpp=.o))
DEPFILES = $(OBJS:.o=.d)

.PHONY: all clean run

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
	@$(CC) $^ -o $@ $(LDFLAGS) -Wl,--wrap=memcpy
	@echo "done."


clean:
	@rm -rf $(OBJ_DIR)

TEST_DATA = ../data/1k

run: $(TARGET)
	./$(TARGET) $(TEST_DATA) $(TEST_DATA)-queries.txt

# test memory usage
mem: $(TARGET)
	../memusg ./$(TARGET) $(TEST_DATA) $(TEST_DATA)-queries.txt

