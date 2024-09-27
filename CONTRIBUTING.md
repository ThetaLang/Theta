# Contributing to ThetaLang

First, thank you for your interest in contributing to ThetaLang! This document outlines the contribution process, from reporting issues to proposing improvements. We value your input and hope this guide helps you get started easily.

## Table of Contents

1. [How to Contribute](#how-to-contribute)
2. [Development Workflow](#development-workflow)
3. [Code Guidelines](#code-guidelines)
4. [Running Tests](#running-tests)
5. [Submitting Changes](#submitting-changes)
6. [Reporting Issues](#reporting-issues)

---

## How to Contribute

There are several ways to contribute to ThetaLang:

- Reporting bugs and issues.
- Suggesting new features or enhancements.
- Writing code to fix bugs or add features.
- Improving documentation and examples.

### Steps for Contributing Code:

1. **Fork the Repository**: Click the "Fork" button on the top-right corner of the [ThetaLang repository](https://github.com/alexdovzhanyn/ThetaLang) and clone your fork locally.
    ```sh
    git clone https://github.com/YOUR_GITHUB_USERNAME/ThetaLang.git
    cd ThetaLang
    ```

2. **Create a Branch**: Create a new branch for your feature or bugfix.
    ```sh
    git checkout -b feature-or-bugfix-name
    ```

3. **Make Your Changes**: Modify the code, make improvements, and commit your changes. Make sure to follow the coding standards listed in [Code Guidelines](#code-guidelines).

4. **Run Tests**: Ensure all tests pass before submitting. Learn how to run tests in the [Running Tests](#running-tests) section.

5. **Submit a Pull Request**: Push your changes to your fork and submit a pull request (PR) to the `master` branch of the original repository. Be sure to include details about what your PR addresses, any related issues, and steps for verification.

---

## Development Workflow

The following steps outline the typical workflow for contributing to ThetaLang:

1. **Sync Your Fork**: Before working on a new contribution, always ensure your fork is up-to-date.
    ```sh
    git fetch upstream
    git checkout master
    git merge upstream/master
    ```

2. **Write Code**: Make sure your changes are aligned with ThetaLang’s goals and existing architecture. Refer to the [Code Guidelines](#code-guidelines) for coding standards.

3. **Commit Messages**: Write clear and concise commit messages that explain the changes made. Use present tense, and separate different types of changes into multiple commits if needed.
    - Example: `Add support for new syntax in pattern matching`.

4. **Pull Requests**: Submit your pull request to the `master` branch and describe what changes were made. Link to relevant issues if applicable.

---

## Code Guidelines

To ensure consistency and maintainability across the codebase, follow these guidelines:

- **Code Formatting**: Follow the existing coding style. ThetaLang uses C++17, so adhere to modern C++ practices (e.g., use `auto` when appropriate, prefer smart pointers).
- **Naming Conventions**:
  - **Variables, Functions, and Methods**: Use `camelCase` for variable names, and method names.
  - **Classes and Structs**: Use `PascalCase` for classes and structs.
  - **Constants**: Use `SCREAMING_SNAKE_CASE` for constants.

### Example:

```cpp
class MyClass {
public:
    void myFunction() {
        int myVariable = 42;
        // ...
    }
};
```

- **Documentation**: Comment your code, especially for complex functions or classes. Provide a description of what each function does and what parameters are expected.

---

## Running Tests

Before submitting any changes, make sure all tests pass. ThetaLang includes several tests for its lexer, parser, and compiler components. Here's how to run them:

1. **Build the Project**:
    ```sh
    ./build.sh
    ```

2. **Run Unit Tests**:
    ```sh
    ./build/LexerTest
    ./build/ParserTest
    ./build/CompilerTest
    ```

3. **Using the REPL for Testing**: You can also use the Interactive Theta (ITH) REPL to quickly check if your changes work as expected:
    ```sh
    ./theta
    ```

For more complex testing, use the [Theta Browser Playground](https://github.com/alexdovzhanyn/theta-browser-playground) to execute your code and visualize the results.

---

## Submitting Changes

Once you've made your changes, tested them, and ensured they meet the code guidelines, it's time to submit a pull request:

1. **Push Your Branch**:
    ```sh
    git push origin your-branch-name
    ```

2. **Open a Pull Request**: Go to the GitHub repository, and you’ll see a prompt to create a pull request. Follow the steps to create a PR targeting the `master` branch of the original repository.

3. **Review Process**: A project maintainer will review your changes, ask questions if needed, and may request modifications. Once approved, your PR will be merged into the project!

---

## Reporting Issues

If you encounter a bug or have suggestions for improvement, we encourage you to report them through GitHub Issues.

### Steps to Report an Issue:

1. **Search for Existing Issues**: Before submitting a new issue, search for similar reports to avoid duplicates.
   
2. **Create a New Issue**: If no similar issues exist, create a new issue. Please include:
   - A clear description of the problem.
   - Steps to reproduce the problem (if applicable).
   - Any relevant logs or screenshots.
   - Your environment (OS, compiler version, etc.).

3. **Feature Requests**: If you’re suggesting a new feature, clearly explain what you want to achieve and why it’s important. Provide examples if possible.

---

Thank you for contributing to ThetaLang! We look forward to collaborating with you. If you have any questions, feel free to reach out through the GitHub Issues or Discussions pages.

