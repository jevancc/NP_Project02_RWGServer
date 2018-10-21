#include <npshell/shell.h>
#include <npshell/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <nonstd/optional.hpp>
#include <regex>
using namespace std;
using namespace np;

int main() {
    np::Shell shell;
    shell.Run();
    return 0;
}
