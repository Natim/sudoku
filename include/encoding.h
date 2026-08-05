#pragma once

#include <string>

// Graphlib text rendering expects ISO-8859-1; source files are UTF-8.
std::string utf8ToLatin1(const std::string & utf8);
