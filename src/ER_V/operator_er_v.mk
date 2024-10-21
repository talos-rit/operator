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
CC          := g++
CFLAGS      := -g -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE -lpthread
CPPFLAGS    := -I include -I$(INT_SRC_DIR) -I$(EXT_SRC_DIR)

AMQ_CFLAGS 	:= -luuid -lssl -lcrypto -lapr-1
AMQ_INC 	:= -I/usr/include/apr-1.0/ -I/usr/local/include/activemq-cpp-3.10.0/ -lactivemq-cpp

# Internal sources (ER_V)
SRCS        :=
SRCS_CPP    := main.cpp
SRCS_CPP    += erv_arm/erv.cpp

# External sources (Common)
EXTS		:= log/log.c
EXTS		+= util/timestamp.c
EXTS		+= data/s_list.c
EXTS		+= api/api.c
EXTS_CPP	:= tamq/tamq.cpp
EXTS_CPP	+= sub/sub.cpp
EXTS_CPP	+= arm/arm.cpp
# EXTS_CPP	:= 

# Automated reformatting
SRCS 		:= $(SRCS:%=$(INT_SRC_DIR)/%)
SRCS_CPP 	:= $(SRCS_CPP:%=$(INT_SRC_DIR)/%)
EXTS 		:= $(EXTS:%=$(EXT_SRC_DIR)/%)
EXTS_CPP 	:= $(EXTS_CPP:%=$(EXT_SRC_DIR)/%)

OBJS := $(SRCS:$(INT_SRC_DIR)/%.c=$(OBJ_DIR)/$(SUB_DIR)/%.o)
OBJS += $(SRCS_CPP:$(INT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(SUB_DIR)/%.o)
OBJS += $(EXTS:$(EXT_SRC_DIR)/%.c=$(OBJ_DIR)/$(EXT_DIR)/%.o)
OBJS += $(EXTS_CPP:$(EXT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(EXT_DIR)/%.o)

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
	$(CC) $(OBJS) -o $(BIN_DIR)/$(NAME) -lactivemq-cpp

# Internal source compilation
$(OBJ_DIR)/$(SUB_DIR)/%.o: $(INT_SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -g -c -o $@ $<

# External source compilation
$(OBJ_DIR)/$(EXT_DIR)/%.o: $(EXT_SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -g -c -o $@ $<

# Internal source compilation
$(OBJ_DIR)/$(SUB_DIR)/%.o: $(INT_SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(AMQ_CFLAGS) $(CPPFLAGS) $(AMQ_INC) -g -c -o $@ $<

# External source compilation
$(OBJ_DIR)/$(EXT_DIR)/%.o: $(EXT_SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(AMQ_CFLAGS) $(CPPFLAGS) $(AMQ_INC) -g -c -o $@ $<

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)

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