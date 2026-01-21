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
DIR_DUP      = mkdir -p $(@D)
PHONIES 	+= all ichor_re clean fclean



# Executable
ichor: $(COMMON_OBJS) $(ICHOR_OBJS) $(ICHOR_MAIN)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(FLAGS) -o $(BIN_DIR)/$@
	@echo "    Target    $@"

dummy: $(COMMON_OBJS) $(ICHOR_OBJS) $(SRC_DIR)/$(ICHOR_DIR)/dummy_main.cpp
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(FLAGS) -o $(BIN_DIR)/$@
	@echo "    Target    $@"

analyze_ichor: 
	@echo "Creating compile_commands.json for Ichor..."
	make clean
	@bear --output $(SRC_DIR)/$(ICHOR_DIR)/compile_commands.json -- $(MAKE) ichor

	@echo "Analyzing Ichor with cppcheck..."
	@cppcheck --enable=all --inconclusive --project=src/Ichor/compile_commands.json --language=c++ --platform=unix64 2> ichor_cppcheck.txt
	@echo "Ichor cppcheck analysis complete. Results saved to ichor_cppcheck.txt"
	
	@echo "Analyzing Ichor with clang-tidy..."
	@run-clang-tidy -p src/Ichor/ -quiet > ichor_clang_tidy.txt
	@echo "Ichor clang-tidy analysis complete. Results saved to ichor_clang_tidy.txt"

ichor_re: fclean

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: clean fclean re
.SILENT:

####################################### END_3 ####