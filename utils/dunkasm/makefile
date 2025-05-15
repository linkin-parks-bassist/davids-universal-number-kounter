mnames := instr param lines label alias files tokenizer buffer context dstrings encoding
hnames := dunkasm.h $(addsuffix .h, $(mnames))
cnames := main.c $(addsuffix .c, $(mnames))

CFLAGS = 
DEBUG_FLAGS = -g -D DEBUG_BUILD

incdir := include
srcdir := src
objdir := bin

headers := $(addprefix $(incdir)/,$(hnames))
cfiles  := $(addprefix $(srcdir)/,$(cnames))
objs	:= $(addprefix $(objdir)/,$(cnames:.c=.o))

all: $(objs)
	gcc $(CFLAGS) $(objs) -o bin/dunkasm

install: all
	cp bin/dunkasm /usr/bin/

$(objdir)/%.o : $(cfiles)
	gcc $(CFLAGS) -I include -c $(srcdir)/$*.c -o $@

$(cfiles) : $(headers)

clean:
	yes | rm bin/dunkasm $(objs) 2> /dev/null || true

debug:
	make CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)"

debug_install:
	make install CFLAGS="$(CFLAGS)  $(DEBUG_FLAGS)"
