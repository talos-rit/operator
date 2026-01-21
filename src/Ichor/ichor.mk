NAME        := ichor
ICHOR_DIR	:= Ichor
DRIVER_DIR	 = $(ICHOR_DIR)/driver

include $(SRC_DIR)/$(DRIVER_DIR)/driver.mk

FLAGS += -I$(SRC_DIR)/$(ICHOR_DIR)

# Main file (separated to not interfere with CPPUTEST)
MAIN_CPP    := main.cpp

# C sources
SRCS        :=

# C++ sources
ICHOR_CPP   := arm/ichor_arm.cpp
ICHOR_CPP	+= conf/ichor_conf.cpp

# Object reformatting
ICHOR_OBJS		:= $(DRIVER_OBJS)
ICHOR_OBJS 		+= $(SRCS:%.c=$(OBJ_DIR)/$(ICHOR_DIR)/%.o)
ICHOR_OBJS 		+= $(ICHOR_CPP:%.cpp=$(OBJ_DIR)/$(ICHOR_DIR)/%.o)
ICHOR_MAIN 		:= $(MAIN_CPP:%.cpp=$(OBJ_DIR)/$(ICHOR_DIR)/%.o)

RM          := rm -rf
# MAKEFLAGS   += --no-print-directory
PHONIES 	+= all ichor_re clean fclean



# Executable
$(BIN_DIR)/ichor: $(COMMON_OBJS) $(ICHOR_OBJS) $(ICHOR_MAIN)
	@mkdir -p $(dir $@)
	$(CC) $^ $(FLAGS) -o $@
	@echo "    Target    $@"

$(BIN_DIR)/ichor_test: ichor_re $(COMMON_OBJS) $(COMMON_UTEST_OBJS) $(ICHOR_OBJS) $(ICHOR_UTEST_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(filter-out $(PHONIES),$^) $(FLAGS) $(UTEST_LIB) -o $@
	$@

$(BIN_DIR)/dummy: $(COMMON_OBJS) $(ICHOR_OBJS) $(SRC_DIR)/$(ICHOR_DIR)/dummy_main.cpp
	@mkdir -p $(dir $@)
	$(CC) $^ $(FLAGS) -o $@
	@echo "    Target    $@"

ichor_re: fclean

ichor: $(BIN_DIR)/ichor
ichor_test: $(BIN_DIR)/ichor_test
dummy: $(BIN_DIR)/dummy

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re
.SILENT:

####################################### END_3 ####
