#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <math.h>

#include "rpn.h"

static inline int 
//~ __attribute__ ((noinline))
__attribute__((always_inline))
exec_instr_with_tests( const RPN_Instruction * instr, RPN_Stack *stack, const RPN_Variables * vars )
{
	switch( instr->opcode )
	{
		case RPN_OPCODE_ADD:
			if( stack->length < 2 )
				return -1;
			
			stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] + stack->stack[ stack->length - 1 ];
			stack->length --;
			break;
				
		case RPN_OPCODE_DIV:
			if( stack->length < 2 ) 
				return -1;
			
			stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] / stack->stack[ stack->length - 1 ];
			stack->length --;
			break;
		
		case RPN_OPCODE_CONST:
			if( stack->length >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack->stack[ stack->length ] = instr->param.value;
			stack->length ++;
			break;
		
		case RPN_OPCODE_VAR:
			if( stack->length >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack->stack[ stack->length ] = vars->array[ instr->param.index ];
			stack->length ++;
			break;
		
		default:
			return -1;
			break;
	}
	return 0;	
}

static inline int 
//~ __attribute__ ((noinline))
__attribute__((always_inline))
exec_instr_without_tests( const RPN_Instruction * instr, RPN_Stack *stack, const RPN_Variables * vars )
{
	switch( instr->opcode )
	{
		case RPN_OPCODE_ADD:
			stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] + stack->stack[ stack->length - 1 ];
			stack->length --;
			break;
				
		case RPN_OPCODE_DIV:
			stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] / stack->stack[ stack->length - 1 ];
			stack->length --;
			break;
		
		case RPN_OPCODE_CONST:
			stack->stack[ stack->length ] = instr->param.value;
			stack->length ++;
			break;
		
		case RPN_OPCODE_VAR:
			stack->stack[ stack->length ] = vars->array[ instr->param.index ];
			stack->length ++;
			break;
		
		default:
			return -1;
			break;
	}
	return 0;	
}

