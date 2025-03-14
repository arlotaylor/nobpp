#define NOBPP_IMPLEMENTATION
#include "nobpp.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    nob::Init(argc, argv, __FILE__);

    std::cout << "Compiling " << (std::filesystem::current_path() / "src").string() << std::endl;

    if (std::filesystem::remove_all(std::filesystem::current_path() / "bin"))
    {
        std::cout << "Pre-existing binaries deleted.\n";
    }
    std::filesystem::create_directories(std::filesystem::current_path() / "bin" / "int");

    nob::CompileDirectory(std::filesystem::current_path() / "src", std::filesystem::current_path() / "bin" / "int");

    nob::LinkDirectory(std::filesystem::current_path() / "bin" / "int", std::filesystem::current_path() / "bin" / "Main.exe");

    return 0;
}
