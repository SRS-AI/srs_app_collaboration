CROSS_COMPILE=aarch64-linux-gnu-
AR=${CROSS_COMPILE}ar
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
LDPP=$(CROSS_COMPILE)g++
NM=${CROSS_COMPILE}nm
RANLIB=${CROSS_COMPILE}ranlib

ELF_PREFIX := srs_app_collaboration

LIBS_BIN := 
SHARED_LIBS := ./lib

PRJDIR := .
BINDIR := ./bin
OBJDIR := ./obj

INCDIR := $(PRJDIR)

C_SOURCES :=  $(wildcard $(PRJDIR)/*.c)
CC_SOURCES := $(wildcard $(PRJDIR)/*.cc)

CCDEFINES += -Dlinux -ffunction-sections -fdata-sections -c -fmessage-length=0 -D_DEFAULT_SOURCE
LDFLAGS = -L$(SHARED_LIBS) -lpthread -lm -Xlinker -lrt -rdynamic -ldl -ltensorflowlite_c

ELF_NAME_BASE := $(ELF_PREFIX)

STRIPPED_C_SOURCES := $(notdir $(C_SOURCES))
STRIPPED_CC_SOURCES := $(notdir $(CC_SOURCES))

C_OBJECTS  := $(STRIPPED_C_SOURCES:%.c=%.o) 
CC_OBJECTS  := $(STRIPPED_CC_SOURCES:%.cc=%.o) 

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.cc $(sort $(dir $(CC_SOURCES)))

ALL_INCLUDES := $(INCDIR)
ALL_H := $(wildcard $(addsuffix /*.h,$(ALL_INCLUDES)))

CCINCLUDES := $(addprefix -I,$(ALL_INCLUDES))
CFLAGS += $(CCDEFINES) $(CCINCLUDES)
OPT_CFLAGS += $(CCDEFINES) $(CCINCLUDES)
DBG_CFLAGS += $(CCDEFINES) $(CCINCLUDES)

ELF := $(BINDIR)/$(ELF_NAME_BASE)
LNK_MAP := $(BINDIR)/$(ELF_NAME_BASE).map

#------- targets ----------------------------------------------------------------
.PHONY: all release clean cleantmp cleanall cleanalltmp

all: release

release: $(OBJDIR) $(BINDIR) $(ELF)
	@echo Building $(ELF)

cleanalltmp: cleantmp

cleantmp:
	if [ -d "./$(OBJDIR)" ];then     \
		rm -f $(OBJDIR)/*.*;\
		rmdir $(OBJDIR);\
	fi

cleanall: clean

clean: cleantmp
	if [ -d "./$(BINDIR)" ];then     \
		rm -f $(ELF) $(LNK_MAP);\
	fi

print-%:
	@echo $* = $($*)

$(OBJDIR):
	if [ ! -d "./$(OBJDIR)" ];then     \
			mkdir -p $(OBJDIR);           \
	fi

$(BINDIR):
	if [ ! -d "./$(BINDIR)" ];then     \
			mkdir -p $(BINDIR);           \
	fi

$(ELF): $(addprefix $(OBJDIR)/,$(C_OBJECTS)) $(addprefix $(OBJDIR)/,$(CC_OBJECTS)) $(addprefix -l,$(LIBS_BIN))
	$(LDPP) $(LDFLAGS)  $^ -o $@

$(OBJDIR)/%.o: %.c $(ALL_H)
	$(CC) $(OPT_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.cc $(ALL_H)
	$(CC) $(OPT_CFLAGS) -c $< -o $@

