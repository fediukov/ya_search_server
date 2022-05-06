#include "string_processing.h"

using namespace std;

/*vector<string> SplitIntoWords(string text) {
    vector<string> words;
    string word;
    for (const char& c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.insert(words.end(), (word));
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.insert(words.end(), (word));
    }

    return words;
}//*/

vector<string_view> SplitIntoWords(string_view text)
{
    vector<string_view> result;
    //1. Удалите начало из str до первого непробельного символа, воспользовавшись методом remove_prefix. 
    // Он уберёт из string_view указанное количество символов.
    text.remove_prefix(std::min(text.find_first_not_of(" "), text.size()));
    //2. В цикле используйте метод find с одним параметром, 
    // чтобы найти номер позиции первого пробела.
    while (true) {
        auto space = text.find(' ');
        //3. Добавьте в результирующий вектор элемент string_view, полученный вызовом метода substr, 
        // где начальная позиция будет 0, а конечная — найденная позиция пробела или npos.
        result.push_back(space == text.npos ? text.substr(0, text.npos) : text.substr(0, space));
        //4. Сдвиньте начало str так, чтобы оно указывало на позицию за пробелом. Это можно сделать 
        // методом remove_prefix, передвигая начало str на указанное в аргументе количество позиций.
        text.remove_prefix(std::min(text.find_first_not_of(" ", space), text.size()));
        if (space == text.npos) {
            break;
        }
    }
    return result;

}
