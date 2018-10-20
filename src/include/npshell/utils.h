#include <string>
#include <cctype>
#include <algorithm>
using namespace std;

namespace np {
namespace utils {
inline string& ltrim(string& s) {
    s.erase(s.begin(),
            find_if(s.begin(), s.end(), [](int ch) { return !isspace(ch); }));
    return s;
}

inline string& rtrim(string& s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](int ch) { return !isspace(ch); })
                .base(),
            s.end());
    return s;
}

inline string& trim(string& s) {
    ltrim(s);
    rtrim(s);
    return s;
}
}  // namespace utils
}  // namespace np
