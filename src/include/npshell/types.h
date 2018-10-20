#include <map>
#include <nonstd/optional.hpp>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;
using nonstd::optional;

namespace np {

class Pipe {
   private:
    int fd_[2];

   public:
    static optional<Pipe> Create();
    Pipe();

    optional<int> In();
    optional<int> Out();
    optional<int> GetIn();
    optional<int> GetOut();
    int Close();

    inline optional<int> I() { return this->In(); }
    inline optional<int> O() { return this->Out(); }
    inline optional<int> GetI() { return this->GetIn(); }
    inline optional<int> GetO() { return this->GetOut(); }
};

enum IO {
    kInherit = 0,
    kPipe,
    kFile,
};

struct IOOption {
    IO type;
    string file;
    int line;
};

class Command {
   private:
    string raw_;
    vector<tuple<string, vector<string>, vector<IOOption>>> parsed_commands_;

   public:
    Command(string command);
    const vector<tuple<string, vector<string>, vector<IOOption>>>& Parse() {
        return this->parsed_commands_;
    }
};

class Environment {
   public:
    map<string, string> parameters;
};

}  // namespace np
