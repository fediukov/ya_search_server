#include "string_processing.h"

using namespace std;

vector<string_view> SplitIntoWords(string_view text)
{
    vector<string_view> result;
    text.remove_prefix(std::min(text.find_first_not_of(" "), text.size()));
    while (true) {
        auto space = text.find(' ');
        result.push_back(space == text.npos ? text.substr(0, text.npos) : text.substr(0, space));
        text.remove_prefix(std::min(text.find_first_not_of(" ", space), text.size()));
        if (space == text.npos) {
            break;
        }
    }
    return result;

}
