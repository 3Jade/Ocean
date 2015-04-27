#pragma once

#include <stdint.h>

enum class OpCode : int64_t
{
	//Stack manipulation
	PUSH	= 0b000001, //Push memory address {arg} onto the stack
	POP		= 0b000010, //Pop the top {arg} memory addresses from the stack
	SWAP	= 0b000011, //Swap the memory address at position {arg} with the top of the stack
	COPY	= 0b000100, //Copy the memory address at position {arg} to the top of the stack

	//Instruction pointer manipulation
	JUMP	= 0b000101, //Move instruction pointer to offset {arg}
	JUMPIF	= 0b000110, //JUMP if the top of the stack contains a non-zero value
	JUMPNIF	= 0b000111, //JUMP if the top of the stack contains a zero value

	//Functions
	CALL	= 0b001000, //Call the specified function. {arg} is a pointer into the strings table with the function's name.
	RETURN	= 0b001001, //Exit the specified function and return to previous instruction pointer location. {arg} is ignored.

	//Variables
	DECL	= 0b001010, //Create a set of variables for the current scope. {arg} is a variable count.
	LOAD	= 0b001011, //Load the contents of a variable onto the top of the stack. {arg} is an offset from the end of the variables array.
	STORE	= 0b001100, //Store the top value of the stack into the given variable name. {arg} is an offset from the end of the variables array.
	DEL		= 0b001101, //Remove {arg} count of variables from the back of the variable array.

	//Class functions
	CREATE	= 0b001110, //Create an object of a non-POD type. {arg} is a pointer into the strings table with the name of the class to construct.
	GET		= 0b001111, //Retrieve a value from a non-POD object. {arg} is a pointer into the strings table with the variable name. This variable is retrieved from the object on top of the stack.
	SET		= 0b010000, //Set a value to a non-POD object. {arg} is a pointer into the strings table with the variable name. This variable is retrieved from the object on top of the stack.
	DESTROY	= 0b010001, //Explicitly release the memory for the object on top of the stack. {arg} is ignored.

	//Modules
	LOADMOD	= 0b010010, //Load the named module if it's not already loaded. {arg} is a pointer into the strings table with the module name.

	//System
	EXIT	= 0b010011, //Exit the application.
};
