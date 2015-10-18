NAME = fill_read_coverage

all: debug release

install:
	@echo [MAKE] install
	@$(MAKE) -C debug install
	@$(MAKE) -C release install

debug:
	@echo [MAKE] $(NAME) $@
	@$(MAKE) -C debug

release:
	@echo [MAKE] $(NAME) $@
	@$(MAKE) -C release

clean:
	@echo [MAKE] clean
	@$(MAKE) -C debug clean
	@$(MAKE) -C release clean

.PHONY: default all debug release clean remove
