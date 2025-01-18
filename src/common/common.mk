CPP_INC     := -I include -I$(SUB_DIR) -I$(COMMON_DIR)

# External sources (Common)
EXTS		:= log/log.c
EXTS		+= util/timestamp.c
EXTS		+= data/s_list.c
EXTS		+= api/api.c

EXTS_CPP	:= tamq/tamq_sub.cpp
EXTS_CPP	+= socket/socket.cpp
EXTS_CPP	+= sub/sub.cpp
EXTS_CPP	+= arm/arm.cpp
EXTS_CPP	+= conf/config.cpp
EXTS_CPP	+= log/log_config.cpp
EXTS_CPP	+= tamq/tamq_conf.cpp
EXTS_CPP	+= tmp/tmp.cpp

ODIR := $(OBJ_DIR)/common
OBJS := $(EXTS:%.c=$(ODIR)/%.o)
OBJS += $(EXTS_CPP:%.cpp=$(ODIR)/%.o)

ABS_OBJS := $(OBJS:%=$(MAKE_DIR)/%)

RM          := rm -f
DIR_DUP     = mkdir -p $(MAKE_DIR)/$(@D)

NAME = $(MAKE_DIR)/$(BIN_DIR)/lib$(COMMON_LIB).a

$(COMMON_LIB): $(OBJS)
	$(DIR_DUP)
	@$(AR) crs $(NAME) $(ABS_OBJS)
	@echo "    Archive    $(notdir $@)"

# External source compilation
$(OBJ_DIR)/%.o: $(MAKE_DIR)/$(SRC_DIR)/%.c
	$(DIR_DUP)
	@$(CC) $(CFLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"

# External source compilation
$(OBJ_DIR)/%.o: $(MAKE_DIR)/$(SRC_DIR)/%.cpp
	$(DIR_DUP)
	@$(CC) $(CFLAGS) -c -o $(MAKE_DIR)/$@ $<
	@echo "    CC        $@"

.PHONY: clean
clean:
	@$(RM) -f $(LIB) $(OBJS)
	@$(RM) -f *.expand
	@echo "    Remove Objects:   $(OBJS)"
