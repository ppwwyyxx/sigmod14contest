# $File: Makefile
# $Date: Wed Apr 16 11:23:08 2014 +0800

OBJ_DIR = build
TARGET = ./main

BUILD ?= develop

INCLUDE_DIR = -Iinclude -isystem third-party

DEFINES += -DGOOGLE_HASH
#DEFINES += -DNUM_THREADS=1		# to disable multi-threading

OPTFLAGS = -Wno-unused-result -Wno-unused-local-typedefs

#OPTFLAGS += -g -O0 -fsanitize=address
OPTFLAGS += -O3
#LDFLAGS += -lprofiler

MALLOC = $(addprefix third-party/, libtcmalloc.a libunwind.a liblzma.a)
#MALLOC = $(addprefix third-party/, libllalloc.a)

LIBS = $(MALLOC)

ifeq ($(BUILD), submit)
OPTFLAGS = -O3
#OPTFLAGS += -fnothrow-opt -fno-rtti -march=native -mfpmath=sse -Ofast -flto -funroll-loops
LDFLAGS += -Wl,--wrap=memcpy
LDFLAGS += -static-libstdc++
DEFINES += -DNUM_THREADS=4
else
DEFINES += -DDEBUG
#LDFLAGS += -lprofiler
OPTFLAGS += -Wall -Wextra -Wconversion
endif

CONCURFLAGS = -fopenmp -pthread
CXXFLAGS = $(INCLUDE_DIR)
CXXFLAGS += -std=c++0x $(CONCURFLAGS)
CXXFLAGS += $(DEFINES) $(OPTFLAGS)
LDFLAGS += $(CONCURFLAGS) $(OPTFLAGS)


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
	@#echo "[cpp] $< ..."
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)/memcpy.o: lib/memcpy.c
	@#echo "[memcpy.o] ..."
	@$(CC) -c $< -o $@


$(OBJ_DIR)/%.d: %.cpp  Makefile
	@mkdir -p $(dir $@)
	@#echo "[dep] $< ..."
	@$(CXX) $(CXXFLAGS) $(INCLUDE_DIR) $(DEFINES) -MM -MT "$(OBJ_DIR)/$(<:.cpp=.o) $(OBJ_DIR)/$(<:.cpp=.d)" "$<"  > "$@"


#$(TARGET): $(OBJS)
$(TARGET): $(OBJS) $(OBJ_DIR)/memcpy.o
	@#echo "Linking ..."
	@$(CXX) $^ $(LIBS) -o $@ $(LDFLAGS)
	@#echo "done."

$(OBJ_DIR)/gen_query.o: gen_query.cc
	@#echo "[gen_query.o] ..."
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)/mem_monitor.o: mem_monitor.cc
	@#echo "[mem_monitor.o] ..."
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

gen_query: $(filter-out $(OBJ_DIR)/main.o $(OBJ_DIR)/query2.o, $(OBJS)) $(OBJ_DIR)/gen_query.o
	@#echo "Linking gen_query"
	@$(CXX) $^ -o $@ $(LDFLAGS)
	@#echo "done."

mem_monitor: $(OBJ_DIR)/mem_monitor.o $(OBJ_DIR)/lib/utils.o $(OBJ_DIR)/lib/Timer.o $(OBJ_DIR)/lib/debugutils.o
	@#echo "Linking mem_monitor"
	@$(CXX) $^ -o $@ $(LDFLAGS)
	@#echo "done."


clean:
	@rm -rf $(OBJ_DIR) gen_query

rebuild:
	@+make clean
	@+make

TEST_DATA = ../data/1k

run: $(TARGET)
	./$(TARGET) $(TEST_DATA) $(TEST_DATA)-queries.txt
