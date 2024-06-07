#include <string>
#include <vector>

using namespace std;

class SyntaxError : public exception {
    private:
        string message;
        vector<int> sourceLocation;

    public:
        SyntaxError(string msg, vector<int> loc) : message(msg), sourceLocation(loc) {}
        string what() {
            return message + " at line " + to_string(sourceLocation[0]) + ", column " + to_string(sourceLocation[1]);
        }
        vector<int> contextLocation() { return sourceLocation; }
};