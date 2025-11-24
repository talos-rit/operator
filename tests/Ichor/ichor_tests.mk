ICHOR_TEST_EXE := ichor_test
ICHOR_REPORT_DIR := $(REPORT_DIR)/Ichor
ICHOR_TEST_OBJ_DIR := $(TEST_OBJ_DIR)/Ichor

ICHOR_UTEST_SRCS := all_tests.cpp
ICHOR_UTEST_SRCS += arm/test_ichor_arm.cpp
ICHOR_UTEST_SRCS += conf/test_ichor_conf.cpp
ICHOR_UTEST_OBJS := $(ICHOR_UTEST_SRCS:%.cpp=$(ICHOR_TEST_OBJ_DIR)/%.o)

ichor_test_build: ichor_test_re $(COMMON_OBJS) $(ICHOR_OBJS) $(ICHOR_UTEST_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(filter-out $(PHONIES),$^) $(FLAGS) $(UTEST_LIB) $(COV_FLAGS) -o $(TEST_BIN_DIR)/$(ICHOR_TEST_EXE)

ichor_test: ichor_test_build
	$(TEST_BIN_DIR)/$(ICHOR_TEST_EXE)

ichor_test_report: ichor_test_build
	@mkdir -p $(ICHOR_REPORT_DIR)
	cd $(ICHOR_REPORT_DIR) && ../../../$(TEST_BIN_DIR)/$(ICHOR_TEST_EXE) -ojunit

ichor_test_re: ichor_re
	@$(RM) $(ICHOR_TEST_OBJ_DIR)

PHONIES += ichor_test ichor_test_report ichor_test_re