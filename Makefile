
CORE = ra
MODULES = ra_correct ra_overlap ra_layout ra_consensus to_afg

INC_DIR = include/$(CORE)
LIB_DIR = lib
BIN_DIR = bin

all: TARGETS=install
clean: TARGETS=remove clean
install: TARGETS=install

all: $(CORE) $(MODULES)

clean: $(CORE) $(MODULES)
	@echo [RM] removing
	@rm $(INC_DIR) $(LIB_DIR) $(BIN_DIR) -rf

install: $(CORE) $(MODULES)

$(CORE):
	@echo [CORE] $@
	@$(MAKE) -s -C $@ $(TARGETS)

$(MODULES): $(CORE)
	@echo [MOD] $@
	@$(MAKE) -s -C $@ $(TARGETS)

.PHONY: $(CORE) $(MODULES)
