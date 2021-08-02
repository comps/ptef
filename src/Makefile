.PHONY: all
all: lib cli bash python

.PHONY: helpers runner report mklog lib cli bash python
helpers:
	@$(MAKE) -C helpers
runner:
	@$(MAKE) -C runner
report:
	@$(MAKE) -C report
mklog:
	@$(MAKE) -C mklog

lib: helpers runner report mklog
	@$(MAKE) -C lib

cli: lib
	@$(MAKE) -C cli
bash: lib
	@$(MAKE) -C bash
python: lib
	@$(MAKE) -C python

.PHONY: clean
clean:
	@$(MAKE) -C helpers clean
	@$(MAKE) -C runner clean
	@$(MAKE) -C report clean
	@$(MAKE) -C mklog clean
	@$(MAKE) -C lib clean
	@$(MAKE) -C cli clean
	@$(MAKE) -C bash clean
	@$(MAKE) -C python clean
