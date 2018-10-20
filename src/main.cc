#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <nonstd/optional.hpp>
#include <regex>
#include <npshell/types.h>
using namespace std;
using namespace np;

int main() {
    auto x = np::Command("./bin/ls aaa bbb  |    cat > ggg.txt");
    for (auto c: x.Parse()) {
        string path;
        vector<string> args;
        vector<IOOption> io;
        tie(path, args, io) = c;
        cout <<"--------------" << endl;
        cout << path << endl;
        for (int i=0;i<args.size(); i++){
            cout << "arg: "<<args[i] << endl;
        }
        cout << io[0].type << endl;
        cout << io[1].type << endl;
        cout << io[2].type << endl;
    }
    return 0;
}
