
CORE = ra
MODULES = ra_consensus to_afg consensus unitigger overlap2dot zoom filter_contained filter_transitive widen_overlaps layout

INC_DIR = include/$(CORE)
LIB_DIR = lib
BIN_DIR = bin

all: TARGETS=install
clean: TARGETS=clean
install: TARGETS=install

all: $(CORE) $(MODULES)

test: ra
	@make -C ra test

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
	@cp bin/release/* bin/

.PHONY: $(CORE) $(MODULES)
