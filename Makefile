CC          = gcc
HPDEMO_LIB_CCFLAGS     = -g -O3 -fPIC -shared -lstdc++ -msse4 \
                     	-I./include  -I/usr/local/include \
                     	-L. -L/usr/local/lib \
                     	-lhashpipe -lrt -lm
HPDEMO_LIB_TARGET   = hashpipe.so
HPDEMO_LIB_SOURCES  = net_thread.c 		\
		      		  output_thread.c 	\
                      databuf.c			\
					  obs_data.c		\
					  obs_redis.c		
HPDEMO_LIB_INCLUDES = databuf.h

all: $(HPDEMO_LIB_TARGET)

$(HPDEMO_LIB_TARGET): $(HPDEMO_LIB_SOURCES) $(HPDEMO_LIB_INCLUDES)
	$(CC) -o $(HPDEMO_LIB_TARGET) $(HPDEMO_LIB_SOURCES) $(HPDEMO_LIB_CCFLAGS)

tags:
	ctags -R .
clean:
	rm -f $(HPDEMO_LIB_TARGET) tags

prefix=/usr/local
LIBDIR=$(prefix)/lib
BINDIR=$(prefix)/bin
install-lib: $(HPDEMO_LIB_TARGET)
	mkdir -p "$(DESTDIR)$(LIBDIR)"
	install -p $^ "$(DESTDIR)$(LIBDIR)"
install: install-lib

.PHONY: all tags clean install install-lib
