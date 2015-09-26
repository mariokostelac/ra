NAME = layout

BIN=../bin/$(NAME)

all: debug release

install: layout.rb
	@echo [MAKE] install
	@cp $< $(BIN)

debug:
	@echo [MAKE] $(NAME) $@

release:
	@echo [MAKE] $(NAME) $@

.PHONY: default all debug release install
