EXENAMES = client server

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
bin/server: server.c | bin
	gcc $^ $(CFLAGS) -o $@

bin:
	mkdir -p bin

.PHONY: release
release: $(addprefix release-bin/, $(EXENAMES))

release-bin/client: client.c | release-bin
	gcc $^ -std=c11 -D_POSIX_C_SOURCE=200809L -o $@
release-bin/server: server.c | release-bin
	gcc $^ -std=c11 -D_POSIX_C_SOURCE=200809L -o $@

release-bin:
	mkdir -p release-bin

clean:
	rm -rf *.o bin release-bin
