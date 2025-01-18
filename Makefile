

MAKE_DIR = $(PWD)
BIN_DIR		:= build/bin
OBJ_DIR		:= build/obj
SRC_DIR		:= src

COMMON_DIR	:= $(SRC_DIR)/common
ERV_DIR    	:= $(SRC_DIR)/ER_V
ICHOR_DIR  	:= $(SRC_DIR)/Ichor

COMMON_LIB 	:= operator_common

# LIB_SRCH_PATH :=
# LIB_SRCH_PATH += -L$(MAKE_DIR)/libs

CC          := g++
CPP_FLAGS   := -g -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE
CPP_LIB 	:= -lpthread

LD = ld
LIBS := -lpthread

CFLAGS 	:=
CFLAGS 	+= $(INC_SRCH_PATH) $(LIB_SRCH_PATH)
CFLAGS 	+= -g -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE
CFLAGS	+= -I$(MAKE_DIR)/$(COMMON_DIR) -I$(MAKE_DIR)/$(ERV_DIR) -I$(MAKE_DIR)/$(ICHOR_DIR)

AMQ_LIB 	:= -luuid -lssl -lcrypto -lapr-1 -lactivemq-cpp
AMQ_INC 	:= -I/usr/include/apr-1.0/ -I/usr/local/include/activemq-cpp-3.10.0/
FLAGS 		:= $(CFLAGS) $(CPP_FLAGS) $(CPP_LIB) $(CPP_INC) $(AMQ_LIB) $(AMQ_INC)
CFLAGS 		+= $(AMQ_LIB) $(AMQ_INC) $(FLAGS)
# CFLAGS += -D_REENTRANT

LDFLAGS :=
RM = rm -rf

export MAKE_DIR BIN_DIR OBJ_DIR SRC_DIR COMMON_DIR COMMON_LIB LIB_SRCH_PATH CC LD CFLAGS LDFLAGS LIBS LINT INC_SRCH_PATH


common:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)
	@$(MAKE) -C $(COMMON_DIR) -f common.mk

erv: common
	@$(MAKE) -C $(ERV_DIR) -f erv.mk

.PHONY: clean
clean:
	$(RM) $(MAKE_DIR)/build/
