#! /bin/bash

# compiles a local version of lli.

# change this ...
export LLVM_SOURCE_PATH="/home/peufeu/dev/llvm/llvm-2.7"

g++ -I$LLVM_SOURCE_PATH/include -I$LLVM_SOURCE_PATH/tools/lli \
	-g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -O2 -fomit-frame-pointer -fno-exceptions \
	-fno-rtti -fPIC -Woverloaded-virtual -pedantic -Wno-long-long -Wall -W -Wno-unused-parameter -Wwrite-strings \
	-c lli_patched_source/lli.cpp -o lli_patched_source/lli.o && \
g++ 	-g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -O2 -fomit-frame-pointer -fno-exceptions \
	-fno-rtti -fPIC -Woverloaded-virtual -pedantic -Wno-long-long -Wall -W -Wno-unused-parameter -Wwrite-strings  \
	-O2 -Wl,-R -Wl,-R -Wl,-export-dynamic \
	-o lli lli_patched_source/lli.o  \
	`llvm-config --ldflags --libs all` && \
exit 0
exit -1


