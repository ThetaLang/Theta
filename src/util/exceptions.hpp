#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#import <string>
#import <vector>
#import <iostream>
#import "../lexer/token.hpp"

using namespace std;

class ParseError : public exception {
    public:
        virtual string what() = 0;
        virtual vector<int> contextLocation() = 0;
};

class SyntaxError : public ParseError {
    private:
        string message;
        vector<int> sourceLocation;

    public:
        SyntaxError(string msg, vector<int> loc) : message(msg), sourceLocation(loc) {}
        string what() override {
            return message + " at line " + to_string(sourceLocation[0]) + ", column " + to_string(sourceLocation[1]);
        }
        vector<int> contextLocation() override { return sourceLocation; }
};

class LinkageError : public ParseError {
    private:
        string message;
        vector<int> sourceLocation;
    public:
        LinkageError(string msg, vector<int> loc) : message(msg), sourceLocation(loc) {}
        
        string what() override {
            return message + " at line " + to_string(sourceLocation[0]) + ", column " + to_string(sourceLocation[1]);
        }

        vector<int> contextLocation() override { return sourceLocation; }
};

class ExceptionFormatter {
    public:
        static void displayFormattedError(string errorType, ParseError &e, string source, string fileName, Token token);
};

#endif