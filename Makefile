#
# This file is a part of Polyfuse
#
# Copyright (c) 2018 Luke Gallagher <luke.gallagher@rmit.edu.au>
#
# For the full copyright and license information, please view the LICENSE file
# that was distributed with this source code.
#

TARGET = polyfuse

GIT = $(shell which git)
GITDIR = $(shell stat .git > /dev/null; echo $$?)
VERSION_NUM = 0.1.0
VERSION_EXTRA = -dev
ifneq ($(GIT),)
ifeq ($(GITDIR), 0)
	VERSION_EXTRA += ($(shell $(GIT) rev-parse --short=8 HEAD))
endif
endif
VERSION = $(VERSION_NUM)$(VERSION_EXTRA)

CC = gcc
CFLAGS += -std=c11 -Wall -Wextra -pedantic -O2 -D_XOPEN_SOURCE=700 \
		  -DPOLYFUSE_VERSION='"$(VERSION)"' -Isrc
LDFLAGS += -lm
DEBUG_CFLAGS = -g -O0 -DDEBUG

SRC = src/main.c src/util.c src/trec.c src/pf_accum.c \
          src/polyfuse.c src/pf_topic.c src/pf_pq.c
OBJ := $(SRC:.c=.o)
DEP := $(patsubst %.c,%.d,$(SRC))

TESTDIR = test

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: CFLAGS := $(CFLAGS:-O%=)
debug: CC := $(CC) $(DEBUG_CFLAGS)
debug: all

.PHONY: release
release: VERSION_EXTRA :=
release: all

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	$(MAKE) -C $(TESTDIR) $@
	$(RM) $(TARGET) $(OBJ) $(DEP)

.PHONY: check
check: $(TARGET)
	$(MAKE) -C $(TESTDIR)
	./test/all -c

-include $(DEP)
