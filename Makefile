CC := g++
TEST_DIR := tests
BIN_DIR := bins
LOG_DIR := logs
OBJ_DIR := objs
LIBMRUBY := mruby/build/host/lib/libmruby.a
SHELL := bash
CFLAGS := -std=c++11 -I. -I./mruby/include -Wall -fprofile-arcs -ftest-coverage
LDFLAGS := -Lmruby/build/host/lib/ -lmruby

SOURCES := $(wildcard *.hpp)
TESTS := $(wildcard $(TEST_DIR)/*.cpp)
BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/test_%, $(TESTS))

ALL_TEST_RESULTS := $(patsubst $(TEST_DIR)/%.cpp, $(LOG_DIR)/test_%, $(TESTS))
ALL_COUNTS := $(patsubst $(TEST_DIR)/%.cpp, %_ccount, $(TESTS))

TESTCOMMAND := $(patsubst $(TEST_DIR)/%.cpp, %, $(wildcard $(TEST_DIR)/*.cpp))

RED_TEXT ?= \e[31m
GREEN_TEXT ?= \e[32m
LRED_TEXT ?= \e[91m
NORMAL_TEXT ?= \e[0m

all: test

$(LIBMRUBY):
	cd mruby && make

$(BIN_DIR)/test_%: $(TEST_DIR)/%.cpp $(LIBMRUBY) $(SOURCES)
	@echo "COMPILE $@"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(patsubst $(BIN_DIR)/test_%, $(TEST_DIR)/%.cpp, $@) $(CFLAGS) -o $@ $(LDFLAGS)

$(LOG_DIR):
	@mkdir -p $(LOG_DIR)

$(LOG_DIR)/test_%: $(BIN_DIR)/test_%
	@mkdir -p $@
	@touch $@
ifeq ($(LEAKCHECK), 1)
	@echo "LEAKCHK $(@:$(LOG_DIR)/test_%=test_%)"
	valgrind --error-exitcode=1 --leak-check=full --log-file=$@/valgrind.log $(BIN_DIR)/$(@:$(LOG_DIR)/%=%) \
		1> $@/valgrind.stdout \
		2> $@/valgrind.stderr; \
		echo "$$?" > $@/valgrind.retcode
endif
	@echo "TESTING $(@:$(LOG_DIR)/test_%=test_%)"
	$(BIN_DIR)/$(@:$(LOG_DIR)/test_%=test_%) \
		1> $@/test.stdout \
		2> $@/test.stderr; \
		echo "$$?" > $@/test.retcode
	@if compgen -G "bins/*.gc*" > /dev/null; then \
	    mv bins/*.gc* $(LOG_DIR); \
	fi;
	@if compgen -G "*.gc*" > /dev/null; then \
	    mv *.gc* $(LOG_DIR); \
	fi;

all_tests: $(ALL_TEST_RESULTS)

count_files:
	@rm -f fail
	@rm -f pcount
	@rm -f fcount
	@rm -f lcount
	@touch pcount
	@touch fcount
	@touch lcount

%_ccount: count_files all_tests
	@if grep -q 0 $(LOG_DIR)/$(@:%_ccount=test_%)/test.retcode; then \
		a=$$(grep -q 0 $(LOG_DIR)/$(@:%_ccount=test_%/valgrind.retcode 2>/dev/null)); \
		retval=$$?; \
		if [ "$(LEAKCHECK)" == "1" ] && [ $$retval -eq 1  ]; then \
			printf "$(LRED_TEXT)LEAKED$(NORMAL_TEXT): $(@:%_ccount=test_%))\n"; \
			echo $@ >> lcount; \
			cat $(LOG_DIR)/$(@:%_ccount=test_%)/valgrind.log; \
			cat $(LOG_DIR)/$(@:%_ccount=test_%)/valgrind.stdout; \
			cat $(LOG_DIR)/$(@:%_ccount=test_%)/valgrind.stderr; \
			touch fail; \
		else \
			printf "$(GREEN_TEXT)PASSED$(NORMAL_TEXT): $(@:%_ccount=test_%)\n"; \
			echo $@ >> pcount; \
		fi; \
	else \
		printf "$(RED_TEXT)FAILED$(NORMAL_TEXT): $(@:%_ccount=test_%)\n"; \
		cat $(LOG_DIR)/$(@:%_ccount=test_%)/test.stderr; \
		touch fail; \
			echo $@ >> fcount; \
	fi; \

all_counts: $(ALL_COUNTS)

summary: all_counts
	@printf "$(words $(shell cat pcount)) $(GREEN_TEXT)passed$(NORMAL_TEXT)\n"
	@printf "$(words $(shell cat fcount)) $(RED_TEXT)failing$(NORMAL_TEXT)\n"
	@if [ "$(LEAKCHECK)" == "1" ]; then \
		printf "$(words $(shell cat lcount)) $(LRED_TEXT)leaking$(NORMAL_TEXT)\n"; \
	fi
	@rm -f pcount
	@rm -f fcount
	@rm -f lcount

gcov.log: all_tests
	@gcov $(LOG_DIR)/*.gcda > gcov.log
	@mv *.gcov $(LOG_DIR)

coverage: gcov.log
	gcovr --html --exclude-unreachable-branches --print-summary -o coverage.html --root .

test: summary
	@if [ -f fail ]; then echo "Test failures detected!"; exit 1; fi

lightclean:
	rm -f *.gcda *.gcno *.gcov *.gcno *.gcov

clean: lightclean
	rm -rf $(BIN_DIR)/* $(LOG_DIR)/* pcount fcount lcount

bigclean: clean
	cd mruby && make clean

distclean: bigclean
	rm -rf bins
	rm -rf logs

.SECONDARY:
.PHONY: distclean clean test
