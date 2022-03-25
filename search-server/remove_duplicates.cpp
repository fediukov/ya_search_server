#include "remove_duplicates.h"

#include <map>
#include <string>
#include <set>

void RemoveDuplicates(SearchServer& search_server) {
	// right version
	std::set<std::set<std::string>> doc_map;
	for (auto it = search_server.begin(); it != search_server.end();)
	{
		std::set<std::string> check;
		for (const auto& [word, freq] : search_server.GetWordFrequencies(*it))
		{
			//check = search_server.GetWordFrequencies(*it)::id_to_words;
			check.insert(word);
		} 
		//extern std::set<std::string> id_to_words;
		if (doc_map.count(check))
		{
			std::cout << "Found duplicate document id " << *it << std::endl;
			// auto id_del = it++;
			search_server.RemoveDocument(*it++);
		}
		else
		{
			doc_map.insert(check);
			++it;
		}
	}
	/* // my version
	std::map<int, std::set<std::string>> doc_map;
	for (auto it = search_server.begin(); it != search_server.end(); ++it)
	{
		for (const auto& [key, value] : search_server.GetWordFrequencies(*it))
		{
			doc_map[*it].emplace(key);
		}
	}

	for (auto it = doc_map.begin(); it != doc_map.end(); ++it)
	{
		for (auto it_next = next(it, 1); it_next != doc_map.end();)
		{
			if ((*it_next).second == (*it).second)
			{
				std::cout << "Found duplicate document id " << (*it_next).first << std::endl;
				search_server.RemoveDocument((*it_next).first);
				auto it_del = it_next++;
				doc_map.erase(it_del);
			}
			else
			{
				++it_next;
			}
		}
	}
	*/
}