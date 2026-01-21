COMMON_DIR  = common


COMMON      := log/log.c
COMMON      += util/timestamp.c
COMMON      += data/s_list.c

COMMON_CPP  := api/api.cpp
COMMON_CPP  += socket/socket.cpp
COMMON_CPP  += sub/sub.cpp
COMMON_CPP  += arm/arm.cpp
COMMON_CPP  += conf/config.cpp
COMMON_CPP  += log/log_config.cpp
COMMON_CPP  += socket/socket_conf.cpp

ODIR        := $(OBJ_DIR)/common
COMMON_OBJS := $(COMMON:%.c=$(ODIR)/%.o)
COMMON_OBJS += $(COMMON_CPP:%.cpp=$(ODIR)/%.o)

PHONIES     += common_re

common_re:
	@$(RM) $(OBJ_DIR)/$(COMMON_DIR)
