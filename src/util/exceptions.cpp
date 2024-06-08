#include "exceptions.hpp"

void ExceptionFormatter::displayFormattedError(string errorType, ParseError &e, string source, string fileName, Token token) {
    cout << "\n" + fileName << endl;
    cout << "  \033[1;31m" + errorType + "\033[0m: " << e.what() << ':' << endl;
    
    string contextPrevLine;
    string contextErrorLine;
    string contextNextLine;
    int line = 1;
    int charIdx = 0;

    // If the error isn't on the first line of the file, we want to add a prevLine to the log output, for context.
    if (e.contextLocation()[0] > 1) {
        for (; line < e.contextLocation()[0] - 1; charIdx++) {
            if (source[charIdx] == '\n') line++;
        }

        while (source[charIdx] != '\n') {
            contextPrevLine += source[charIdx];  
            charIdx++;
        }
    }
    
    // Skip to the line where the error is
    for (; line < e.contextLocation()[0]; charIdx++) {
        if (source[charIdx] == '\n') line++;
    }

    while (source[charIdx] != '\n') {
        contextErrorLine += source[charIdx];  
        charIdx++;
    }

    // If there's another line after this one, add its contents to the nextLine for logging output
    if (charIdx++ < source.length()) {
        while (source[charIdx] != '\n') {
            contextNextLine += source[charIdx];
            charIdx++;
        }
    }

    string errorMarker(e.contextLocation()[1] + to_string(e.contextLocation()[0]).length() + 1, ' ');
    string errorPoint(token.getText().length(), '^');

    if (contextPrevLine != "") {
        cout << "    " + to_string(e.contextLocation()[0] - 1) + ": " + contextPrevLine << endl;
    }

    cout << "    " + to_string(e.contextLocation()[0]) + ": " +  contextErrorLine << endl;
    cout << "    " + errorMarker + "\033[31m" + errorPoint + "\033[0m" << endl;

    if (contextNextLine != "") {
        cout << "    " + to_string(e.contextLocation()[0] + 1) + ": " + contextNextLine << endl;
    }
}