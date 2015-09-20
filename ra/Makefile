DX = doxygen
DOC = doc/Doxyfile

NAME = ra

all: debug release tests

install: all

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

tests:
	@echo [MAKE]  tests
	@$(MAKE) -C tests

.PHONY: default all debug release docs clean uninstall remove tests
