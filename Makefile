CC          = gcc
LIB_CCFLAGS     = -g -O3 -fPIC -shared -msse4


LIB_TARGET   = limbo.so

LIB_INCLUDES = -I./include \
				-I/usr/local/include
LIB_LINKS	 = -L. -L/usr/local/lib \
					-lstdc++ -lhiredis\
				-lhashpipe -lrt -lm
LIB_DIR		 = lib

LIB_SRCS = ${wildcard ./src/*.c}

all: $(LIB_TARGET)

$(LIB_TARGET): file ${LIB_SRCS} 
	$(CC) ${LIB_SRCS} -o $@ ${LIB_INCLUDES} ${LIB_LINKS} ${LIB_CCFLAGS} 
	@mv ${LIB_TARGET} ${LIB_DIR}
file :
	@echo ${LIB_SRCS}

tags:
	ctags -R .
	
clean:
	rm -f $(LIB_DIR)/$(LIB_TARGET) tags

prefix=/usr/local
LIBDIR=$(prefix)/lib
BINDIR=$(prefix)/bin
install-lib: $(LIB_DIR)/$(LIB_TARGET)
	mkdir -p "$(DESTDIR)$(LIBDIR)"
	install -p $^ "$(DESTDIR)$(LIBDIR)"
install: install-lib

.PHONY: all tags clean install install-lib
