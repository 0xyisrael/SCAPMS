#pragma once
#include <vector>
#include <string>
#include "../model/PatientRecord.h"
#include "../utils/StringUtils.h"

namespace clinic {
namespace SearchEngine {

// LINEAR SEARCH: works on unsorted data. Returns index or -1.
inline int linearSearchById(const std::vector<PatientRecord*>& v, int id) {
    for (size_t i = 0; i < v.size(); i++)
        if (v[i]->getId() == id) return static_cast<int>(i);
    return -1;
}

// BINARY SEARCH: the vector MUST already be sorted by ID. Returns index or -1.
inline int binarySearchById(const std::vector<PatientRecord*>& v, int id) {
    int low = 0, high = static_cast<int>(v.size()) - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int midId = v[mid]->getId();
        if (midId == id) return mid;
        if (midId < id) low = mid + 1;
        else            high = mid - 1;
    }
    return -1;
}

// RECURSIVE SEARCH: finds every patient whose name contains the keyword.
// The function calls itself with index + 1 until it reaches the end.
inline void recursiveSearchByName(const std::vector<PatientRecord*>& v,
                                  const std::string& keyword,
                                  size_t index,
                                  std::vector<int>& results) {
    if (index >= v.size()) return;                       // base case
    if (toLower(v[index]->getName()).find(toLower(keyword)) != std::string::npos)
        results.push_back(static_cast<int>(index));
    recursiveSearchByName(v, keyword, index + 1, results);   // recursive call
}
}
}