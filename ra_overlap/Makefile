CP = g++
LD = g++

NAME = ra_overlap

UNAME_S := $(shell uname -s)
versions := 0

ifeq ($(UNAME_S),Darwin)
	compiler := $(shell g++-4.8 --version 2>/dev/null)
	ifdef compiler
		CP = g++-4.8
		LD = g++-4.8
	endif

	compiler := $(shell g++-4.9 --version 2>/dev/null)
	ifdef compiler
		CP = g++-4.9
		LD = g++-4.9
	endif
endif

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = ../include/$(NAME)
LIB_DIR = ../lib
EXC_DIR = ../bin

I_CMD = $(addprefix -I, $(SRC_DIR) ../include )
L_CMD = $(addprefix -L, ../lib )

DEP_LIBS = ../lib/libra.a

CP_FLAGS = $(I_CMD) -std=c++0x -O3 -Wall
LD_FLAGS = $(I_CMD) $(L_CMD) -lra -lstdc++ -pthread

API = $(addprefix $(SRC_DIR)/, )

SRC = $(shell find $(SRC_DIR) -type f -regex ".*\.cpp")
OBJ = $(subst $(SRC_DIR), $(OBJ_DIR), $(addsuffix .o, $(basename $(SRC))))
DEP = $(OBJ:.o=.d)
INC = $(subst $(SRC_DIR), $(INC_DIR), $(API))
LIB = $(LIB_DIR)/lib$(NAME).a
EXC = $(NAME)
BIN = $(EXC_DIR)/$(EXC)

DEBUG := $(shell env | grep DEBUG | cut -d= -f2)
ifeq ($(DEBUG),1)
	CP_FLAGS += -g
	CP_FLAGS += -DDEBUG
endif

all: $(EXC)

install: bin

bin: $(BIN)

include: $(INC)

lib: $(LIB)

$(EXC): $(OBJ) $(DEP_LIBS)
	@echo [LD] $@
	@mkdir -p $(dir $@)
	@$(LD) $(OBJ) -o $@ $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo [CP] $<
	@mkdir -p $(dir $@)
	@$(CP) $< -c -o $@ -MMD $(CP_FLAGS)

$(INC_DIR)/%.hpp: $(SRC_DIR)/%.hpp
	@echo [CP] $@
	@mkdir -p $(dir $@)
	@cp $< $@

$(LIB): $(OBJ)
	@echo [AR] $@
	@mkdir -p $(dir $@)
	@ar rcs $(LIB) $(OBJ)

$(BIN): $(EXC)
	@echo [CP] $@
	@mkdir -p $(dir $@)
	@cp $< $@

clean:
	@echo [RM] cleaning
	@rm $(OBJ_DIR) $(EXC) -rf

remove:
	@echo [RM] removing
	@rm $(INC_DIR) $(LIB) $(BIN) $(EXC) $(WIN) -rf

-include $(DEP)
