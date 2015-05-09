CP = g++
LD = g++

NAME = ra

OBJ_DIR = obj
SRC_DIR = src

CP_FLAGS = -std=c++0x -O3 -Wall
LD_FLAGS = -lpthread

SRC = $(shell find $(SRC_DIR) -type f -regex ".*\.cpp")
OBJ = $(subst $(SRC_DIR), $(OBJ_DIR), $(addsuffix .o, $(basename $(SRC))))
DEP = $(OBJ:.o=.d)

EXC = $(NAME)

all: $(EXC)

$(EXC): $(OBJ)
	@echo [LD] $@
	@mkdir -p $(dir $@)
	@$(LD) -o $@ $^ $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo [CP] $<
	@mkdir -p $(dir $@)
	@$(CP) $< -c -o $@ -MMD $(CP_FLAGS)

clean:
	@echo [RM] cleaning
	@rm $(OBJ_DIR) $(EXC) -rf

-include $(DEP)
