#pragma once
#include <vector>
#include "../model/PatientRecord.h"
#include "../utils/StringUtils.h"

namespace clinic {
namespace SortEngine {

enum class SortField { Name, Id, Age };

inline bool lessThan(const PatientRecord* a, const PatientRecord* b, SortField f) {
    switch (f) {
        case SortField::Name: return toLower(a->getName()) < toLower(b->getName());
        case SortField::Id:   return a->getId() < b->getId();
        default:              return a->getAge() < b->getAge();
    }
}

// INSERTION SORT: simple, fast on small or nearly-sorted lists.
inline void insertionSort(std::vector<PatientRecord*>& v, SortField f) {
    for (size_t i = 1; i < v.size(); i++) {
        PatientRecord* key = v[i];
        int j = static_cast<int>(i) - 1;
        while (j >= 0 && lessThan(key, v[j], f)) {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = key;
    }
}

// MERGE SORT: always O(n log n), good for large lists. Uses recursion.
inline void mergeSortRange(std::vector<PatientRecord*>& v, int left, int right, SortField f) {
    if (left >= right) return;                           // base case
    int mid = left + (right - left) / 2;
    mergeSortRange(v, left, mid, f);                     // sort left half
    mergeSortRange(v, mid + 1, right, f);                // sort right half

    std::vector<PatientRecord*> temp;                    // merge the two halves
    int i = left, j = mid + 1;
    while (i <= mid && j <= right) {
        if (lessThan(v[j], v[i], f)) temp.push_back(v[j++]);
        else                         temp.push_back(v[i++]);
    }
    while (i <= mid)   temp.push_back(v[i++]);
    while (j <= right) temp.push_back(v[j++]);
    for (size_t k = 0; k < temp.size(); k++) v[left + k] = temp[k];
}

inline void mergeSort(std::vector<PatientRecord*>& v, SortField f) {
    if (!v.empty()) mergeSortRange(v, 0, static_cast<int>(v.size()) - 1, f);
}
}
}