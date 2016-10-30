Ocean is a WIP scripting language generator and virtual machine.

The scripting language generator has exactly one goal: To be simple and easy to work with so that users can create their own scripting languages at a whim without long compile turnarounds or lots of time spent understanding abstract syntax trees and other compiler concepts. The syntax definition file is a simple, regular expression-based format that expands on Backus-Naur Form with a few additional new concepts. The compiler parses the syntax file at runtime and then uses that data to parse a script file, and convert it to bytecode, which will be the same for all user-defined scripting languages such that any Ocean-compiled file can import and work with any other file from any language. Once done, additional functionality not directly supported by the compiler will be able to be added by users via shared library modules loaded by the compiler.

The VM itself has several more concrete goals:
- Easy embedding in C and C++ code to make it easy to use in environments such as game engines
- Strong support for multithreading with no locks at the interpreter level
- Emphasis on performance, to achieve the highest possible throughput of bytecode instructions
- Built-in support for cooperative multitasking modules, such as coroutines, generators, continuations, and channels
- Fast native function calls with minimal overhead transitioning between script and native environments
- Support for multiple memory management models on a per-object basis, including automatic stack memory, reference-counted memory, and manual memory management
- Cross-platform compatibility
- Native mathematics and other native function calls whenever possible
- Support for dynamic module importing

For the moment, only a few of these objectives have been met and the VM is limited to mathematical operations, function calls, and printing their results.