#include "File.h"
#include "Index.h"
#include "LineSink.h"

#include <tclap/CmdLine.h>

#include <iostream>

using namespace std;
namespace tc = TCLAP;

namespace {

struct PrintSink : LineSink {
    void onLine(size_t l, size_t, const char *line, size_t length) override {
        cout << l << ":" << string(line, length) << endl;
    }
};

}

int Main(int argc, const char *argv[]) {
    tc::CmdLine cmd("Lookup indices in a compressed text file");
    tc::UnlabeledValueArg<string> inputFile(
            "input-file",  "Read input from <file>", true, "", "<file>", cmd);
    tc::UnlabeledMultiArg<uint64_t> query(
            "query",  "Query for <query>", false, "<query>", cmd);
    tc::SwitchArg verbose("v", "verbose", "Be more verbose", cmd);

    cmd.parse(argc, argv);

    auto compressedFile = inputFile.getValue();
    File in(fopen(compressedFile.c_str(), "rb"));
    if (in.get() == nullptr) {
        cerr << "could not open " << compressedFile << " for reading" << endl;
        return 1;
    }

    auto indexFile = inputFile.getValue() + ".zindex";
    File inIndex(fopen(indexFile.c_str(), "rb"));
    if (inIndex.get() == nullptr) {
        cerr << "could not open " << indexFile << " for reading" << endl;
        return 1;
    }
    auto index = Index::load(move(in), move(inIndex));

    PrintSink sink;
    for (auto &q: query.getValue()) {
        index.getLine(q, sink);
    }

    return 0;
}

int main(int argc, const char *argv[]) {
    try {
        return Main(argc, argv);
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
}