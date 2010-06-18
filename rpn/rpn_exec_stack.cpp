#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <math.h>

#include "rpn.h"

// can try to put it in a global too, optimizes better than a local (!!!)
//~ RPN_Stack	stack;

// execs a RPN program using a stack, returns 0 on success, result in *result, takes params in *vars
int rpn_exec_stack( RPN_ProgramPtr _program, int program_length, RPN_Value *result, const RPN_Variables * vars )
{
	const RPN_Instruction *program = (const RPN_Instruction *) _program;
	int pc;
	RPN_Stack	stack;
	rpn_stack_init( &stack );
	
	// using stack.length fails to optimize because of spurious aliasing with stack_data[] so we use a 
	// serarate variable. Switch the next 2 lines to see the effect.
	int stack_length = 0;
	//~ int& stack_length = stack.length;
	//~ int& __restrict stack_length = stack.length;
	
	//~ RPN_Value * __restrict stack_data = stack.stack;
	RPN_Value * __restrict stack_data = stack.stack;
	
	if( program_length <= 0 )
		return -1;
	
	for( pc=0; pc<program_length; pc++ )
	{
		const RPN_Instruction *instr = &program[ pc ];
		
		switch( instr->opcode )
		{
			case RPN_OPCODE_ADD:
				if( stack_length < 2 )
					goto error;
				
				stack_data[ stack_length - 2 ] = stack_data[ stack_length - 2 ] + stack_data[ stack_length - 1 ];
				stack_length --;
				break;
			
			case RPN_OPCODE_SUB:
				if( stack_length < 2 ) 
					goto error;
				
				stack_data[ stack_length - 2 ] = stack_data[ stack_length - 2 ] - stack_data[ stack_length - 1 ];
				stack_length --;
				break;
			
			case RPN_OPCODE_MUL:
				if( stack_length < 2 ) 
					goto error;
				
				stack_data[ stack_length - 2 ] = stack_data[ stack_length - 2 ] * stack_data[ stack_length - 1 ];
				stack_length --;
				break;
			
			case RPN_OPCODE_DIV:
				if( stack_length < 2 ) 
					goto error;
				
				stack_data[ stack_length - 2 ] = stack_data[ stack_length - 2 ] / stack_data[ stack_length - 1 ];
				stack_length --;
				break;
			
			case RPN_OPCODE_CONST:
				if( stack_length >= RPN_Stack_Size-1 ) 
					goto error;
				
				stack_data[ stack_length ++ ] = instr->param.value;
				break;
			
			case RPN_OPCODE_VAR:
				if( stack_length >= RPN_Stack_Size-1 ) 
					goto error;
				
				stack_data[ stack_length ++ ] = vars->array[ instr->param.index ];
				break;
			
			default:
				goto error;
				break;
		error:
			printf( "Error at instruction %d\n", pc );
			
			// IF the return is not commented, loop unroll will not happen, and the function will not be specialized.
			//~ return -1;
		}			
	}
	
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
	
	*result = stack_data[0];
	return 0;
}
