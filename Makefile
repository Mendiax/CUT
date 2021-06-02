# SHELL SECTION
RM := rm -rf

# PROJECT TREE
S_DIR := ./src
I_DIR := ./inc ./external/KTest/inc
A_DIR := ./app
T_DIR := ./test

# FILES
SRC := $(wildcard $(S_DIR)/*.c)

A_SRC := $(SRC) $(wildcard $(A_DIR)/*.c)
T_SRC := $(SRC) $(wildcard $(T_DIR)/*.c)

A_OBJ := $(A_SRC:%.c=%.o)
T_OBJ := $(T_SRC:%.c=%.o)

OBJ := $(A_OBJ) $(T_OBJ)

DEPS := $(OBJ:%.o=%.d)

LIB_PATH := ./external/KTest/
LIB := ktest

# EXEC
EXEC := main.out
T_EXEC := test.out

# COMPILATOR SECTION


CC ?= gcc

C_FLAGS := -Wall -Wextra -g -rdynamic

DEP_FLAGS := -MMD -MP

H_INC := $(foreach d, $(I_DIR), -I$d)
L_INC := $(foreach l, $(LIB), -l$l)
L_PATH := $(foreach p, $(LIB_PATH), -L$p)

ifeq ($(CC),clang)
	C_FLAGS += -Weverything
else ifneq (, $(filter $(CC), cc gcc))
	C_FLAGS += -rdynamic
endif

ifeq ("$(origin O)", "command line")
	OPT := -O$(O)
else
	OPT := -O3
endif

ifeq ("$(origin G)", "command line")
	GGDB := -ggdb$(G)
else
	GGDB :=
endif

C_FLAGS += $(OPT) $(GGDB) $(DEP_FLAGS)
main:$(EXEC)

all:  $(EXEC)

test: $(T_EXEC)

$(EXEC): $(A_OBJ)
	@echo "compile main"
	$(CC)  $(H_INC) $(A_OBJ) -o $@ $(L_PATH) $(L_INC) $(C_FLAGS)

$(T_EXEC): $(T_OBJ)
	@echo "compile tests"
	$(CC) $(H_INC) $(T_OBJ) -o $@ $(L_PATH) $(L_INC) $(C_FLAGS)

%.o:%.c %.d
	$(CC) $(C_FLAGS) $(H_INC) -c $< -o $@

clean:
	@echo "clean all"
	$(RM) $(EXEC)
	$(RM) $(T_EXEC)
	$(RM) $(OBJ)
	$(RM) $(DEPS)

$(DEPS):

include $(wildcard $(DEPS))
