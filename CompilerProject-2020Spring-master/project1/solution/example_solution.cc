// this is a silly solution
// just to show you how different
// components of this framework work
// please bring your wise to write
// a 'real' solution :)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <dirent.h>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "Parser.h"
#include "json_parser.h"

using namespace Boost::Internal;
using namespace Boost;
using namespace std;

vector<string> listdir(const char *path) {
    vector<string> files;
    struct dirent *entry;
    DIR *dp;

    dp = opendir(path);
    if (dp == NULL) {
        cout<<"ERROR: opendir: Path does not exist or could not be read."<<endl;
        exit(1);
    }

    while ((entry = readdir(dp))) {
        string file=entry->d_name;
        if(file=="."||file=="..")
            continue;
        files.push_back(file);
    }
    closedir(dp);
    return files;
}

int main() {
    string inputDir="./cases/";
    string outputDir="./kernels/";
    vector<string> files=listdir("./cases");
    for(auto file:files) {
        //cout << file << endl;
        Json_Parser J;
        std::string json_path =inputDir+ file;
        J.read_file(json_path);
        J.ParseJson(1);
        Parser *parser = new Parser();
        parser->parse(J.name, J.ins, J.outs, J.data_type, J.kernel);
        IRPrinter *printer = new IRPrinter;
        printer->modified = false;
        std::string code = printer->print(parser->kernel);
        //std::cout << code;
        std::ofstream ofile(outputDir+J.name+".cc", std::ios::out);
        ofile << "#include \"../run.h\"\n" << code;
        ofile.close();
        delete parser;
        delete printer;
    }
    return 0;
}