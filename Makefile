# SPDX-License-Identifier: MIT
# Author:  Giovanni Santini
# Mail:    giovanni.santini@proton.me
# License: MIT

#
# Compiler files
#
CFLAGS      = -Wall -Werror -Wpedantic -std=c99
DEBUG_FLAGS = -ggdb
LDFLAGS     = -lasound -lm
TEST_LDFLAGS = -Wl,-T,tests/micro-tests.ld
CC?         = gcc

#
# Project files
#
OUT_NAME = demo
OBJ      = demo.o
TEST_NAME = lp_tests
TEST_OBJ  = tests/tests.o

#
# Commands
#
all: $(OUT_NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(OUT_NAME)

check: test

test: $(TEST_NAME)
	chmod +x $(TEST_NAME)
	./$(TEST_NAME)

run: $(OUT_NAME)
	chmod +x $(OUT_NAME)
	./$(OUT_NAME)

clean:
	rm -f $(OBJ)

distclean:
	rm -f $(OUT_NAME)

$(OUT_NAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(CFLAGS) -o $(OUT_NAME)

$(TEST_NAME): $(TEST_OBJ)
	$(CC) $(TEST_OBJ) $(TEST_LDFLAGS) $(LDFLAGS) $(CFLAGS) -o $(TEST_NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
