#ifndef _RPN_OPCODES_H_INCLUDED
#define _RPN_OPCODES_H_INCLUDED

// Opcode types for our little virtual machine
typedef enum {
	RPN_OPCODE_INVALID,
	RPN_OPCODE_ADD,
	RPN_OPCODE_SUB,
	RPN_OPCODE_MUL,
	RPN_OPCODE_DIV,
	RPN_OPCODE_CONST,	// has a value
	RPN_OPCODE_VAR	 	// has a value
}	RPN_OpCode;


// Instructions for our little virtual machine
typedef struct {
	RPN_OpCode	opcode;
	union {
		int		index;
		double		value;
	} param;
} RPN_Instruction;	

#endif