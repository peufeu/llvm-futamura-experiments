#ifndef _RPN_EXEC_H_INCLUDED
#define _RPN_EXEC_H_INCLUDED

// let's make the stack size a compile time constant...
#define RPN_Stack_Size 8

typedef double RPN_Value;
typedef union {
	struct {
		RPN_Value	a;
		RPN_Value	b;
		RPN_Value	c;
	} vars;
	
	RPN_Value array[ 26 ];
} RPN_Variables;

typedef struct {
	int		length;		// 0 if empty
	RPN_Value	stack[ RPN_Stack_Size ];
} RPN_Stack;

typedef long RPN_ProgramPtr;
//~ typedef const RPN_Instruction *RPN_ProgramPtr;

int rpn_exec_recursive( RPN_ProgramPtr _program, int program_length, RPN_Value *result, const RPN_Variables * vars );

int rpn_exec_stack( RPN_ProgramPtr _program, int program_length, RPN_Value *result, const RPN_Variables * vars );
int rpn_exec_stack_f( RPN_ProgramPtr _program, int program_length, RPN_Value *result, const RPN_Variables * vars );
int rpn_exec_stack_const( RPN_ProgramPtr _program, int program_length, RPN_Value *result, const RPN_Variables * vars );

static inline void rpn_stack_init( RPN_Stack * stack )
{
	//~ int i;
	stack->length = 0;
	//~ for( i=0; i<RPN_Stack_Size; i++ )
		//~ stack->stack[i] = NAN;
}


#endif