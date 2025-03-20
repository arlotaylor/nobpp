#define NOBPP_IMPLEMENTATION
#include "nobpp.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    nob::Init(argc, argv, __FILE__);

    for (std::string& str : nob::OtherCLArguments)
    {
        std::cout << str << "\n";

        std::filesystem::path file = { str };
        (nob::CompileCommand()
         + nob::SourceFile{ file } + nob::CompilerFlag::CPPVersion17
#ifdef NOBPP_INIT_SCRIPT
         + nob::MacroDefinition{ "NOBPP_INIT_SCRIPT", NOBPP_INIT_SCRIPT }
#endif
         + nob::IncludeDirectory{ std::filesystem::current_path() }
         + nob::AddLinkCommand{ nob::LinkCommand() }
        ).Run();
    }

    return 0;
}
