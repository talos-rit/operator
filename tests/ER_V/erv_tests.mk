ERV_TEST_EXE := erv_test
ERV_REPORT_DIR := $(REPORT_DIR)/erv

# all_tests has to be last since for some reason it prints logs twice otherwise
ERV_UTEST_SRCS := ER_V/all_tests.cpp
ERV_UTEST_OBJS := $(ERV_UTEST_SRCS:$(TESTS_DIR)/%.cpp=$(TEST_OBJ_DIR)/ER_V/%.o)

erv_test_build: erv_test_re $(COMMON_OBJS) $(ERV_OBJS) $(ERV_UTEST_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(filter-out $(PHONIES),$^) $(FLAGS) $(UTEST_LIB) --coverage -o $(TEST_BIN_DIR)/$(ERV_TEST_EXE)

erv_test: erv_test_build
	$(TEST_BIN_DIR)/$(ERV_TEST_EXE)

erv_test_report: erv_test_build
	@mkdir -p $(ERV_REPORT_DIR)
	cd $(ERV_REPORT_DIR) && ../../../$(TEST_BIN_DIR)/$(ERV_TEST_EXE) -ojunit

erv_test_re: erv_re
	@$(RM) $(TEST_OBJ_DIR)/erv

PHONIES += erv_test erv_test_report erv_test_re