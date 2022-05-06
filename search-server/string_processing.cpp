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
    //1. ������� ������ �� str �� ������� ������������� �������, ���������������� ������� remove_prefix. 
    // �� ����� �� string_view ��������� ���������� ��������.
    text.remove_prefix(std::min(text.find_first_not_of(" "), text.size()));
    //2. � ����� ����������� ����� find � ����� ����������, 
    // ����� ����� ����� ������� ������� �������.
    while (true) {
        auto space = text.find(' ');
        //3. �������� � �������������� ������ ������� string_view, ���������� ������� ������ substr, 
        // ��� ��������� ������� ����� 0, � �������� � ��������� ������� ������� ��� npos.
        result.push_back(space == text.npos ? text.substr(0, text.npos) : text.substr(0, space));
        //4. �������� ������ str ���, ����� ��� ��������� �� ������� �� ��������. ��� ����� ������� 
        // ������� remove_prefix, ���������� ������ str �� ��������� � ��������� ���������� �������.
        text.remove_prefix(std::min(text.find_first_not_of(" ", space), text.size()));
        if (space == text.npos) {
            break;
        }
    }
    return result;

}
