# @author   clemedon (Clément Vidon)
# Modified by Brooke Leinberger
####################################### BEG_3 ####

NAME        := erv
ERV_DIR			:= ER_V

# C sources
SRCS        := acl/acl.c

# C++ sources
MAIN_CPP    := main.cpp
SRCS_CPP    := erv_arm/erv.cpp
SRCS_CPP	+= erv_conf/erv_conf.cpp

# Object reformatting
ERV_OBJS 		:= $(SRCS:%.c=$(OBJ_DIR)/$(ERV_DIR)/%.o)
ERV_OBJS 		+= $(SRCS_CPP:%.cpp=$(OBJ_DIR)/$(ERV_DIR)/%.o)
ERV_MAIN 		:= $(MAIN_CPP:%.cpp=$(OBJ_DIR)/$(ERV_DIR)/%.o)

RM          := rm -rf
# MAKEFLAGS   += --no-print-directory
DIR_DUP      = mkdir -p $(@D)
PHONIES 	+= erv erv_test erv_re

erv_print:
	$(info PHONIES: )

# Executable
erv: $(COMMON_OBJS) $(ERV_OBJS) $(ERV_MAIN)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(FLAGS) -o $(BIN_DIR)/$@
	@echo "    Target    $@"

erv_re:
	@$(RM) $(OBJ_DIR)/$(ERV_DIR)


#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re
.SILENT:

####################################### END_3 ####