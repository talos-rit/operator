TESTS_DIR     = tests
TEST_OBJ_DIR := $(OBJ_DIR)/tests
TEST_BIN_DIR := $(BIN_DIR)/tests
REPORT_DIR   := build/reports

COV_FLAGS    := --coverage -lgcov

RM           := rm -rf

# Patterns for test object files
# Conditionally overriding some rules to add --coverage flag when building tests
ifneq (,$(findstring test, $(MAKECMDGOALS)))
# C
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -c $(COV_FLAGS) -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"

# C++
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -c $(COV_FLAGS) -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"
endif

# C
$(TEST_OBJ_DIR)/%.o: $(TESTS_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "  TEST CC     $@"

# C++
$(TEST_OBJ_DIR)/%.o: $(TESTS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "  TEST CC     $@"

# Run all tests
test_all: common_test erv_test ichor_test

test_all_report: common_test_report erv_test_report ichor_test_report cov_report

# Create HTML code coverage report
cov_report:
	lcov --capture --directory build/obj/ --output-file coverage.info --exclude '*/12/*' --exclude '/usr/include/*' --exclude '*/tests/*'
	genhtml coverage.info --output-directory coverage

test_re: common_test_re erv_test_re ichor_test_re
	@$(RM) $(TEST_OBJ_DIR)

PHONIES += test_all test_re test_all_report

.PHONY: test_all test_re test_all_report

include tests/common/common_tests.mk
include tests/ER_V/erv_tests.mk
include tests/Ichor/ichor_tests.mk
