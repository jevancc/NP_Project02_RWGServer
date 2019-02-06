#ifndef _NP_SHELL_UTILS_H_
#define _NP_SHELL_UTILS_H_

#include <algorithm>
#include <cctype>
#include <string>
using namespace std;

namespace np {
namespace utils {
inline string& ltrim(string& s) {
  s.erase(s.begin(),
          find_if(s.begin(), s.end(), [](int ch) { return !isspace(ch); }));
  return s;
}

inline string& rtrim(string& s) {
  s.erase(
      find_if(s.rbegin(), s.rend(), [](int ch) { return !isspace(ch); }).base(),
      s.end());
  return s;
}

inline string& trim(string& s) {
  ltrim(s);
  rtrim(s);
  return s;
}

inline bool is_prefix(const string& s1, const string& s2) {
  return s2.substr(0, s1.size()) == s1;
}

template <class T>
ssize_t SafeSend(int fd, const T& v) {
  const size_t kSendBufferSize = 512;
  for (size_t i = 0; i < v.size();) {
    size_t t = send(fd, &v[0] + i, min(kSendBufferSize, v.size() - i), 0);
    if (t != size_t(-1)) {
      i += t;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // keep
    } else if (errno == EINTR) {
      // keep
    } else {
      return ssize_t(-1);
    }
  }
  return v.size();
}

}  // namespace utils
}  // namespace np

#endif
