# @author   clemedon (Clément Vidon)
# Modified by Brooke Leinberger
####################################### BEG_3 ####

NAME        := ichor

SUB_DIR		:= $(SRC_DIR)/Ichor
TEST 		:= $(BIN_DIR)/$(NAME)_test
BINS		:= $(BIN_DIR)/$(NAME)

# Compiler options
# CC          := g++
# CPP_FLAGS   := -g -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE
# CPP_LIB 	:= -lpthread
# CPP_INC     := -I include -I$(COMMON_DIR) -I$(SUB_DIR)
# AMQ_LIB 	:= -luuid -lssl -lcrypto -lapr-1 -lactivemq-cpp
# AMQ_INC 	:= -I/usr/include/apr-1.0/ -I/usr/local/include/activemq-cpp-3.10.0/
# FLAGS 		:= $(CFLAGS) $(CPP_FLAGS) $(CPP_LIB) $(CPP_INC) $(AMQ_LIB) $(AMQ_INC)

# Internal sources (ER_V)
SRCS        :=

MAIN_CPP    := main.cpp
SRCS_CPP    := arm/ichor.cpp
SRCS_CPP	+= conf/ichor_conf.cpp

# Automated reformatting
SRCS 		:= $(SRCS:%=$(SUB_DIR)/%)
SRCS_CPP 	:= $(SRCS_CPP:%=$(SUB_DIR)/%)
MAIN_CPP	:= $(MAIN_CPP:%=$(SUB_DIR)/%)

OBJS := $(SRCS:$(SUB_DIR)/%.c=$(MAKE_DIR)/$(OBJ_DIR)/$(SUB_DIR)/%.o)
OBJS += $(SRCS_CPP:$(SUB_DIR)/%.cpp=$(MAKE_DIR)/$(OBJ_DIR)/$(SUB_DIR)/%.o)
MAIN := $(MAIN_CPP:$(SUB_DIR)/%.cpp=$(MAKE_DIR)/$(OBJ_DIR)/$(SUB_DIR)/%.o)

RM        	:= rm -f
DIR_DUP      = mkdir -p $(@D)

all: $(NAME)


# Executable
$(NAME): $(MAIN) $(OBJS)
	$(CC) $(MAIN) $(OBJS) $(CFLAGS) $(FLAGS) -L$(MAKE_DIR)/$(BIN_DIR) -l$(COMMON_LIB) -o $(MAKE_DIR)/$(BIN_DIR)/$(NAME)
	@echo "    Target created: $@"

# Internal source compilation
$(MAKE_DIR)/$(OBJ_DIR)/$(SUB_DIR)/%.o: $(MAKE_DIR)/$(SUB_DIR)/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(FLAGS) -c -o $@ $<

# Internal source compilation
$(MAKE_DIR)/$(OBJ_DIR)/$(SUB_DIR)/%.o: $(MAKE_DIR)/$(SUB_DIR)/%.cpp
	$(DIR_DUP)
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all


#------------------------------------------------#
#   Tests                                        #
#------------------------------------------------#

INT_UTEST_CPP := all_tests.cpp

EXT_UTEST_CPP := tmp/tests/tmp_test.cpp

INT_UTESTS_CPP	:= $(INT_UTEST_CPP:%=$(SUB_DIR)/%)
EXT_UTESTS_CPP	:= $(EXT_UTEST_CPP:%=$(EXT_SRC_DIR)/%)

ERV_UTEST_OBJS		:= $(INT_UTESTS_CPP:$(SUB_DIR)/%.cpp=$(OBJ_DIR)/$(SUB_DIR)/%.o)
ERV_UTEST_OBJS		+= $(EXT_UTESTS_CPP:$(EXT_SRC_DIR)/%.cpp=$(OBJ_DIR)/$(EXT_DIR)/%.o)

test: re $(OBJS) $(ERV_UTEST_OBJS)
	g++ $(ERV_UTEST_OBJS) $(OBJS) -g -o $(TEST) $(FLAGS) -lCppUTest -lCppUTestExt -I$(SUB_DIR) -I$(EXT_SRC_DIR)
	$(TEST)

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re
.SILENT:

####################################### END_3 ####