
# $File: Makefile
# $Date: Mon Mar 03 11:29:42 2014 +0800

OBJ_DIR = build
TARGET = main

INCLUDE_DIR = -Iinclude
DEFINES = -DDEBUG

OPTFLAGS = -Wall -Wextra -Wconversion -Wno-unused-result
#OPTFLAGS += -g
OPTFLAGS += -O3

CXXFLAGS = $(INCLUDE_DIR)
CXXFLAGS += -fopenmp  -std=c++11
CXXFLAGS += $(DEFINES) $(OPTFLAGS)
LDFLAGS += -fopenmp

CC = g++
SHELL = bash
ccSOURCES = $(shell find -name "*.cpp" | sed 's/^\.\///g')
OBJS = $(addprefix $(OBJ_DIR)/,$(ccSOURCES:.cpp=.o))
DEPFILES = $(OBJS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

sinclude $(DEPFILES)

$(OBJ_DIR)/%.o: %.cpp
	@echo "[cpp] $< ..."
	@$(CC) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)/%.d: %.cpp  Makefile
	@mkdir -p $(dir $@)
	@echo "[dep] $< ..."
	@$(CC) $(CXXFLAGS) $(INCLUDE_DIR) $(DEFINES) -MM -MT "$(OBJ_DIR)/$(<:.cpp=.o) $(OBJ_DIR)/$(<:.cpp=.d)" "$<"  > "$@"


$(TARGET): $(OBJS)
	@echo "Linking ..."
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "done."


clean:
	@rm -rf $(OBJ_DIR) $(TARGET)

TEST_DATA = ../1k

run: $(TARGET)
	./$(TARGET) $(TEST_DATA) $(TEST_DATA)-queries.txt

# test memory usage
mem: $(TARGET)
	../memusg ./$(TARGET) $(TEST_DATA) $(TEST_DATA)-queries.txt

