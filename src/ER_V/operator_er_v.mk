# @author   clemedon (Clément Vidon)
# Modified by Brooke Leinberger
####################################### BEG_3 ####

NAME        := erv

#------------------------------------------------#
#   INGREDIENTS                                  #
#------------------------------------------------#
# INT_SRC_DIR   source directory
# OBJ_DIR   object directory
# SRCS      source files
# OBJS      object files
#
# CC        compiler
# CFLAGS    compiler flags
# CPPFLAGS  preprocessor flags

SUB_DIR		:= src/ER_V
EXT_DIR 	:= src/common

INT_SRC_DIR := $(SUB_DIR)
EXT_SRC_DIR := $(EXT_DIR)
OBJ_DIR     := build/obj
BIN_DIR		:= build/bin
TEST 		:= $(BIN_DIR)/$(NAME)_test
BINS		:= $(BIN_DIR)/$(NAME)

# Compiler options
CC          := gcc
CFLAGS      := -g -Wall -Wextra -Werror -std=c11 -D _DEFAULT_SOURCE
CPPFLAGS    := -I include -I$(INT_SRC_DIR) -I$(EXT_SRC_DIR) -lpthread

# Internal sources (ChariotNode)
SRCS        := log_demo.c
# SRCS        += tmp/tmp.c

# External sources (Common)
EXTS		:= log/log.c
EXTS		+= util/timestamp.c
EXTS		+= data/s_list.c
# EXTS		+= tamq/tamq.cpp

# Automated reformatting
SRCS := $(SRCS:%=$(INT_SRC_DIR)/%)
EXTS := $(EXTS:%=$(EXT_SRC_DIR)/%)
OBJS := $(SRCS:$(INT_SRC_DIR)/%.c=$(OBJ_DIR)/$(SUB_DIR)/%.o)
OBJS += $(EXTS:$(EXT_SRC_DIR)/%.c=$(OBJ_DIR)/$(EXT_DIR)/%.o)
# OBJS_CPP := $(SRCS:$(INT_SRC_DIR)/%.cpp=$(OBJ_DIR)/CPP/$(SUB_DIR)/%.o)
# OBJS_CPP += $(EXTS:$(EXT_SRC_DIR)/%.cpp=$(OBJ_DIR)/CPP/$(EXT_DIR)/%.o)

#------------------------------------------------#
#   UTENSILS                                     #
#------------------------------------------------#
# RM        force remove
# MAKEFLAGS make flags
# DIR_DUP   duplicate directory tree

RM          := rm -f
# MAKEFLAGS   += --no-print-directory
DIR_DUP     = mkdir -p $(@D)

#------------------------------------------------#
#   RECIPES                                      #
#------------------------------------------------#
# all       default goal
# $(NAME)   linking .o -> binary
# %.o       compilation .c -> .o
# clean     remove .o
# fclean    remove .o + binary
# re        remake default goal

all: $(NAME)

# Executable
$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(BIN_DIR)/$(NAME)

# Internal source compilation
$(OBJ_DIR)/$(SUB_DIR)/%.o: $(INT_SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -g -c -o $@ $<

# External source compilation
$(OBJ_DIR)/$(EXT_DIR)/%.o: $(EXT_SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)
	mkdir -p $(BIN_DIR)
	# mkdir -p $(OBJ_DIR)/C/$(SUB_DIR) $(OBJ_DIR)/C/$(EXT_DIR)
	# mkdir -p $(OBJ_DIR)/CPP/$(SUB_DIR) $(OBJ_DIR)/CPP/$(EXT_DIR)

re:
	$(MAKE) fclean
	$(MAKE) all


#------------------------------------------------#
#   Tests                                        #
#------------------------------------------------#

# CPPFLAGS += -I$(CPPUTEST_HOME)/include
# CXXFLAGS += -include $(CPPUTEST_HOME)/include/CppUTest/MemoryLeakDetectorNewMacros.h
# CFLAGS += -include $(CPPUTEST_HOME)/include/CppUTest/MemoryLeakDetectorMallocMacros.h
# LD_LIBRARIES = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt
# TEST_MAIN = $(INT_SRC_DIR)/all_tests.cpp

INT_UTEST := all_tests.c
INT_UTEST += tmp/tests/tmp_test.c
INT_UTEST += tmp/tmp.c

EXT_UTEST := log/log.c
EXT_UTEST += util/timestamp.c
EXT_UTEST += data/s_list.c
EXT_UTEST += data/s_list_test.c

UTEST := $(INT_UTEST:%=$(SUB_DIR)/src/%)
UTEST += $(EXT_UTEST:%=$(EXT_DIR)/src/%)

test: re
	g++ $(UTEST) -g -o $(TEST) -lCppUTest -lCppUTestExt -I$(INT_SRC_DIR) -I$(EXT_SRC_DIR)
	$(TEST)

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re
.SILENT:



####################################### END_3 ####