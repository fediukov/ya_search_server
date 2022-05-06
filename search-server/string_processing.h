#pragma once

#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

std::vector<std::string_view> SplitIntoWords(std::string_view text);

template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string, std::less<>> non_empty_strings;
    // new version
    /*if constexpr (std::is_convertible_v<StringContainer, std::string>)
    {
        for(auto const& word : SplitIntoWords(strings))
        {
            non_empty_strings.insert(std::string(word));
        }
    }
    else
    {
        for(auto const& word : strings)
        {
            non_empty_strings.insert(word);
        }
    }//*/
    // old version
    for (const auto& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(std::string(str));
        }
    }//*/
    return non_empty_strings;
}