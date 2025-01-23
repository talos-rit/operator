# This project assumes there is some higher level Makefile driving compilation (Namely, the Talos Operator)
# aswell as access to the Operator's common directory.
# The Talos Operator repo is located at: https://github.com/talos-rit/operator

ifndef DRIVER_DIR
DRIVER_DIR = Ichor/driver #set dir, if not set already (relative to SRC_DIR)
endif

FLAGS += -I$(SRC_DIR)/$(DRIVER_DIR)

DRIVER		:=

DRIVER_CPP	:=

DRIVER_UTEST:=

ODIR := $(OBJ_DIR)/$(DRIVER_DIR)
DRIVER_OBJS := $(DRIVER:%.c=$(ODIR)/%.o)
DRIVER_OBJS += $(DRIVER_CPP:%.cpp=$(ODIR)/%.o)
DRIVER_UTEST_OBJS := $(DRIVER_UTEST:%.cpp=$(ODIR)/%.o)