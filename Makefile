CC := g++
CFLAGS := -std=c++11 -I. -I./mruby/include
LDFLAGS := -Lmruby/build/host/lib/ -lmruby
TEST_DIR := tests
BIN_DIR := bins
LIBMRUBY := mruby/build/host/lib/libmruby.a
GITREF := 1.4.0

TESTS := $(wildcard $(TEST_DIR)/*.cpp)
BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/test_%, $(TESTS))

TESTCOMMAND := $(patsubst $(TEST_DIR)/%.cpp, %, $(wildcard $(TEST_DIR)/*.cpp))

all: test

$(LIBMRUBY):
	git clone https://github.com/mruby/mruby
	cd mruby && git checkout `cat gitref`
	cd mruby && make

$(BIN_DIR)/test_%: $(TEST_DIR)/%.cpp $(LIBMRUBY) mruby.hpp
	$(CC) $(patsubst $(BIN_DIR)/test_%, $(TEST_DIR)/%.cpp, $@) $(CFLAGS) -o $@ $(LDFLAGS)

test: $(BINS)
	@mkdir -p logs
	@$(foreach file, $(TESTCOMMAND), \
		if sh -c $(BIN_DIR)/test_$(file) 1> logs/$(file).stdout 2> logs/$(file).stderr; then \
			echo "PASSED: $(file)"; \
		else \
			echo "FAILED: $(file)"; fi; \
	)

clean:
	rm $(BINS)/*
	cd mruby && make clean

.PHONY: clean test
