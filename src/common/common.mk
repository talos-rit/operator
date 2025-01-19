# External sources (Common)
COMMON		:= log/log.c
COMMON		+= util/timestamp.c
COMMON		+= data/s_list.c
COMMON		+= api/api.c

COMMON_CPP	:= tamq/tamq_sub.cpp
COMMON_CPP	+= socket/socket.cpp
COMMON_CPP	+= sub/sub.cpp
COMMON_CPP	+= arm/arm.cpp
COMMON_CPP	+= conf/config.cpp
COMMON_CPP	+= log/log_config.cpp
COMMON_CPP	+= tamq/tamq_conf.cpp
COMMON_CPP	+= tmp/tmp.cpp

UTEST_CPP := tmp/tests/tmp_test.cpp

ODIR := $(OBJ_DIR)/common
COMMON_OBJS := $(COMMON:%.c=$(ODIR)/%.o)
COMMON_OBJS += $(COMMON_CPP:%.cpp=$(ODIR)/%.o)
COMMON_UTEST_OBJS := $(UTEST_CPP:%.cpp=$(ODIR)/%.o)