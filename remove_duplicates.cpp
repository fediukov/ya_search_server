#include "remove_duplicates.h"

#include <map>
#include <string>
#include <string_view>
#include <set>

void RemoveDuplicates(SearchServer& search_server) {
	std::set<std::set<std::string_view>> doc_map;
	for (auto it = search_server.begin(); it != search_server.end();)
	{
		std::set<std::string_view> check;
		for (const auto& [word, freq] : search_server.GetWordFrequencies(*it))
		{
			check.insert(word);
		} 
		if (doc_map.count(check))
		{
			std::cout << "Found duplicate document id " << *it << std::endl;
			search_server.RemoveDocument(*it++);
		}
		else
		{
			doc_map.insert(check);
			++it;
		}
	}
}