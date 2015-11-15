
CORE = ra
MODULES = ra_consensus to_afg consensus unitigger overlap2dot zoom \
					filter_contained filter_transitive widen_overlaps layout \
					filter_erroneous_overlaps depot fill_read_coverage

INC_DIR = include/$(CORE)
LIB_DIR = lib
BIN_DIR = bin

all: TARGETS=install
clean: TARGETS=clean
install-debug: TARGETS=install
install-release: TARGETS=install

all: install-release

test: ra
	@make -C ra test

clean: $(CORE) $(MODULES)
	@echo [RM] removing $(INC_DIR) $(LIB_DIR) $(BIN_DIR)
	@rm $(INC_DIR) $(LIB_DIR) $(BIN_DIR) -rf

install: install-release

install-release: build
	@echo [CP] release binaries
	@cp $(BIN_DIR)/release/* bin/

install-debug: build
	@echo [CP] debug binaries
	@cp $(BIN_DIR)/debug/* bin/

build: $(CORE) $(MODULES)

$(CORE):
	@echo [CORE] $@
	@$(MAKE) -s -C $@ $(TARGETS)

$(MODULES): $(CORE)
	@echo [MOD] $@
	@$(MAKE) -s -C $@ $(TARGETS)

.PHONY: $(CORE) $(MODULES) install install-release install-debug test build clean all
