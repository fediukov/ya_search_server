#pragma once

#include <deque>
#include <functional>
#include <list>
#include <numeric>
#include <string>
#include <vector>
#include <execution>

#include "log_duration.h"
#include "search_server.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries); //*/
