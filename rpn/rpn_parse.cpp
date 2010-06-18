#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "rpn.h"

// display an instruction for debugging purposes.
void rpn_print_instruction( const RPN_Instruction * instr )
{
	printf( "op %2d ", instr->opcode );
	switch( instr->opcode )
	{
		case	RPN_OPCODE_ADD:		printf( "ADD" );		 			break;	
		case	RPN_OPCODE_SUB:         printf( "SUB" ); 					break;
		case	RPN_OPCODE_MUL:         printf( "MUL" );			 		break;  
		case	RPN_OPCODE_DIV:         printf( "DIV" ); 					break;  
		case	RPN_OPCODE_CONST:       printf( "CONST %lf", instr->param.value );			break;
		case	RPN_OPCODE_VAR:         printf( "VAR   %c", (char)(instr->param.index + 'a'));	break;
		default:			printf( "Unknown opcode %d", instr->opcode );		break;
	}
}

// display a program for debugging purposes.
void rpn_print_program( const RPN_Instruction * program, int program_length )
{
	int i;
	printf( "\nProgram contains %d instructions\n", program_length );
	for( i=0; i<program_length; i++ )
	{
		printf( "%5d : ", i );
		rpn_print_instruction( &program[i] );
		printf( "\n" );
	}
	printf( "\n" );
}

// parses a text program and returns its length (-1 for error)
static int rpn_parse_2( const char* s, RPN_Instruction * program )
{
	char		c;
	int		program_length = 0;
	
	// parse it
	while( (c = *s) )
	{
		RPN_Instruction	instr = { RPN_OPCODE_INVALID, 0 };

		if(isspace( c ))
		{
			s++;
			continue;
		}
		
		if( c>= 'a' && c <= 'z' )	// variable ?
		{
			instr.opcode		= RPN_OPCODE_VAR;
			instr.param.index	= c - 'a';
			goto add_instr_inc;
		}
		
		if( isdigit( c ))		// integer constant ?
		{
			char * endp;
			instr.opcode		= RPN_OPCODE_CONST;
			instr.param.value	= strtod( s, &endp );
			if( !endp )	goto error;
			s = endp;
			goto add_instr;
		}
		
		switch( c )
		{
			case '+':	instr.opcode	= RPN_OPCODE_ADD;	goto add_instr_inc;
			case '-':	instr.opcode	= RPN_OPCODE_SUB;	goto add_instr_inc;
			case '*':	instr.opcode	= RPN_OPCODE_MUL;	goto add_instr_inc;
			case '/':	instr.opcode	= RPN_OPCODE_DIV;	goto add_instr_inc;
		}

error:		
		printf( "Error at position : '%s'", s );
		return -1;

add_instr_inc:
		s++;
add_instr:
		if( program )
			program[ program_length ] = instr;
		
		program_length++;
	}
	
	return program_length;
}

// parses a text program and returns its length (-1 for error)
// allocates storage for program (except in empty program case)
int rpn_parse( const char* s, RPN_Instruction ** p_program )
{
	int		r, program_length;
	
	// make a dummy pass to get program length
	program_length = rpn_parse_2( s, NULL );
	if( program_length <= 0 ) return program_length;
		
	// allocate storage for program
	*p_program = (RPN_Instruction*) malloc( program_length * sizeof( RPN_Instruction ));
	
	// parse for real this time
	return rpn_parse_2( s, *p_program );
}
