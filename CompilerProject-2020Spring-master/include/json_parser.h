#ifndef BOOST_JSON_H
#define BOOST_JSON_H
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>
#include <cstring>
#include <fstream>
#include <vector>

namespace Boost{

    class Json_Parser{
    public:
        std::string name;
        std::vector <std::string> ins;
        std::string outs;
        std::string data_type;
        std::string kernel;
        std::string F;
        std::vector <std::string> grad_to;

        void read_file(std::string path){
            std::ifstream inf;
            inf.open(path);
            std::string line;
            while(getline(inf,line)){
                F+=line;
            }
        }

        int ParseJson(int project){
            using namespace boost::property_tree;

            std::stringstream ss(F);
            ptree pt;
            try{
                read_json(ss,pt);
            }catch(ptree_error & e){
                return 1;
            }

            name = pt.get<std::string> ("name");
            ptree ptChildRead = pt.get_child("ins");
            for(BOOST_AUTO(pos,ptChildRead.begin());pos!=ptChildRead.end();++pos){
                std::string ins_item = pos->second.get_value<std::string>();
                ins.push_back(ins_item);
            }
            ptChildRead = pt.get_child("outs");
            for(BOOST_AUTO(pos,ptChildRead.begin());pos!=ptChildRead.end();++pos){
                std::string out_item = pos->second.get_value<std::string>();
                outs = out_item;
            }
            data_type = pt.get<std::string> ("data_type");
            kernel = pt.get<std::string> ("kernel");

            if (project == 2)
            {
                ptChildRead = pt.get_child("grad_to");
                for(BOOST_AUTO(pos,ptChildRead.begin());pos!=ptChildRead.end();++pos){
                    std::string grad_to_item = pos->second.get_value<std::string>();
                    grad_to.push_back(grad_to_item);
                }
            }

            return 0;
        }
    };

}

#endif