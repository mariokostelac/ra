CP = g++
LD = g++

NAME = ra

OBJ_DIR = obj
SRC_DIR = src
VND_DIR = vendor
INC_DIR = ../include/$(NAME)
LIB_DIR = ../lib
EXC_DIR = ../bin

I_CMD = $(addprefix -I, $(SRC_DIR))
I_CMD_V = $(addprefix -I, $(VND_DIR))
L_CMD = $(addprefix -L, )

CP_FLAGS = $(I_CMD) $(I_CMD_V) -std=c++0x -g -Wall
LD_FLAGS = $(I_CMD) $(I_CMD_V) $(L_CMD) -pthread -lstdc++

API = $(addprefix $(SRC_DIR)/, CommonHeaders.hpp EnhancedSuffixArray.hpp IO.hpp \
	    Overlap.hpp PartialOrderAlignment.hpp Preprocess.hpp ra.hpp Read.hpp ReadIndex.hpp \
	    StringGraph.hpp Utils.hpp)

SRC = $(shell find $(SRC_DIR) -type f -regex ".*\.cpp")
VND = $(shell find $(VND_DIR) -type f -regex ".*\.cpp")
OBJ = $(subst $(SRC_DIR), $(OBJ_DIR), $(addsuffix .o, $(basename $(SRC))))
OBJ += $(subst $(VND_DIR), $(OBJ_DIR), $(addsuffix .o, $(basename $(VND))))
DEP = $(OBJ:.o=.d)
INC = $(subst $(SRC_DIR), $(INC_DIR), $(API))
LIB = $(LIB_DIR)/lib$(NAME).a
EXC = $(NAME)
BIN = $(EXC_DIR)/$(EXC)

all: $(OBJ)

install: lib include

bin: $(BIN)

include: $(INC)

lib: $(LIB)

$(EXC): $(OBJ)
	@echo [LD] $@
	@mkdir -p $(dir $@)
	@$(LD) -o $@ $^ $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo [CP] $<
	@mkdir -p $(dir $@)
	@$(CP) $< -c -o $@ -MMD $(CP_FLAGS)

$(OBJ_DIR)/%.o: $(VND_DIR)/%.cpp
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
	@rm $(INC_DIR) $(LIB) $(BIN) $(EXC) -rf

-include $(DEP)
