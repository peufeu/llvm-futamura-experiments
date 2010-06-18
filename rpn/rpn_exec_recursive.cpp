#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
     
#include "rpn.h"

// recursively execs a RPN program, starting from the end (reverse order)
static RPN_Value 
__attribute__ ((noinline))
rpn_exec_recursive_eval( const RPN_Instruction * program, int & pc, const RPN_Variables * vars, jmp_buf & exception_env )  
{
	RPN_Value result;
	
	if( pc < 0 )
		longjmp( exception_env, 0 );
	
	const RPN_Instruction & instr = program[ pc-- ];
		
	switch( instr.opcode )
	{
		case RPN_OPCODE_ADD:
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env );
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env ) + result;
			break;
		
		case RPN_OPCODE_SUB:
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env );
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env ) - result;
			break;
		
		case RPN_OPCODE_MUL:
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env );
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env ) * result;
			break;
		
		case RPN_OPCODE_DIV:
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env );
			result = rpn_exec_recursive_eval( program, pc, vars, exception_env ) / result;
			break;
		
		case RPN_OPCODE_CONST:
			result = instr.param.value;
			break;
		
		case RPN_OPCODE_VAR:
			result = vars->array[ instr.param.index ];
			break;
		
		default:
			//~ longjmp( exception_env, 0 );
			break;
	}
	//~ printf( "=> %lf\n", result );
	return result;
}

// recursively execs a RPN program, returns 0 on success, result in *result, takes params in *vars
int rpn_exec_recursive( RPN_ProgramPtr _program, int program_length, RPN_Value * result, const RPN_Variables * vars )
{	
	jmp_buf exception_env;

	if( program_length <= 0 )
		return -1;
	
	if (!setjmp( exception_env ))  
	{
		const RPN_Instruction * __restrict program = (const RPN_Instruction *) _program;
		int	pc = program_length - 1;
		
		*result = rpn_exec_recursive_eval( program, pc, vars, exception_env );
		
		return 0;
        }
        else
	{
		printf( "Error at instruction %d\n", program_length );
		return -1;
	}
}




