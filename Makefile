# include src/ER_4U/operator_er4u.mk
# include src/ActiveMQ_Demo/demo.mk

MAKE_DIR	 = $(PWD)
OBJ_DIR     := build/obj
BIN_DIR		:= build/bin
SRC_DIR		:= src

# Compiler options
CC          := g++
CPP_FLAGS   := -g -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE
CPP_LIB 	:= -lpthread
CPP_INC     := -I./src/common -I./src/ER_V
AMQ_LIB 	:= -luuid -lssl -lcrypto -lapr-1 -lactivemq-cpp
AMQ_INC 	:= -I/usr/include/apr-1.0/ -I/usr/local/include/activemq-cpp-3.10.0/
FLAGS 		:= $(CPP_FLAGS) $(CPP_LIB) $(CPP_INC) $(AMQ_LIB) $(AMQ_INC)
UTEST_LIB 	:= -lCppUTest -lCppUTestExt

.PHONY: clean
clean:
	@rm -rf build/

# C
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(DIR_DUP)
	@$(CC) $(FLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"

# C++
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(DIR_DUP)
	@$(CC) $(FLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"


include src/common/common.mk
include src/ER_V/erv.mk