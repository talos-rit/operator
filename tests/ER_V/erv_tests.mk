ERV_TEST_EXE := erv_test
ERV_REPORT_DIR := $(REPORT_DIR)/ER_V
ERV_TEST_OBJ_DIR := $(TEST_OBJ_DIR)/ER_V

ERV_UTEST_SRCS := all_tests.cpp
ERV_UTEST_SRCS += erv_conf/test_erv_config.cpp
ERV_UTEST_SRCS += erv_arm/test_erv_arm.cpp
ERV_UTEST_OBJS := $(ERV_UTEST_SRCS:%.cpp=$(ERV_TEST_OBJ_DIR)/%.o)

erv_test_build: erv_test_re $(COMMON_OBJS) $(ERV_OBJS) $(ERV_UTEST_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(filter-out $(PHONIES),$^) $(FLAGS) $(UTEST_LIB) $(COV_FLAGS) -o $(TEST_BIN_DIR)/$(ERV_TEST_EXE)

erv_test: erv_test_build
	$(TEST_BIN_DIR)/$(ERV_TEST_EXE)

erv_test_report: erv_test_build
	@mkdir -p $(ERV_REPORT_DIR)
	cd $(ERV_REPORT_DIR) && ../../../$(TEST_BIN_DIR)/$(ERV_TEST_EXE) -ojunit

erv_test_re: erv_re
	@$(RM) $(ERV_TEST_OBJ_DIR)

PHONIES += erv_test erv_test_report erv_test_re