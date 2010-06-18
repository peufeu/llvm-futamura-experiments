#! /bin/bash

# compiles the example program (yeah I should write a Makefile)

# LLVM is compiled with -fno-rtti so we need to use it too if we want to inherit from llvm's classes
export MYCFLAGS="-emit-llvm -fno-rtti -O3 -Iinclude -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -g"
export COMPILER="llvm-g++"
#~ export COMPILER="clang"

$COMPILER $MYCFLAGS -c rpn/rpn_exec_recursive.cpp	-o objs/rpn_exec_recursive.bc && \
$COMPILER $MYCFLAGS -c rpn/rpn_exec_stack.cpp		-o objs/rpn_exec_stack.bc && \
$COMPILER $MYCFLAGS -c rpn/rpn_exec_stack_f.cpp		-o objs/rpn_exec_stack_f.bc && \
$COMPILER $MYCFLAGS -c rpn/rpn_exec_stack_const.cpp	-o objs/rpn_exec_stack_const.bc && \
$COMPILER $MYCFLAGS -c rpn/rpn_parse.cpp		-o objs/rpn_parse.bc && \
$COMPILER $MYCFLAGS -c rpn_main.cpp			-o objs/rpn_main.bc && \
llvm-ld `llvm-config --ldflags --libs all` \
	-o objs/rpn_module_bitcode objs/rpn_main.bc objs/rpn_exec_recursive.bc objs/rpn_exec_stack.bc \
	objs/rpn_parse.bc objs/rpn_exec_stack_f.bc objs/rpn_exec_stack_const.bc && \
./lli -O3 -disable-lazy-compilation objs/rpn_module_bitcode.bc && \
exit 0
exit -1


