mnames := instr param lines label alias files tokenizer
hnames := dunkasm.h $(addsuffix .h, $(mnames))
cnames := main.c $(addsuffix .c, $(mnames))

CFLAGS = 

incdir := include
srcdir := src
objdir := bin

headers := $(addprefix $(incdir)/,$(hnames))
cfiles  := $(addprefix $(srcdir)/,$(cnames))
objs    := $(addprefix $(objdir)/,$(cnames:.c=.o))

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
	make CFLAGS="-g"
