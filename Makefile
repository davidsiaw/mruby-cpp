CC := g++
CFLAGS := -std=c++11 -I. -I./mruby/include
LDFLAGS := -Lmruby/build/host/lib/ -lmruby
TEST_DIR := tests
BIN_DIR := bins
LOG_DIR := logs
LIBMRUBY := mruby/build/host/lib/libmruby.a
GITREF := 1.4.0

TESTS := $(wildcard $(TEST_DIR)/*.cpp)
BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/test_%, $(TESTS))

TESTCOMMAND := $(patsubst $(TEST_DIR)/%.cpp, %, $(wildcard $(TEST_DIR)/*.cpp))

all: test

mruby/minirake:
	git clone https://github.com/mruby/mruby

mruby/gitref: mruby/minirake
	echo $(GITREF) > mruby/gitref

$(LIBMRUBY): mruby/gitref
	cd mruby && git checkout `cat gitref`
	cd mruby && make

$(BIN_DIR)/test_%: $(TEST_DIR)/%.cpp $(LIBMRUBY) mruby.hpp
	@mkdir -p $(BIN_DIR)
	$(CC) $(patsubst $(BIN_DIR)/test_%, $(TEST_DIR)/%.cpp, $@) $(CFLAGS) -o $@ $(LDFLAGS)

test: $(BINS)
	@mkdir -p $(LOG_DIR)
	@$(foreach file, $(TESTCOMMAND), \
		if sh -c $(BIN_DIR)/test_$(file) 1> $(LOG_DIR)/$(file).stdout 2> $(LOG_DIR)/$(file).stderr; then \
			echo "PASSED: $(file)"; \
		else \
			echo "FAILED: $(file)"; fi; \
	)

clean:
	rm $(BIN_DIR)/*
	cd mruby && make clean

.PHONY: clean test
