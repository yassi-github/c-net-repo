EXENAMES = client server-exclusive server-no-exclusive sample-client sample-server

CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -fstack-protector -Wall -Wextra -Werror
OPTIMIZATION = -O3
DEBUG = no

ifeq ($(DEBUG), yes)
	CFLAGS += -g
	OPTIMIZATION = -O0
endif

CFLAGS += $(OPTIMIZATION)

all: $(addprefix bin/, $(EXENAMES))

bin/client: client.c | bin
	gcc $^ $(CFLAGS) -o $@
bin/server-exclusive: server-exclusive.c | bin
	gcc $^ $(CFLAGS) -o $@
bin/server-no-exclusive: server-no-exclusive.c | bin
	gcc $^ $(CFLAGS) -o $@
bin/sample-client: sample-client.c | bin
	gcc $^ $(CFLAGS) -o $@
bin/sample-server: sample-server.c | bin
	gcc $^ $(CFLAGS) -o $@

bin:
	mkdir -p bin

clean:
	rm -rf *.o bin
