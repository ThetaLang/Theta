# Theta Language Specification

## Introduction
Theta is a strongly typed, functional, compiled programming language designed to be data-driven and composable, with built-in support for pattern matching and modular organization through capsules. The language aims to provide a clean and expressive syntax while ensuring type safety and functional programming paradigms.

---

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
Single-line comments start with `//` and extend to the end of the line.

```ebnf
Comment = "//" .* '\n'
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

```theta
struct StructName {
  fieldName<Type>
  ...
}
```

Example:
```theta
struct MessageRequest {
  hostname<String>
  port<Number>
  path<String>
  method<String>
  headers<MessageRequestHeaders>
}
```

### 2.3. Enums
Enumerated types with custom values represented as symbols. Internally, enums are syntactic sugar over a dictionary where both keys and values are the same symbol.

```theta
enum EnumName {
  :ENUM_1
  :ENUM_2
  ...
}
```

Example:
```theta
enum Status {
  :SUCCESS
  :FAILURE
  :PENDING
}
```

---

## 3. Variables and Constants

### 3.1. Variable Declaration
Variables are declared by their name, suffixed with their type, followed by an equal sign and their value.

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
functionName<ReturnType> = param1<Type1>, param2<Type2>, ... -> {
  // function body
}
```

Example:
```theta
add<Number> = a<Number>, b<Number> -> a + b
```

### 4.2. Function Composition
Functions can be composed using the `=>` operator, where the value on the left is passed as the first argument to the function on the right.

```theta
value => function
```

Example:
```theta
requestParams => Json.encodeStruct => Http.request
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
  add<Number> = a<Number>, b<Number> -> a + b
  subtract<Number> = a<Number>, b<Number> -> a - b

  struct Point {
    x<Number>
    y<Number>
  }

  origin<Point> = %Point { x: 0, y: 0 }
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
  pattern1 >> result1
  pattern2 >> result2
  ...
  _ => defaultResult
}
```

Example:
```theta
matchStatus<String> = status<Enum> -> {
  match status {
    :SUCCESS >> 'Operation was successful'
    :FAILURE >> 'Operation failed'
    _ => 'Unknown status'
  }
}
```

---

## 7. Example Program

Putting it all together, here is a complete example using the discussed features:

```theta
link Http
link Json

capsule TelegramCommands {

  struct MessageRequest {
    hostname<String>
    port<Number>
    path<String>
    method<String>
    headers<MessageRequestHeaders>
  }

  struct MessageRequestHeaders {
    contentType<String>
  }

  sendMessage<void> = message<String> -> {
    requestParams<MessageRequest> = %MessageRequest {
      hostname: 'api.telegram.org',
      port: 443,
      path: '/bot#{process.env.TG_API_KEY}/sendMessage',
      method: 'POST',
      headers: %MessageRequestHeaders {
        contentType: 'application/json'
      }
    }

    requestParams
      => Json.encodeStruct
      => Http.request
  }

}
```

---

## Conclusion

Theta is designed to be a modern, strongly typed, functional programming language that emphasizes modularity through capsules and clarity through its syntax and structure. This specification outlines the core features and syntax of Theta, providing a foundation for further development and refinement.
