#define NOBPP_IMPLEMENTATION
#include "nobpp.hpp"

#include <iostream>

int main(int argc, char** argv)
{
#ifdef NOBPP_INIT_SCRIPT
    nob::Command cmd = nob::Command() + std::filesystem::path(NOBPP_INIT_SCRIPT);
#else
    // this functionality is required on msvc
    #ifdef __nob_msvc__
        #error
    #endif

    nob::Command cmd;
#endif

    if (argc < 2)
    {
        std::cout << "No args...\n";
    }

    for (int i = 1; i < argc; i++)
    {
        std::cout << i << "\n";

        std::filesystem::path file = { argv[i] };
        cmd = cmd + (nob::CompileCommand()
         + nob::SourceFile{ file } + nob::CompilerFlag::CPPVersion17
         + nob::IncludeDirectory{ std::filesystem::current_path() }
         + nob::AddLinkCommand{ nob::LinkCommand() }
        );
    }

    cmd.Run();

    std::cout << "Press Enter to exit.";
    std::getline(std::cin, std::string{});

    return 0;
}
