CFLAGS= -std=c11 #-pedantic #-fsanitize=address
#LIBS_MACOS= -L /usr/local/lib -lcheck
#LIBS_LINUX= -lsubunit -lpthread -lrt #-D_GNU_SOURCE
CC = gcc

OS = $(shell uname)
ifeq ($(OS), Darwin) # MAC
OPEN= open ./report/index.html
LIBS = -L /usr/local/lib #-lcheck
CC +=-D MacOS
LEAK_CHECK=leaks --atExit -- ./s_test
endif

ifeq ($(OS), Linux) # LINUX
OPEN=
LIBS=  -lsubunit -lpthread -lrt #-D_GNU_SOURCE
CC += -D Linux
LEAK_CHECK=valgrind --log-file=valgrind_output --leak-check=full --show-leak-kinds=all -s ./s_test
endif

all: clean s_decimal.a test gcov_report


clean:
	rm -Rf *.o *.a *.gcno *.gcda *.info report s_test s_decimal s_test_report

test.o: s_tests_decimal.c
	$(CC) -c s_tests_decimal.c -lcheck -lm $(LIBS)

test: clean test.o s_decimal.a s_tests_decimal.o
	$(CC) s_tests_decimal.o -L. s_decimal.a -o s_test -lcheck -lm $(LIBS) 
	./s_test

clang:
	clang-format -i *.c *.h
	clang-format -n *.c *.h

s_decimal.a: clean s_additional.c s_arithmetic.c s_comparison.c s_converter.c s_decimal.h s_math_funcs.c
	$(CC) -c s_additional.c s_arithmetic.c s_comparison.c s_converter.c s_math_funcs.c -lm
	ar rc s_decimal.a s_additional.o s_arithmetic.o s_comparison.o s_converter.o s_math_funcs.o
	ranlib s_decimal.a

gcov_report: clean s_additional.c s_arithmetic.c s_comparison.c s_converter.c s_decimal.h s_math_funcs.c s_tests_decimal.c
	$(CC) -fprofile-arcs -ftest-coverage --coverage *.c -o s_test_report -lcheck -lm $(LIBS)
	./s_test_report
	lcov -t s_test_report -o test.info -c -d .
	genhtml -o report test.info
	$(OPEN)


s_decimal.o: s_arithmetic.c s_comparison.c s_converter.c s_math_funcs.c s_additional.c
	gcc $(CFLAGS) -c *.c

s_arithmetic.o: s_arithmetic.c
	gcc -c s_arithmetic.c

s_comparison.o: s_comparison.c
	gcc -c s_comparison.c

s_converter.o: s_converter.c
	gcc -c s_converter.c

s_math_funcs.o: s_math_funcs.c
	gcc -c s_math_funcs.c

s_additional.o: s_additional.c
	gcc -c s_additional.c
 
leaks: test
	$(LEAK_CHECK)

lcov:

cppcheck:
	cppcheck *.c *.h

