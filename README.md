# Bald++
**A modern, high-performance interpreted programming language with automatic memory management and JIT compilation.**

![Bald++ Logo](https://img.shields.io/badge/Bald++-v1.0-blue.svg)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)
![C++23](https://img.shields.io/badge/C++-23-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

Bald++ is a dynamic programming language built with modern C++23, featuring a dual virtual machine architecture (tree-walking interpreter + bytecode JIT), automatic memory management through smart pointers and RAII, and a comprehensive standard library. The language combines the simplicity of scripting languages with the performance optimizations of modern runtime systems.

## 🚀 Features

- **🏃‍♂️ High Performance**: Dual VM architecture with adaptive JIT compilation
- **🧠 Smart Memory Management**: Zero-pause garbage collection using RAII and smart pointers  
- **⚡ Modern Syntax**: Clean, readable syntax with powerful language constructs
- **📚 Rich Standard Library**: Comprehensive built-in functions for strings, arrays, math, and I/O
- **🔧 Easy Integration**: Simple C++ API for embedding in applications
- **🌐 Cross-Platform**: Builds on Linux, macOS, and Windows
- **🎯 Production Ready**: Comprehensive test suite and performance benchmarks

## 📋 Language Overview

### Basic Syntax

```bald
// Variables and basic operations
name = "Bald++"
version = 1.0
count = 42

// Arithmetic
result = (count * 2) + version
println("Result: " + to_string(result))

// Arrays
numbers = [1, 2, 3, 4, 5]
push(numbers, 6)
println("Numbers: " + join(numbers, ", "))

// Functions
factorial = function(n)
    if n <= 1 then
        return 1
    end if
    return n * factorial(n - 1)
end function

println("5! = " + to_string(factorial(5)))

// Control Flow
for i in range(1, 6, 1)
    if i % 2 == 0 then
        println(to_string(i) + " is even")
    else
        println(to_string(i) + " is odd")
    end if
end for
```

### Data Types

| Type | Description | Example |
|------|-------------|---------|
| **Number** | Double-precision floating point | `42`, `3.14`, `-17` |
| **String** | UTF-8 text sequences | `"Hello"`, `'World'` |
| **Boolean** | True/false values | `true`, `false` |
| **Array** | Dynamic collections | `[1, 2, 3]`, `["a", "b", "c"]` |
| **Function** | First-class functions | `function(x) return x * 2 end function` |
| **Nil** | Null/empty value | `nil` |

### Control Structures

#### Conditional Statements
```bald
if condition then
    // statements
else if other_condition then
    // statements  
else
    // statements
end if
```

#### Loops
```bald
// For loops with range
for i in range(0, 10, 1)
    println(to_string(i))
end for

// For loops over arrays
for item in array
    println(to_string(item))
end for

// While loops
while condition
    // statements
end while
```

#### Functions
```bald
// Function definition
function_name = function(param1, param2)
    // function body
    return result
end function

// Function call
result = function_name(arg1, arg2)
```

## 📚 Standard Library

### 🔢 Math Functions
- `abs(x)` - Absolute value
- `ceil(x)` - Round up to nearest integer
- `floor(x)` - Round down to nearest integer  
- `round(x)` - Round to nearest integer
- `sqrt(x)` - Square root
- `rnd(n)` - Random integer from 0 to n-1
- `parse_num(s)` - Convert string to number (returns nil if invalid)
- `to_string(n)` - Convert number to string

### 📝 String Functions
- `len(s)` - Get string length
- `lower(s)` - Convert to lowercase
- `upper(s)` - Convert to uppercase
- `split(s, delim)` - Split string by delimiter
- `join(list, delim)` - Join array elements into string
- `replace(s, old, new)` - Replace substring

### 📋 Array Functions
- `range(start, stop, step)` - Generate array of numbers from start to stop-1
- `len(list)` - Get array length
- `push(list, item)` - Add item to end of array
- `pop(list)` - Remove and return last item
- `insert(list, index, item)` - Insert item at index
- `remove(list, index)` - Remove item at index
- `sort(list)` - Sort array in-place

### 🖥️ I/O Functions
- `print(x)` - Print value without newline
- `println(x)` - Print value with newline
- `read()` - Read line from input
- `stacktrace()` - Get current call stack

## 🏗️ Installation and Building

### Prerequisites

- **GCC 11+** or **Clang 12+** or **MSVC 2022+** (C++23 support required)
- **CMake 3.20+**
- **Git**

### Quick Start

#### Linux/macOS
```bash
# Clone repository
git clone <repository-url>
cd bald-plus-plus

# Build and run
./build.sh --release --examples
./build/interpreter examples/hello_world.bald
```

#### Windows
```cmd
# Clone repository
git clone <repository-url>
cd bald-plus-plus

# Build and run
build.bat --release --examples
build\interpreter.exe examples\hello_world.bald
```

### Build Options

#### Linux/macOS Build Script
```bash
# Development build
./build.sh --debug --examples --tests

# Release build
./build.sh --release --examples

# Clean build
./build.sh --clean --release --examples

# Show all options
./build.sh --help
```

#### Windows Build Script  
```cmd
# Development build
build.bat --debug --examples --tests

# Release build
build.bat --release --examples

# Clean build
build.bat --clean --release --examples

# Show all options
build.bat --help
```

### Manual Building

#### Linux/macOS
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
cmake --build . --parallel
```

#### Windows (Visual Studio)
```cmd
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
cmake --build . --config Release --parallel
```

## 🚀 Usage

### Running Programs
```bash
# Run a Bald++ program
./build/interpreter program.bald

# Interactive REPL mode
./build/interpreter

# Run with verbose output
./build/interpreter --verbose program.bald

# Run with JIT compilation enabled
./build/interpreter --jit program.bald
```

### Command Line Options
- `--verbose` - Enable verbose output
- `--debug` - Enable debug mode
- `--jit` - Enable JIT compilation
- `--help` - Show help message

## 📊 Example Programs

### Factorial Calculator
```bald
// factorial.bald - Recursive factorial calculation

factorial = function(n)
    if n <= 1 then
        return 1
    end if
    return n * factorial(n - 1)
end function

for i in range(0, 11, 1)
    result = factorial(i)
    println("factorial(" + to_string(i) + ") = " + to_string(result))
end for
```

### Array Sorting
```bald
// sorting.bald - Array sorting demonstration

// Create descending array
arr = []
for i in range(10, 0, -1)
    push(arr, i)
end for

println("Before sorting: " + join(arr, ", "))

// Sort the array
sort(arr)

println("After sorting: " + join(arr, ", "))
```

### Prime Number Generation
```bald
// primes.bald - Sieve of Eratosthenes

sieve_of_eratosthenes = function(limit)
    if limit < 2 then
        return []
    end if

    is_prime = []
    for i in range(0, limit + 1, 1)
        push(is_prime, true)
    end for

    is_prime[0] = false
    is_prime[1] = false

    sqrt_limit = floor(sqrt(limit))
    for p in range(2, sqrt_limit + 1, 1)
        if is_prime[p] then
            multiple = p * p
            while multiple <= limit
                is_prime[multiple] = false
                multiple = multiple + p
            end while
        end if
    end for

    primes = []
    for i in range(2, limit + 1, 1)
        if is_prime[i] then
            push(primes, i)
        end if
    end for

    return primes
end function

// Generate primes up to 100
primes = sieve_of_eratosthenes(100)
println("Primes up to 100: " + join(primes, ", "))
println("Found " + to_string(len(primes)) + " primes")
```

## 🧪 Testing and Benchmarks

### Running Tests
```bash
# Run all example programs
make run_all_programs

# Run individual tests
./build/interpreter examples/factorial_demo.bald
./build/interpreter examples/sorting_demo.bald
./build/interpreter examples/primes_demo.bald

# Run comprehensive benchmark
./build/interpreter examples/comprehensive_demo.bald
```

### Performance Benchmarks
The included benchmarks test:
- **Recursive factorial** calculation (factorial(20), 1000 iterations)
- **Array sorting** (100,000 elements, quicksort + built-in sort)
- **Prime generation** (Sieve of Eratosthenes up to 100,000)

Typical performance on modern hardware:
- **Factorial(20)**: ~10-50μs per calculation
- **Sort 100K elements**: ~50-150ms
- **Generate 9,592 primes**: ~100-300ms

## 🏗️ Architecture

### Virtual Machine Design
Bald++ uses a **dual virtual machine architecture**:

1. **Tree-Walking Interpreter**: Direct AST evaluation for development and debugging
2. **Bytecode JIT Compiler**: Adaptive compilation to bytecode for performance-critical code

```
Source Code (.bald) → Lexer → Parser → AST → Tree-Walker VM
                                         ↓
                                   JIT Compiler → Bytecode VM
```

### Memory Management
- **Smart Pointers**: Automatic reference counting with `std::shared_ptr`
- **RAII**: Deterministic resource cleanup
- **Scope-Based GC**: Environment destructors handle variable cleanup  
- **Zero-Pause Collection**: No stop-the-world garbage collection

### JIT Compilation
- **Adaptive Compilation**: Hot expressions compiled to bytecode automatically
- **Performance Tracking**: Statistics on compilation efficiency
- **Bytecode Cache**: Compiled expressions cached for reuse

## 🤝 Contributing

### Development Setup
```bash
# Clone and build development version
git clone <repository-url>
cd bald-plus-plus
./build.sh --debug --examples --tests

# Run tests
make test
make run_all_programs

# Format code
make format
```

### Project Structure
```
bald-plus-plus/
├── lib/                    # Core interpreter
│   ├── interpreter.h       # Main interpreter header  
│   ├── interpreter.cpp     # Implementation
│   ├── gc.h               # Garbage collection
│   ├── simple_jit.h       # JIT compiler header
│   └── simple_jit.cpp     # JIT implementation
├── examples/              # Example programs (.bald)
├── tests/                 # Unit tests
├── build/                 # Build artifacts
├── CMakeLists.txt         # CMake configuration
├── build.sh              # Linux/macOS build script
├── build.bat             # Windows build script  
└── README.md             # This file
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built as part of a programming language implementation course
- Demonstrates modern C++23 features and best practices
- Inspired by languages like Python, JavaScript, and Lua
- Uses advanced virtual machine and memory management techniques

## 🚀 Quick Links

- **[Examples](examples/)** - Sample programs and tutorials
- **[Build Scripts](build.sh)** - Automated building for all platforms  

---

**Bald++ - Where simplicity meets performance.** 💪

*Built with ❤️ and modern C++23*