static inline int 
//~ __attribute__ ((noinline))
__attribute__((always_inline))
exec_instr_with_tests_sl( const RPN_Instruction * instr, RPN_Stack *stack, const RPN_Variables * vars, int& stack_length )
{
	switch( instr->opcode )
	{
		case RPN_OPCODE_ADD:
			if( stack_length < 2 )
				return -1;
			
			stack->stack[ stack_length - 2 ] = stack->stack[ stack_length - 2 ] + stack->stack[ stack_length - 1 ];
			stack_length --;
			break;
				
		case RPN_OPCODE_DIV:
			if( stack_length < 2 ) 
				return -1;
			
			stack->stack[ stack_length - 2 ] = stack->stack[ stack_length - 2 ] / stack->stack[ stack_length - 1 ];
			stack_length --;
			break;
		
		case RPN_OPCODE_CONST:
			if( stack_length >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack->stack[ stack_length ] = instr->param.value;
			stack_length ++;
			break;
		
		case RPN_OPCODE_VAR:
			if( stack_length >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack->stack[ stack_length ] = vars->array[ instr->param.index ];
			stack_length ++;
			break;
		
		default:
			return -1;
			break;
	}
	return 0;	
}

static inline int 
//~ __attribute__ ((noinline))
__attribute__((always_inline))
exec_instr_without_tests_sl( const RPN_Instruction * instr, RPN_Stack *stack, const RPN_Variables * vars, int& stack_length )
{
	switch( instr->opcode )
	{
		case RPN_OPCODE_ADD:
			stack->stack[ stack_length - 2 ] = stack->stack[ stack_length - 2 ] + stack->stack[ stack_length - 1 ];
			stack_length --;
			break;
				
		case RPN_OPCODE_DIV:
			stack->stack[ stack_length - 2 ] = stack->stack[ stack_length - 2 ] / stack->stack[ stack_length - 1 ];
			stack_length --;
			break;
		
		case RPN_OPCODE_CONST:
			stack->stack[ stack_length ] = instr->param.value;
			stack_length ++;
			break;
		
		case RPN_OPCODE_VAR:
			stack->stack[ stack_length ] = vars->array[ instr->param.index ];
			stack_length ++;
			break;
		
		default:
			return -1;
			break;
	}
	return 0;	
}


static const RPN_Instruction static_constant_program[5] = {
	{ RPN_OPCODE_VAR, { index: 0 } },
	{ RPN_OPCODE_VAR, { index: 1 } },
	{ RPN_OPCODE_CONST, { value: 10.0 } },
	{ RPN_OPCODE_ADD },
	{ RPN_OPCODE_ADD }
};
	
/* this one has the rpn program as a local variable so we can see how gcc handles it... */
int rpn_exec_stack_const( RPN_ProgramPtr _program, int program_length, RPN_Value *result, const RPN_Variables * vars )
{
	RPN_Stack	stack;
	rpn_stack_init( &stack );
	
	/*
	// can try this to see if the compiler realizes it can specialize exec_instr
	
	RPN_Instruction program[5];
	
	program[0].opcode = RPN_OPCODE_VAR;
	program[0].param.index = 0;
	
	program[1].opcode = RPN_OPCODE_VAR;
	program[1].param.index = 1;
	
	program[2].opcode = RPN_OPCODE_CONST;
	program[2].param.value = 10.0;
	
	program[3].opcode = RPN_OPCODE_ADD;
	program[4].opcode = RPN_OPCODE_ADD; 
	*/
	
	// or just do this
	const RPN_Instruction * program = static_constant_program;
	
	//~ exec_instr_without_tests( &program[ 0 ], &stack, vars );
	//~ exec_instr_without_tests( &program[ 1 ], &stack, vars );
	//~ exec_instr_without_tests( &program[ 2 ], &stack, vars );
	//~ exec_instr_without_tests( &program[ 3 ], &stack, vars );
	//~ exec_instr_without_tests( &program[ 4 ], &stack, vars );

	// same with tests
	//~ if( exec_instr_with_tests( &program[ 0 ], &stack, vars ) ) return -1;
	//~ if( exec_instr_with_tests( &program[ 1 ], &stack, vars ) ) return -1;
	//~ if( exec_instr_with_tests( &program[ 2 ], &stack, vars ) ) return -1;
	//~ if( exec_instr_with_tests( &program[ 3 ], &stack, vars ) ) return -1;
	//~ if( exec_instr_with_tests( &program[ 4 ], &stack, vars ) ) return -1;

	// pass stack_length as separate (avoids bogus aliasing with llvm)
	int stack_length = 0;
	//~ exec_instr_without_tests_sl( &program[ 0 ], &stack, vars, stack_length );
	//~ exec_instr_without_tests_sl( &program[ 1 ], &stack, vars, stack_length );
	//~ exec_instr_without_tests_sl( &program[ 2 ], &stack, vars, stack_length );
	//~ exec_instr_without_tests_sl( &program[ 3 ], &stack, vars, stack_length );
	//~ exec_instr_without_tests_sl( &program[ 4 ], &stack, vars, stack_length );

	// same with tests ... got to unroll the loop by hand !...
	if( exec_instr_with_tests_sl( &program[ 0 ], &stack, vars, stack_length ) ) return -1;
	if( exec_instr_with_tests_sl( &program[ 1 ], &stack, vars, stack_length ) ) return -1;
	if( exec_instr_with_tests_sl( &program[ 2 ], &stack, vars, stack_length ) ) return -1;
	if( exec_instr_with_tests_sl( &program[ 3 ], &stack, vars, stack_length ) ) return -1;
	if( exec_instr_with_tests_sl( &program[ 4 ], &stack, vars, stack_length) ) return -1;

	*result = stack.stack[0];
	return 0;
}

/*
int main()
{
	RPN_Variables	vars;
	RPN_Value	result;
	
	// some default values
	double a = 3, b = 5, c = 7;
	
	vars.vars.a	= a;
	vars.vars.b	= b;
	vars.vars.c	= c;

	int r = rpn_exec_stack_const( NULL, 0, &result, &vars );
	
	printf( "%d %lf\n", r, result );
	
	return 0;
}
*/

/* 
To observe the generated code do :

llvm-g++ -Winline -emit-llvm -O3 -Iinclude -S -c rpn_exec_stack_const.cpp -o -
g++ -Winline -emit-llvm -O3 -Iinclude -S -c rpn_exec_stack_const.cpp -o -
clang -emit-llvm -O3 -Iinclude -S -c rpn_exec_stack_const.cpp -o -

g++ -O3 generates this assembly code for rpn_exec_stack_const : 

   (in other words it perfectly specializes our little interpreter !,
   even in the "with tests" case, ie, it determines the RPN_Stack will never
   overflow and removes all tests !)
   
   However if unused opcodes (MUL/DIV) are uncommented it doesn't optimize at all...

.LVL0:
        subq    $2032, %rsp
        movq    (%rcx), %rax
        movsd   8(%rcx), %xmm0
        addsd   .LC0(%rip), %xmm0
        movq    %rax, -104(%rsp)
        xorl    %eax, %eax
        addsd   -104(%rsp), %xmm0
        movsd   %xmm0, (%rdx)
        addq    $2032, %rsp
        ret

llvm-g++ generates perfect code ONLY if stack_length is passed as a separate parameter ;
if it is inside the stack struct, false aliasing is detected and no optimization takes place

define i32 @_Z20rpn_exec_stack_constliPdPK13RPN_Variables(i64 %_program, i32 %program_length, double* nocapture %result, %union.RPN_Variables* nocapture %vars) nounwind {
entry:
  %0 = getelementptr inbounds %union.RPN_Variables* %vars, i64 0, i32 0, i64 0 ; <double*> [#uses=1]
  %1 = load double* %0, align 8                   ; <double> [#uses=1]
  %2 = getelementptr inbounds %union.RPN_Variables* %vars, i64 0, i32 0, i64 1 ; <double*> [#uses=1]
  %3 = load double* %2, align 8                   ; <double> [#uses=1]
  %4 = fadd double %3, 1.000000e+01               ; <double> [#uses=1]
  %5 = fadd double %1, %4                         ; <double> [#uses=1]
  store double %5, double* %result, align 8
  ret i32 0
}

*/