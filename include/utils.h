#include <string>
#include <vector>
#include <set>

#ifndef GOL3D_UTILS_H
#define GOL3D_UTILS_H

bool in(const std::set<int> &theSet, const int &theItem);

std::vector<std::string> split(const std::string &str, const std::string &delim);

#endif //GOL3D_UTILS_H
