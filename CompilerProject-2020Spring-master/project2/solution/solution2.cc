// this is a silly solution
// just to show you how different
// components of this framework work
// please bring your wise to write
// a 'real' solution :)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <dirent.h>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "../src/DParser.cc"
#include "Parser.h"
#include "json_parser.h"
#include "../src/gradBuilder.cc"

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
        if(file=="."||file==".."||file=="caseall.cc")
            continue;
        files.push_back(file);
    }
    closedir(dp);
    return files;
}

void setInOut(set<string> varId, vector<string> grad_to, vector<string> &ins, string &out)
{
    out = "d" + grad_to[grad_to.size() - 1];
    set<string>::iterator it = varId.find(out);
    varId.erase(it);
    ins.clear();
    vector<string> dTail;
    for (it = varId.begin(); it != varId.end(); it++)
    {
        if ((*it)[0] == 'd')
            dTail.push_back(*it);
        else
            ins.push_back(*it);
    }
    for (int i = 0; i < dTail.size(); ++i)
        ins.push_back(dTail[i]);
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
        J.ParseJson(2);
        DParser *dparser = new DParser();
        dparser->parse(J.name, J.ins, J.outs, J.data_type, J.kernel);
        J.kernel = "";
        //cout << J.name << " " << dKernel;
        set<string> varIdSet;
        gradBuilder *mutator;
        IRPrinter *dPrinter;
        varMutator *varAdder;
        Stmt statement;
        string dKernel;
        for (int i = 0; i < J.grad_to.size(); ++i)
        {
            mutator = new gradBuilder();
            dPrinter = new IRPrinter();
            varAdder = new varMutator();
            dPrinter->modified = true;
            mutator->gradToVarName = J.grad_to[i];
            Group dontcare = mutator->mutate(dparser->kernel);

            statement = varAdder->mutate(mutator->adjointStmt);
            dKernel = dPrinter->print(statement);

            J.kernel = J.kernel + dKernel;
            varIdSet.insert(varAdder->varSet.begin(), varAdder->varSet.end());
            delete mutator;
            delete dPrinter;
            delete varAdder;
        }
        setInOut(varIdSet, J.grad_to, J.ins, J.outs);

        Parser *parser = new Parser();
        cout <<J.name<<"_" <<J.kernel;
        cout <<J.ins.size();
        for (int i = 0; i < J.ins.size(); ++i)
           cout << "_" << J.ins[i] << "_";
        cout << "\n" << "_" << J.outs << "_\n";
        int r = J.kernel.find('\r\n');
        while (r != string::npos)
        {
            if (r != string::npos)
            {
                J.kernel.replace(r, 1, "");
                r = J.kernel.find('\r\n');
            }
        }
        parser->parse(J.name, J.ins, J.outs, J.data_type, J.kernel);
        IRPrinter *printer = new IRPrinter();
        printer->modified = false;
        std::string code = printer->print(parser->kernel);
        //std::cout << code;
        std::ofstream ofile(outputDir+J.name+".cc", std::ios::out);
        ofile << "#include \"../run2.h\"\n" << code;
        ofile.close();
        delete dparser;
        delete parser;
        delete printer;
    }
    return 0;


}
