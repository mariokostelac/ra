DX = doxygen
DOC = doc/Doxyfile

NAME = ra

all: debug release

install: all

test: release
	@./debug/run_tests

debug:
	@echo [MAKE] $(NAME) $@
	@$(MAKE) -C debug

release:
	@echo [MAKE] $(NAME) $@
	@$(MAKE) -C release

docs:
	@echo [DX] generating documentation
	@$(DX) $(DOC)

clean:
	@echo [MAKE] clean
	@$(MAKE) -C debug clean
	@$(MAKE) -C release clean

.PHONY: default all debug release docs clean uninstall remove
