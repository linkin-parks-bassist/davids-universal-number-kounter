hnames := dunkprog.h
cnames := dunkprog.c

incdir := include
srcdir := src
objdir := bin

headers := $(addprefix $(incdir)/,$(hnames))
cfiles  := $(addprefix $(srcdir)/,$(cnames))
objs    := $(addprefix $(objdir)/,$(cnames:.c=.o))

all: $(objs)
	gcc $(objs) -o bin/dunkasm

install: all
	cp bin/dunkasm /usr/bin/

$(objdir)/%.o : $(cfiles)
	gcc -I include -c $(srcdir)/$*.c -o $@

$(cfiles) : $(headers)

clean:
	rm bin/* 2> /dev/null || true

