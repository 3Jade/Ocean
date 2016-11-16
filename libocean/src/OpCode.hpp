#pragma once

#include <stdint.h>

enum class OpCode : int64_t
{
	//Stack manipulation
	PUSH	= 0x00, //Push memory address {arg} onto the stack
	POP		= 0x01, //Pop the top {arg} memory addresses from the stack
	SWAP	= 0x02, //Swap the memory address at position {arg} with the top of the stack
	COPY	= 0x03, //Copy the memory address at position {arg} to the top of the stack

	//Instruction pointer manipulation
	JUMP	= 0x10, //Move instruction pointer to offset {arg}
	JUMPIF	= 0x11, //JUMP if the top of the stack contains a non-zero value
	JUMPNIF	= 0x12, //JUMP if the top of the stack contains a zero value

	//Functions
	CALL	= 0x20, //Call the specified function. {arg} is a pointer into the strings table with the function's name.
	CALLN	= 0x21, //Call the specified native function. {arg} is a pointer directly to the function in memory, patched in at startup.
	RETURN	= 0x22, //Exit the specified function and return to previous instruction pointer location. {arg} is the number of variables to delete.
	CALLND  = 0x23, //Indicates a non-destructive function call which is going to return with the stack in the same condition it's currently in (it will read, but not consume, its parameters)

	//Variables
	DECL	= 0x30, //Create a set of variables for the current scope. {arg} is a variable count.
	LOAD	= 0x31, //Load the contents of a variable onto the top of the stack. {arg} is an offset from the end of the variables array.
	STORE	= 0x32, //Store the top value of the stack into the given variable name. {arg} is an offset from the end of the variables array.
	DEL		= 0x33, //Remove {arg} count of variables from the back of the variable array.
	MEMO    = 0x34, //Store the top value of the stack into the given variable name, without popping the stack. {arg} is an offset from the end of the variables array.

	//Class functions
	CREATE	= 0x40, //Create an object of a non-POD type. {arg} is a pointer into the strings table with the name of the class to construct.
	GET		= 0x41, //Retrieve a value from a non-POD object. {arg} is a pointer into the strings table with the variable name. This variable is retrieved from the object on top of the stack.
	SET		= 0x42, //Set a value to a non-POD object. {arg} is a pointer into the strings table with the variable name. This variable is retrieved from the object on top of the stack.
	DESTROY	= 0x43, //Explicitly release the memory for the object on top of the stack. {arg} is ignored.

	//Modules
	LOADMOD	= 0x50, //Load the named module if it's not already loaded. {arg} is a pointer into the strings table with the module name.

	//System
	EXIT	= 0x60, //Exit the application.
	
	MAX
};
