# @author   clemedon (Clément Vidon)
# Modified by Brooke Leinberger
####################################### BEG_3 ####

NAME        := erv

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
CPP_FLAGS   := -g -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE
CPP_LIB 	:= -lpthread
CPP_INC     := -I include -I$(INT_SRC_DIR) -I$(EXT_SRC_DIR)
AMQ_LIB 	:= -luuid -lssl -lcrypto -lapr-1 -lactivemq-cpp
AMQ_INC 	:= -I/usr/include/apr-1.0/ -I/usr/local/include/activemq-cpp-3.10.0/
FLAGS 		:= $(CPP_FLAGS) $(CPP_LIB) $(CPP_INC) $(AMQ_LIB) $(AMQ_INC)

# Internal sources (ER_V)
SRCS        := acl/acl.c

MAIN_CPP    := main.cpp
SRCS_CPP    := erv_arm/erv.cpp
SRCS_CPP	+= erv_conf/erv_conf.cpp

# External sources (Common)
EXTS		:= log/log.c
EXTS		+= util/timestamp.c
EXTS		+= data/s_list.c
EXTS		+= api/api.c

EXTS_CPP	:= tamq/tamq_sub.cpp
EXTS_CPP	+= socket/socket.cpp
EXTS_CPP	+= sub/sub.cpp
EXTS_CPP	+= arm/arm.cpp
EXTS_CPP	+= conf/config.cpp
EXTS_CPP	+= log/log_config.cpp
EXTS_CPP	+= tamq/tamq_conf.cpp
EXTS_CPP	+= tmp/tmp.cpp

# Automated reformatting
SRCS 		:= $(SRCS:%=$(INT_SRC_DIR)/%)
EXTS 		:= $(EXTS:%=$(EXT_SRC_DIR)/%)
SRCS_CPP 	:= $(SRCS_CPP:%=$(INT_SRC_DIR)/%)
MAIN_CPP	:= $(MAIN_CPP:%=$(INT_SRC_DIR)/%)
EXTS_CPP 	:= $(EXTS_CPP:%=$(EXT_SRC_DIR)/%)

OBJS := $(SRCS:$(INT_SRC_DIR)/%.c=$(OBJ_DIR)/$(SUB_DIR)/%.o)
OBJS += $(EXTS:$(EXT_SRC_DIR)/%.c=$(OBJ_DIR)/$(EXT_DIR)/%.o)
OBJS += $(SRCS_CPP:$(INT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(SUB_DIR)/%.o)
OBJS += $(EXTS_CPP:$(EXT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(EXT_DIR)/%.o)
MAIN := $(MAIN_CPP:$(INT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(SUB_DIR)/%.o)


RM          := rm -f
# MAKEFLAGS   += --no-print-directory
DIR_DUP     = mkdir -p $(@D)

all: $(NAME)

# Executable
$(NAME): $(MAIN) $(OBJS)
	$(CC) $(MAIN) $(OBJS) $(FLAGS) -o $(BIN_DIR)/$(NAME)

# Internal source compilation
$(OBJ_DIR)/$(SUB_DIR)/%.o: $(INT_SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(FLAGS) -c -o $@ $<

# External source compilation
$(OBJ_DIR)/$(EXT_DIR)/%.o: $(EXT_SRC_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(FLAGS) -c -o $@ $<

# Internal source compilation
$(OBJ_DIR)/$(SUB_DIR)/%.o: $(INT_SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(FLAGS) -c -o $@ $<

# External source compilation
$(OBJ_DIR)/$(EXT_DIR)/%.o: $(EXT_SRC_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(FLAGS) -c -o $@ $<

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

INT_UTEST_CPP := all_tests.cpp

EXT_UTEST_CPP := tmp/tests/tmp_test.cpp

INT_UTESTS_CPP	:= $(INT_UTEST_CPP:%=$(INT_SRC_DIR)/%)
EXT_UTESTS_CPP	:= $(EXT_UTEST_CPP:%=$(EXT_SRC_DIR)/%)

UTEST_OBJS		:= $(INT_UTESTS_CPP:$(INT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(SUB_DIR)/%.o)
UTEST_OBJS		+= $(EXT_UTESTS_CPP:$(EXT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(EXT_DIR)/%.o)

test: re $(OBJS) $(UTEST_OBJS)
	g++ $(UTEST_OBJS) $(OBJS) -g -o $(TEST) $(FLAGS) -lCppUTest -lCppUTestExt -I$(INT_SRC_DIR) -I$(EXT_SRC_DIR)
	$(TEST)

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re
.SILENT:

####################################### END_3 ####