all: utils docs

utils: dunkasm dunkprog

install:
	sudo make -C utils/dunkasm install
	sudo make -C utils/dunkprog install

docs:
	make -C docs


clean:
	make -C docs clean
	make -C utils/dunkasm clean
	make -C utils/dunkprog clean

.PHONY: docs

dunkasm:
	make -C utils/dunkasm

dunkprog:
	make -C utils/dunkprog
