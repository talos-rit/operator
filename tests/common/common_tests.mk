COMMON_TEST_EXE := common_test
COMMON_REPORT_DIR := $(REPORT_DIR)/common
COMMON_TEST_OBJ_DIR := $(TEST_OBJ_DIR)/common

COMMON_UTEST_SRCS := conf/test_config_sys.cpp
COMMON_UTEST_SRCS += conf/test_config_parsing.cpp
COMMON_UTEST_SRCS += data/s_list_test.cpp
COMMON_UTEST_SRCS += log/test_log_config.cpp
COMMON_UTEST_SRCS += socket/test_socket_config.cpp
COMMON_UTEST_SRCS += all_tests.cpp 
COMMON_UTEST_OBJS := $(COMMON_UTEST_SRCS:%.cpp=$(COMMON_TEST_OBJ_DIR)/%.o)


test_common_build: test_common_re $(COMMON_OBJS) $(COMMON_UTEST_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(filter-out $(PHONIES),$^) $(FLAGS) $(UTEST_LIB) $(COV_FLAGS) -o $(TEST_BIN_DIR)/$(COMMON_TEST_EXE)

test_common: test_common_build
	$(TEST_BIN_DIR)/$(COMMON_TEST_EXE)

test_common_report: test_common_build
	@mkdir -p $(COMMON_REPORT_DIR)
	cd $(COMMON_REPORT_DIR) && ../../../$(TEST_BIN_DIR)/$(COMMON_TEST_EXE) -ojunit
# TODO: Add single test report aggregation here

test_common_re: common_re
	@$(RM) $(COMMON_TEST_OBJ_DIR)

PHONIES += test_common test_common_build test_common_report test_common_re