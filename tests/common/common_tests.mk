COMMON_TEST_EXE := common_test
COMMON_REPORT_DIR := $(REPORT_DIR)/common

COMMON_UTEST_SRCS := common/conf/test_config_sys.cpp
COMMON_UTEST_SRCS += common/conf/test_config_parsing.cpp
COMMON_UTEST_SRCS += common/data/s_list_test.cpp
COMMON_UTEST_SRCS += common/log/test_log_config.cpp
COMMON_UTEST_SRCS += common/socket/test_socket_config.cpp
COMMON_UTEST_SRCS += common/all_tests.cpp 
COMMON_UTEST_OBJS := $(COMMON_UTEST_SRCS:%.cpp=$(TEST_OBJ_DIR)/%.o)


common_test_build: common_test_re $(COMMON_OBJS) $(COMMON_UTEST_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(filter-out $(PHONIES),$^) $(FLAGS) $(UTEST_LIB) --coverage -o $(TEST_BIN_DIR)/$(COMMON_TEST_EXE)

common_test: common_test_build
	$(TEST_BIN_DIR)/$(COMMON_TEST_EXE)

common_test_report: common_test_build
	@mkdir -p $(COMMON_REPORT_DIR)
	cd $(COMMON_REPORT_DIR) && ../../../$(TEST_BIN_DIR)/$(COMMON_TEST_EXE) -ojunit
# TODO: Add single test report aggregation here

common_test_re: common_re
	@$(RM) $(TEST_OBJ_DIR)/common

PHONIES += common_test common_test_report common_test_re