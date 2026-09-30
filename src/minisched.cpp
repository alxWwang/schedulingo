#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

void run_funcs(std::vector<std::string>& args){
    printf("Job Title: %s\n", args[0].c_str());
    printf("Time limit: %d\n", stoi(args[1]));
    printf("Command: ");
    for (size_t i = 2; i < args.size(); i ++ ){
        std::cout << (args[i]) << " ";
    }
    printf("\n\n");
}

int main(int argc, char *argv[]){
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << "<file>";
        return 1;
    }
    std::string path = argv[1];
    std::ifstream incoming_job(path);
    std::string line;
    if (!incoming_job) {
        std::cerr << "File " << argv[1] << " is unavailable\n";
        return 1;
    }

    while (std::getline(incoming_job, line)){
        if (line.size() == 0 || line[0] == '#') continue;
        std::istringstream in(line);
        std::string word;
        std::vector<std::string> tokens;
        while (in >> word){
            tokens.push_back(word);
        }
        if (tokens.size() > 2){
            run_funcs(tokens);
        }
    }
    return 0;
}

