#define NOBPP_IMPLEMENTATION
#include "../nobpp.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    nob::Init nobInit(argc, argv, __FILE__);
    nob::CLFlags.set(nob::CLArgument::Clean);

    std::cout << "Compiling build file(s).\n";

    for (std::string& str : nob::OtherCLArguments)
    {
        std::cout << str << "\n";

        std::filesystem::path file = { str };
        (nob::CompileCommand()
         + nob::SourceFile{ file } + nob::CompilerFlag::CPPVersion17
         + nob::IncludeDirectory{ std::filesystem::absolute(std::filesystem::path{ argv[0] }).parent_path() }
         + nob::AddLinkCommand{ nob::LinkCommand() }
        ).Run();
    }

    std::cout << "Done! (press enter)";
    std::getline(std::cin, std::string());

    return 0;
}
