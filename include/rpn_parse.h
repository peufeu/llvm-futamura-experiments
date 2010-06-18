#ifndef _RPN_PARSE_H_INCLUDED
#define _RPN_PARSE_H_INCLUDED

void rpn_print_instruction( const RPN_Instruction * instr );
void rpn_print_program( const RPN_Instruction * program, int program_length );
int rpn_parse( const char* s, RPN_Instruction ** p_program );

#endif