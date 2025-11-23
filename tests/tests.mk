TESTS_DIR	 = tests
TEST_OBJ_DIR := $(OBJ_DIR)/tests
TEST_BIN_DIR := $(BIN_DIR)/tests
REPORT_DIR	:= build/reports

RM := rm -rf
DIR_DUP = mkdir -p $(@D)

# Patterns for test object files
# Overriding some rules to add --coverage flag
# C
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(DIR_DUP)
	@$(CC) $(FLAGS) -c --coverage -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"

# C++
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(DIR_DUP)
	@$(CC) $(FLAGS) -c --coverage -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"

# C
$(TEST_OBJ_DIR)/%.o: $(TESTS_DIR)/%.c
	$(DIR_DUP)
	@$(CC) $(FLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "  TEST CC     $@"

# C++
$(TEST_OBJ_DIR)/%.o: $(TESTS_DIR)/%.cpp
	$(DIR_DUP)
	@$(CC) $(FLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "  TEST CC     $@"

# Run all tests
test_all: test_common test_erv test_ichor

test_all_report: test_common_report test_erv_report test_ichor_report

test_re: common_test_re erv_test_re ichor_test_re
	@$(RM) $(TEST_OBJ_DIR)

PHONIES += test_erv test_all test_re test_all_report

.PHONY: test_erv test_all test_re test_all_report

include tests/common/common_tests.mk
include tests/ER_V/erv_tests.mk
include tests/Ichor/ichor_tests.mk