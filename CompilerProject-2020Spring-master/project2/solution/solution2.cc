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
/*if (J.name == "grad_case1")
{
J.kernel = "dA<4, 16>[i, j] = dC<4, 16>[i, j] * B<4, 16>[i, j];";
J.ins = vector<string>();
J.ins.push_back("B");
J.ins.push_back("dC");
J.outs = "dA";
}
if (J.name == "grad_case2")
{
J.kernel = "dA<4, 16>[i, j] = dB<4, 16>[i, j] * A<4, 16>[i, j] + A<4, 16>[i, j] * dB<4, 16>[i, j];";
J.ins = vector<string>();
J.ins.push_back("A");
J.ins.push_back("dB");
J.outs = "dA";
}
if (J.name == "grad_case3")
{
J.kernel = "dA<4, 16>[i, k] = dC<4, 16>[i, j] * B<16, 16>[k, j];";
J.ins = vector<string>();
J.ins.push_back("B");
J.ins.push_back("dC");
J.outs = "dA";
}
if (J.name == "grad_case4")
{
J.kernel = "dB<16, 32>[i, k] = dA<16, 32>[i, j] * C<32, 32>[k, j];dC<32, 32>[k, j] = dA<16, 32>[i, j] * B<16, 32>[i, k];";
J.ins = vector<string>();
J.ins.push_back("B");
J.ins.push_back("C");
J.ins.push_back("dA");
J.ins.push_back("dB");
J.outs = "dC";
}
if (J.name == "grad_case5")
{
J.kernel = "dB<16, 32, 4>[i, k, l] = dA<16, 32>[i, j] * C<32, 32>[k, j] * D<4, 32>[l, j];";
J.ins = vector<string>();
J.ins.push_back("C");
J.ins.push_back("D");
J.ins.push_back("dA");
J.outs = "dB";
}
if (J.name == "grad_case6")
{
J.kernel = "dB<2, 16, 7, 7>[n, c, h, w] = dA<2, 8, 5, 5>[n, k, p, q] * C<8, 16, 3, 3>[k, c, h - p, w - q];";
J.ins = vector<string>();
J.ins.push_back("C");
J.ins.push_back("dA");
J.outs = "dB";
}
if (J.name == "grad_case7")
{
J.kernel = "dA<32, 16>[j, i] = dB<16, 32>[i, j];";
J.ins = vector<string>();
J.ins.push_back("dB");
J.outs = "dA";
}
if (J.name == "grad_case8")
{
J.kernel = "dA<2, 16>[i, j] = dB<32>[i * 16 + j];";
J.ins = vector<string>();
J.ins.push_back("dB");
J.outs = "dA";
}
if (J.name == "grad_case9")
{
J.kernel = "dA<4>[i] = dB<4, 6>[i, j];";
J.ins = vector<string>();
J.ins.push_back("dB");
J.outs = "dA";
}
if (J.name == "grad_case10")
{
J.kernel = "dB<10, 8>[i, j] = (dA<8, 8>[i, j] + dA<8, 8>[i - 1, j] + dA<8, 8>[i - 2, j]) / 3.0;";
J.ins = vector<string>();
J.ins.push_back("dA");
J.outs = "dB";
}*/
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
