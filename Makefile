.PHONY: all
all:
	$(MAKE) -C src install-include
	$(MAKE) -C src dep
	$(MAKE) -C src
	$(MAKE) -C doc
	$(MAKE) -C src doc

.PHONY: dot dot-png dot-eps
depend: depend-png depend-eps

depend-eps:
	dot -Teps depend-libraries.dot > depend-libraries.eps
	dot -Teps depend-includes.dot > depend-includes.eps
depend-png:
	dot -Tpng depend-libraries.dot > depend-libraries.png
	dot -Tpng depend-includes.dot > depend-includes.png

.PHONY: clean

clean: clean-bin clean-src clean-include clean-lib

.PHONY: clean-src
clean-src:
	$(MAKE) -C src     clean

.PHONY: clean-bin
clean-bin:
	$(MAKE) -C bin     clean

.PHONY: clean-include
clean-include:
	$(MAKE) -C include clean

.PHONY: clean-lib
clean-lib:
	$(MAKE) -C lib     clean

.PHONY: clean-doc
clean-doc:
	$(MAKE) -C doc     clean
