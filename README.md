[![Discord](https://img.shields.io/discord/1289060671971000320?style=for-the-badge&logo=discord&link=https%3A%2F%2Fdiscord.gg%2FmzWgbhGQ6C)](https://discord.gg/mzWgbhGQ6C)
[![GitHub Issues or Pull Requests](https://img.shields.io/github/issues/alexdovzhanyn/ThetaLang?style=for-the-badge)](https://github.com/alexdovzhanyn/ThetaLang/issues)
![GitHub License](https://img.shields.io/github/license/alexdovzhanyn/ThetaLang?style=for-the-badge)
![ThetaLang](https://github.com/user-attachments/assets/fd4238c5-096a-43a6-96c7-40343572b4d4)

## Introduction

Welcome to Theta! If you're into modern, functional programming languages with a clean syntax and powerful features, you're in the right place. Theta combines the best of functional programming with a strong type system and expressive syntax to help you write clean, efficient, and maintainable code. Whether you're building data-driven applications or just tinkering with new ideas, Theta is here to make your coding experience smoother and more enjoyable

[View the formal language grammar in BNF, if you're into that kind of thing.](doc/theta_grammar.bnf)

## Table of Contents

1. [Features](#features)
2. [Example Code](#example-code)
3. [Building Theta](#building-theta)
4. [Contributing](/CONTRIBUTING.md)
5. [Reporting Issues](#reporting-issues)
6. [Language Specification](#theta-language-specification)

---

## Features

- **Strong Typing**: Catch errors early and ensure type safety throughout your codebase.
- **Functional Paradigms**: Enjoy first-class functions, immutability, and higher-order functions.
- **Pattern Matching**: Simplify your code with built-in pattern matching and destructuring.
- **Modular Design**: Organize your code into reusable modules, known as capsules.
- **Interactive REPL**: Experiment with code in real-time using the Interactive Theta (ITH) REPL.

## Example Code

Check out some Theta code to get a feel for its clean and expressive syntax:

### Function Definition and Pattern Matching

```theta
capsule MathFunctions {
    add<Function<Number>> = (a<Number>, b<Number>) -> a + b

    factorial<Function<Number>> = (n<Number>) -> {
        match (n) {
            0 -> 1
            n -> n * factorial(n - 1)
        }
    }
}
```

### Structs and Lists

```theta
link Theta.IO

capsule DataStructures {
    struct Point {
        x<Number>
        y<Number>
    }

    distance<Function<Number>> = (p1<Point>, p2<Point>) -> {
        return Math.sqrt((p2.x - p1.x) ** 2 + (p2.y - p1.y) ** 2)
    }

    main<Function<String>> -> {
        points<List<Point>> = [
          @Point{ x: 0, y: 0 },
          @Point{ x: 3, y: 4 }
        ]

        dist<Number> = distance(points[0], points[1])
        Theta.IO.println("Distance: " + dist)
    }
}
```

## Building Theta

### Prerequisites
- **C++ Compiler**: Make sure you have a compiler that supports C++17.

1. **Clone the Repository**: 
  ```sh
  git clone https://github.com/alexdovzhanyn/ThetaLang.git
  cd ThetaLang
  ```
2. **Initialize Submodules**:
  ```sh
  git submodule update --init --recursive
  ```
3. **Install Wasmer**
  ```sh
    curl https://get.wasmer.io -sSfL | WASMER_DIR=lib/wasmer sh
  ```
4. **Build the Project**:
  ```sh
  ./build.sh
  ```
  or, if on Windows:
  ```sh
  ./build-windows.sh
  ```

### Verifying the Installation

To make sure Theta is set up correctly, run:

```sh
theta --version
```

You should see the current version of Theta displayed.

### Testing Changes

Theta has an Interactive Theta (ITH) REPL. Just type `theta` into the terminal. Note that it currently only supports single-line expressions. The REPL will show you the AST generated from your code.

For running Theta code, you can use the [Theta Browser Playground](https://github.com/alexdovzhanyn/theta-browser-playground).

### Reporting Issues

If you find any issues or have suggestions, please use the [Issues page](https://github.com/alexdovzhanyn/ThetaLang/issues). We appreciate your feedback and contributions!

# Theta Language Specification

## 1. Basic Concepts

### 1.1. Lexical Elements

#### 1.1.1. Identifiers
Identifiers are used to name variables, functions, capsules, and structs. They must start with a letter or underscore and can contain letters, digits, and underscores.

```ebnf
Identifier = Letter (Letter | Digit | "_")*
Letter = "A".."Z" | "a".."z" | "_"
Digit = "0".."9"
```

#### 1.1.2. Keywords
Reserved words that have special meaning in Theta:
```
link, capsule, struct, true, false, void, Number, String, Boolean, Symbol, Enum
```

#### 1.1.3. Comments
Single-line comments start with `//` and extend to the end of the line. Multi-line comments
start with `/-` and extend until reaching a `-/`.

Example:
```theta
// This is a single line comment
// This is another

/-
  This is a multiline comment.
  It extends over multiple lines.
-/
```

---

## 2. Types

### 2.1. Primitive Types
- `String`: Represented by single quotes `'example'`.
- `Boolean`: Represented by `true` or `false`.
- `Number`: Represents both integers and floating-point numbers.
- `List`: Represented by square brackets `[ ]`.
- `Dict`: Represented by curly braces `{ }`.
- `Symbol`: Represented by a colon followed by an identifier, e.g., `:symbol`.

### 2.2. Structs
User-defined data structures composed of primitives or other structs. Structs can only be defined within capsules.
Structs can be referenced by name within the capsule that they are defined, but must be prefixed by their
containing capsule if used in another capsule

```theta
struct StructName {
  fieldName<Type>
  ...
}
```

Example:
```theta
capsule Messaging {
  struct MessageRequest {
    hostname<String>
    port<Number>
    path<String>
    method<String>
    headers<MessageRequestHeaders>
  }
}

// If referenced in another capsule
Messaging.MessageRequest
```

### 2.3. Enums
Enumerated types with custom values represented as symbols. Enum names must be in Pascal case. Enums
are scoped the same as variables, therefore an enum defined in a capsule will be accessible from outside the capsule,
while an enum defined within a function will be scoped to that function.

```theta
enum EnumName {
  :ENUM_1
  :ENUM_2
  ...
}
```

Within a capsule:
```theta
capsule Networking {
  enum Status {
    :SUCCESS
    :FAILURE
    :PENDING
  }
}


// Used like so:
myVar == Networking.Status.SUCCESS

// Or like so, if being referenced from within the capsule:
myVar == Status.SUCCESS
```

Within a function:
```theta
capsule Networking {
  isNetworkRequestSuccess<Boolean> = request<NetworkRequest> -> {
    enum PassingStatuses {
      :SUCCESS
      :REDIRECT
    }

    return Enumerable.includes(PassingStatuses, request.status)
  }

  isNetworkRequestFailure<Boolean> = request<NetworkRequest> -> {
    // PassingStatuses is not available in here
  }
}
```

---

## 3. Variables and Constants

### 3.1. Variable Declaration
Variables are declared by their name, suffixed with their type, followed by an equal sign and their value. Variables are immutable.

```theta
variableName<Type> = value
```

Example:
```theta
greeting<String> = 'Hello, World'
```

---

## 4. Functions

### 4.1. Function Definition
Functions are defined as variables pointing to a block. The return type is specified after the function name.

```theta
functionName<ReturnType> = (param1<Type1>, param2<Type2>, ...) -> {
  // function body
}
```

Example:
```theta
add<Number> = (a<Number>, b<Number>) -> a + b
```

### 4.2. Function Composition
Functions can be composed using the `=>` operator, where the value on the left is passed as the first argument to the function on the right.

```theta
value => function
```

Example:
```theta
requestParams => Json.encodeStruct() => Http.request()
```

---

## 5. Capsules

### 5.1. Capsule Definition
Capsules are static groupings of related functions, variables, and structs, providing modularity and namespace management. All code in Theta must be contained within capsules.

```theta
capsule CapsuleName {
  // variable, function, and struct definitions
}
```

Example:
```theta
capsule Math {
  add<Number> = (a<Number>, b<Number>) -> a + b
  subtract<Number> = (a<Number>, b<Number>) -> a - b

  struct Point {
    x<Number>
    y<Number>
  }

  origin<Point> = @Point { x: 0, y: 0 }
}
```

### 5.2. Importing Capsules
Capsules are imported using the `link` keyword.

```theta
link CapsuleName
```

Example:
```theta
link Http
link Json
```

---

## 6. Pattern Matching

Pattern matching allows for intuitive matching of data structures.

```theta
match value {
  pattern1 -> result1
  pattern2 -> result2
  ...
  _ -> defaultResult
}
```

Example:
```theta
matchStatus<String> = status<Enum> -> {
  match status {
    :SUCCESS -> 'Operation was successful'
    :FAILURE -> 'Operation failed'
    _ -> 'Unknown status'
  }
}
```

---

## 7. Destructuring
Theta supports list, dictionary, and struct destructuring during variable assignment. It is a powerful
way to pattern match values out of variables, based on the shape of the data:

```theta
myList<List<Number>> = [ 1, 2, 3 ]

// a is 1, b is 2, and c is 3. Notice that you don't have to specify the types of
// a, b, and c here. That is because the compiler can infer the types, because it
// knows we are destructuring a List<Number>, so its values must all be of type Number
[ a, b, c ] = list

myDict<Dict<Number>> = { x: 1, y: 2, z: 3 }

// x is 1, y is 2, z is 3
{x, y, z} = dict
```

---

## 8. File Extension
Theta code files are saved with the extension `.th`

---
## 9. Example Program

Putting it all together, here is a complete example using the discussed features:

```theta
// in Math.th
capsule Math {
    struct Point {
        x<Number>
        y<Number>
    }

    distance<Number> = (point1<Point>, point2<Point>) -> {
        // Calculate distance...
    }

    dimensionalDistanceX<Number> = (point1<Point>, point2<Point>) -> {
      { x: point1X } = point1
      { x: point2X } = point2

      return point2X - point1X
    }
}

// in Main.th
link Math

capsule Main {
    point1 = @Math.Point { x: 0, y: 0 }
    point2 = @Math.Point { x: 3, y: 4 }

    distance = Math.distance(point1, point2)
}
```
