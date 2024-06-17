#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#import <string>
#import <vector>
#import <iostream>
#import "../lexer/token.hpp"

using namespace std;

class ThetaCompilationError : public exception {
    private:
        string message;
        vector<int> sourceLocation;

    public:
        ThetaCompilationError(string msg, vector<int> loc) : message(msg), sourceLocation(loc) {};

        string what() {
            return message + " at line " + to_string(sourceLocation[0]) + ", column " + to_string(sourceLocation[1]);
        }

        vector<int> contextLocation() { return sourceLocation; }
};

class SyntaxError : public ThetaCompilationError {
    using ThetaCompilationError::ThetaCompilationError;
};

class LinkageError : public ThetaCompilationError {
    using ThetaCompilationError::ThetaCompilationError;
};

class ParseError : public ThetaCompilationError {
    using ThetaCompilationError::ThetaCompilationError;
};

class ExceptionFormatter {
    public:
        static void displayFormattedError(string errorType, ThetaCompilationError &e, string source, string fileName, Token token);
};

#endif