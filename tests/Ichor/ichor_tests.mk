ICHOR_TEST_EXE := ichor_test
ICHOR_REPORT_DIR := $(REPORT_DIR)/ichor

# all_tests has to be last since for some reason it prints logs twice otherwise
ICHOR_UTEST_SRCS := Ichor/all_tests.cpp
ICHOR_UTEST_OBJS := $(ICHOR_UTEST_SRCS:$(TESTS_DIR)/%.cpp=$(TEST_OBJ_DIR)/Ichor/%.o)

ichor_test_build: ichor_test_re $(COMMON_OBJS) $(ICHOR_OBJS) $(ICHOR_UTEST_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(filter-out $(PHONIES),$^) $(FLAGS) $(UTEST_LIB) --coverage -o $(TEST_BIN_DIR)/$(ICHOR_TEST_EXE)

ichor_test: ichor_test_build
	$(TEST_BIN_DIR)/$(ICHOR_TEST_EXE)

ichor_test_report: ichor_test_build
	@mkdir -p $(ICHOR_REPORT_DIR)
	cd $(ICHOR_REPORT_DIR) && ../../../$(TEST_BIN_DIR)/$(ICHOR_TEST_EXE) -ojunit

ichor_test_re: ichor_re
	@$(RM) $(TEST_OBJ_DIR)/ichor

PHONIES += ichor_test ichor_test_report ichor_test_re