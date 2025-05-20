hnames := dunkprog.h microops.h opcodes.h
cnames := dunkprog.c

CFLAGS = 

incdir := include
srcdir := src
objdir := bin

headers := $(addprefix $(incdir)/,$(hnames))
cfiles  := $(addprefix $(srcdir)/,$(cnames))
objs    := $(addprefix $(objdir)/,$(cnames:.c=.o))

all: $(objs)
	gcc $(CFLAGS) $(objs) -o bin/dunkprog

install: all
	cp bin/dunkprog /usr/bin/

$(objdir)/%.o : $(cfiles)
	gcc -I include -c $(CFLAGS) $(srcdir)/$*.c -o $@

$(cfiles) : $(headers)

clean:
	yes | rm bin/* 2> /dev/null || true

