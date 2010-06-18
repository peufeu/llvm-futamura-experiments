#include "llvm/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Support/StandardPasses.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Attributes.h"
#include "llvm/BasicBlock.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/Casting.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"

#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <set>

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>

#include "rpn.h"

using namespace llvm;

// pointers to ourself, initialized from lli
extern Module * LLVM_Module;
extern ExecutionEngine * LLVM_EE;

// ok it's dirty to include .cpp files...
#include "llvm/loop_unroll.cpp"
#include "llvm/inline_function.cpp"
#include "llvm/futamurize.cpp"

typedef int (*rpn_exec_pfunc_2)( RPN_Value *, const RPN_Variables * );

// Checking function to verify we get the right results
extern double check_formula( double a, double b, double c )
{ 
	//~ return (a+b+10)*(a-b+5) / (c-4) ;
	return (a+b+10);
}

int main( int argc, char ** argv ) 
{
	printf( "#####################\n" );
	printf( "Got LLVM_Module : %p\n", LLVM_Module );
	printf( "Got LLVM_EE     : %p\n", LLVM_EE );
	
	// Create a simple RPN calculator program...
	RPN_Instruction	* program;
	int		program_length;
	
	// Formula as RPN
	//~ const char	* program_text = "a b 10 + + a b - 5 + * c 4 - /";
	const char	* program_text = "a b 10 + +";

	// Parse it and encode it as opcodes for our tiny RPM calculator VM
	program_length = rpn_parse( program_text, &program );
	if( !program_length ) return 0;
	if( program_length < 0 ) return -1;

	// print it
	rpn_print_program( program, program_length );
	
	// Create variables for our calculator
	// some default values
	double a = 3, b = 5, c = 7;
	
	RPN_Variables	vars;
	RPN_Value	result;
	int		r;
	
	memset( &vars, 0, sizeof( vars ));
	vars.vars.a	= a;
	vars.vars.b	= b;
	vars.vars.c	= c;
	
	// have the tiny RPM calculator VM execute (interpret) the program to chekc it works
	r = rpn_exec_stack_const( (RPN_ProgramPtr)program, program_length, &result, &vars );
	printf( "rpn_exec_stack_const : rcode %d result %lf expected %lf\n", r, result, check_formula( a,b,c ) );

	r = rpn_exec_stack_f( (RPN_ProgramPtr)program, program_length, &result, &vars );
	printf( "rpn_exec_stack_f     : rcode %d result %lf expected %lf\n", r, result, check_formula( a,b,c ) );

	r = rpn_exec_stack( (RPN_ProgramPtr)program, program_length, &result, &vars );
	printf( "rpn_exec_stack       : rcode %d result %lf expected %lf\n", r, result, check_formula( a,b,c ) );

	r = rpn_exec_recursive( (RPN_ProgramPtr)program, program_length, &result, &vars );
	printf( "rpn_exec_recursive   : rcode %d result %lf expected %lf\n", r, result, check_formula( a,b,c ) );

	LLVMContext &context = getGlobalContext();
	
	// now futamurize our interpreter to turn it into specialized machine code for our little rpn program
	
	// get original function
	const Function *orig_func;
	
	// uncomment line below to choose which interpreter variant to use
	
	// try this one first
	// rpn_exec_stack : futamurizer doesn't work because loop-unroller doesn't unroll the loop 
	// (loop contains an exit condition so the exact # of iterations is unknown... only the max # is)
	//~ const GlobalValue* orig_func_ptr = LLVM_EE->getGlobalValueAtAddress( (void *)rpn_exec_stack );
	
	// rpn_exec_stack_f : special version of rpn_exec_stack where the opcode switch() is in a separate function.
	// loop exit condition is removed (so there is no error handling) to make the loop unrollable.
	//~ const GlobalValue* orig_func_ptr = LLVM_EE->getGlobalValueAtAddress( (void *)rpn_exec_stack_f );
	
	// version where the "program" is a known compile time constant, to see if the compiler can do at compile-time
	// what we're trying to do at runtime
	//~ const GlobalValue* orig_func_ptr = LLVM_EE->getGlobalValueAtAddress( (void *)rpn_exec_stack_const );

	// rpn_exec_recursive : this one is the trickiest since it operates recursively. 
	// Works perfectly but needs the inliner to do quite a lot of work and doesn't eliminate the setjmp() ; we'd need C++ exceptions
	const GlobalValue* orig_func_ptr = LLVM_EE->getGlobalValueAtAddress( (void *)rpn_exec_recursive );

	printf("Trying to get orig_func_ptr through getGlobalValueAtAddress : %p\n", orig_func_ptr );
	if( orig_func_ptr )
	{
		printf( "Success.\n" );
		orig_func = dyn_cast<Function>( orig_func_ptr );
	}
	else
	{
		printf( "Failure. Run lli with -disable-lazy-compilation.\n" );
		return -1;
	}
	
	//~ orig_func->dump();
	//~ return 0;

	// create a set of addresses which will contain known constant bytes for the lifetime of
	// the specialized function
	std::set<const unsigned char *> constant_addresses_set;
	{
		// the known constant memory locations are basically the RPN program
		for( int instr = 0; instr<program_length; instr++ )
			for( int byte = 0; byte < sizeof( RPN_Instruction ); byte++ )
			{
				const unsigned char * p = ((const unsigned char*)&program[instr]) + byte;
				//~ printf( "setting as const memoty location : %p\n", p );
				constant_addresses_set.insert( p );
			}
	}
	
	// Replace constant arguments with Constant Values
	DenseMap<const Value*, Value*> argmap;
	
	Function::const_arg_iterator arg_it = orig_func->arg_begin();
	const Argument* cur_arg;
	
	// replace first argument (program)
	cur_arg = dyn_cast<Argument>( arg_it );
	printf("- function arg 1 ptr = %p\n", cur_arg );

	// I have to pass the pointer as an int, no idea why.
	//~ argmap[ cur_arg ] = ConstantExpr::getBitCast( ConstantInt::get( Type::getInt64Ty( context ), (long)program, false /* signed */ ), cur_arg->getType() );
	argmap[ cur_arg ] = ConstantInt::get( Type::getInt64Ty( context ), (long)program, false /* signed */ );

	// replace second argument (program_length)
	arg_it ++;
	cur_arg = dyn_cast<Argument>( arg_it );
	printf("- function arg 0 ptr = %p\n", cur_arg );
	argmap[ cur_arg ] = ConstantInt::get( Type::getInt32Ty( context ), program_length, true /* signed */ );
	
	Function *specialized_func = futamurize( orig_func, argmap, constant_addresses_set );
	
	// JIT it
	printf( "\nCompiling %s\n", specialized_func->getNameStr().c_str() );
		
	specialized_func->dump();

	// Execute it
	void *fptr = LLVM_EE->getPointerToFunction( specialized_func );

	if( 1 )
	{
		printf( "Executing %s\n", specialized_func->getNameStr().c_str() );
		rpn_exec_pfunc_2 fp = (rpn_exec_pfunc_2)(intptr_t)fptr;
		
		result = -1;
		r = fp( &result, &vars );
	}
	
	// should display rcode 0 (no error) and same result as interpreter
	printf( "\nspecialized func   : rcode %d result %lf expected %lf\n", r, result, check_formula( a,b,c ) );
	
	return 0;
}


