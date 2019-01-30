#include <cstdio>

int main () {
  int count = 0;
  char c;
  while ((c = getchar()) != EOF) {
    if (c == '0') {
      ++count;
    }
  }
  printf("%d\n", count);
  return 0;
}
