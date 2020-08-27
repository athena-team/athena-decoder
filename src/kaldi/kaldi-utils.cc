#ifdef _WIN32_WINNT_WIN8
#include <Synchapi.h>
#elif defined(_WIN32) || defined(_MSC_VER) || defined(MINGW)
#include <Windows.h>
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif /* _MSC_VER < 1900 */
#else
#include <unistd.h>
#endif

#include <string>
#include <stdio.h>
#include "kaldi/kaldi-utils.h"


namespace kaldi {

std::string CharToString(const char &c) {
  char buf[20];
  if (std::isprint(c))
    snprintf(buf, sizeof(buf), "\'%c\'", c);
  else
    snprintf(buf, sizeof(buf), "[character %d]", static_cast<int>(c));
  return (std::string) buf;
}

void Sleep(float seconds) {
#if defined(_MSC_VER) || defined(MINGW)
  ::Sleep(static_cast<int>(seconds * 1000.0));
#else
  usleep(static_cast<int>(seconds * 1000000.0));
#endif
}

}  // end namespace kaldi
