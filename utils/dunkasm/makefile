mnames := instr param lines label alias files tokenizer buffer context dstrings encoding error options inclusion
hnames := dunkasm.h $(addsuffix .h, $(mnames))
cnames := main.c $(addsuffix .c, $(mnames))

CFLAGS = 
DEBUG_FLAGS = -g -D DEBUG_BUILD -O0

incdir := include
srcdir := src
objdir := bin

headers := $(addprefix $(incdir)/,$(hnames))
cfiles  := $(addprefix $(srcdir)/,$(cnames))
objs	:= $(addprefix $(objdir)/,$(cnames:.c=.o))
nmobjs  := $(addsuffix .o,$(addprefix $(objdir)/,$(mnames)))

build:
	make all

all: $(objs)
	gcc $(CFLAGS) $(objs) -o bin/dunkasm
	gcc $(CFLAGS) -I include tests/tests.c $(nmobjs) -o bin/tests
	bin/tests

install: all install_lang
	cp bin/dunkasm /usr/bin/

install_lang:
	scripts/extract_instr.py
	scripts/install_lang.sh
	rm dunkasm.lang

$(objdir)/%.o : $(cfiles)
	gcc $(CFLAGS) -I include -c $(srcdir)/$*.c -o $@

$(cfiles) : $(headers)

clean:
	yes | rm bin/dunkasm $(objs) 2> /dev/null || true

debug:
	make CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)"

debug_install:
	make install CFLAGS="$(CFLAGS)  $(DEBUG_FLAGS)"

bin/tests.o : debug
	gcc $(CFLAGS) $(DEBUG_FLAGS) -I include -c tests/tests.c -o bin/tests.o

tests: bin/tests.o
	gcc $(CFLAGS) $(DEBUG_FLAGS) -I include bin/tests.o $(nmobjs) -o bin/tests
