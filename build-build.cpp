#define NOBPP_IMPLEMENTATION
#include "nobpp.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    nob::Init nobInit(argc, argv, __FILE__);

    for (std::string& str : nob::OtherCLArguments)
    {
        std::cout << str << "\n";

        std::filesystem::path file = { str };
        (nob::CompileCommand()
         + nob::SourceFile{ file } + nob::CompilerFlag::CPPVersion17
#ifdef NOBPP_INIT_SCRIPT
         + nob::MacroDefinition{ "NOBPP_INIT_SCRIPT", NOBPP_INIT_SCRIPT }
#endif
         + nob::IncludeDirectory{ std::filesystem::absolute(std::filesystem::path{ argv[0] }).parent_path() }
         + nob::AddLinkCommand{ nob::LinkCommand() }
        ).Run();
    }

    // std::getline(std::cin, std::string());

    return 0;
}
