#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <math.h>

#include "rpn.h"

static int __attribute__ ((noinline))
rpn_exec_instr( const RPN_Instruction *instr, RPN_Stack *stack, const RPN_Variables * vars )
{
	switch( instr->opcode )
	{
		case RPN_OPCODE_ADD:
			if( stack->length < 2 )
				return -1;
			
			stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] + stack->stack[ stack->length - 1 ];
			stack->length --;
			break;
		
		//~ case RPN_OPCODE_SUB:
			//~ if( stack->length < 2 ) 
				//~ return -1;
			
			//~ stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] - stack->stack[ stack->length - 1 ];
			//~ stack->length --;
			//~ break;
		
		//~ case RPN_OPCODE_MUL:
			//~ if( stack->length < 2 ) 
				//~ return -1;
			
			//~ stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] * stack->stack[ stack->length - 1 ];
			//~ stack->length --;
			//~ break;
		
		//~ case RPN_OPCODE_DIV:
			//~ if( stack->length < 2 ) 
				//~ return -1;
			
			//~ stack->stack[ stack->length - 2 ] = stack->stack[ stack->length - 2 ] / stack->stack[ stack->length - 1 ];
			//~ stack->length --;
			//~ break;
		
		case RPN_OPCODE_CONST:
			if( stack->length >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack->stack[ stack->length ++ ] = instr->param.value;
			break;
		
		case RPN_OPCODE_VAR:
			if( stack->length >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack->stack[ stack->length ++ ] = vars->array[ instr->param.index ];
			break;
		
		default:
			return -1;
			break;
	}
	return 0;	
}

static inline int __attribute__ ((noinline))
rpn_exec_instr_with_tests_sl( const RPN_Instruction * instr, RPN_Value __restrict *stack, int * __restrict stack_length, const RPN_Variables * vars )
{
	switch( instr->opcode )
	{
		case RPN_OPCODE_ADD:
			if( (*stack_length) < 2 )
				return -1;
			
			stack[ (*stack_length) - 2 ] = stack[ (*stack_length) - 2 ] + stack[ (*stack_length) - 1 ];
			(*stack_length) --;
			break;
				
		case RPN_OPCODE_CONST:
			if( (*stack_length) >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack[ (*stack_length) ] = instr->param.value;
			(*stack_length) ++;
			break;
		
		case RPN_OPCODE_VAR:
			if( (*stack_length) >= RPN_Stack_Size-1 ) 
				return -1;
			
			stack[ (*stack_length) ] = vars->array[ instr->param.index ];
			(*stack_length) ++;
			break;
		
		default:
			return -1;
			break;
	}
	return 0;	
}

int rpn_exec_stack_f( RPN_ProgramPtr _program, int program_length, RPN_Value *result, const RPN_Variables * vars )
{
	const RPN_Instruction *program = (const RPN_Instruction *) _program;
	RPN_Stack	stack;
	
	int stack_length;
	//~ int &stack_length = stack.length;
	
	stack_length = 0;
	
	if( program_length <= 0 )
		return -1;
	
	int pc;
	for( pc=0; pc<program_length; pc++ )
	{
		// if the error handling is uncommented, loop doesn't unroll.
		//~ if( rpn_exec_instr_with_tests_sl( &program[ pc ], stack.stack, &stack_length, vars ) ) goto error;
		rpn_exec_instr_with_tests_sl( &program[ pc ], stack.stack, &stack_length, vars );
	}
	
	//~ if( rpn_exec_instr_with_tests_sl( &program[ pc=0 ], stack.stack, &stack_length, vars ) ) goto error;
	//~ if( rpn_exec_instr_with_tests_sl( &program[ pc=1 ], stack.stack, &stack_length, vars ) ) goto error;
	//~ if( rpn_exec_instr_with_tests_sl( &program[ pc=2 ], stack.stack, &stack_length, vars ) ) goto error;
	//~ if( rpn_exec_instr_with_tests_sl( &program[ pc=3 ], stack.stack, &stack_length, vars ) ) goto error;
	//~ if( rpn_exec_instr_with_tests_sl( &program[ pc=4 ], stack.stack, &stack_length, vars ) ) goto error;
	
	if( stack_length > 1 )
	{
		printf( "Error : more than one value returned.\n" );
		return -1;
	}
	else if( stack_length < 1 )
	{
		printf( "Error : no value returned.\n" );
		return -1;
	}
	
	*result = stack.stack[0];
	return 0;

error:
	printf( "Error at instruction %d\n", pc );
	return -1;
}
