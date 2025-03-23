#define NOBPP_IMPLEMENTATION
#include "nobpp.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    nob::Init nobInit(argc, argv, __FILE__);

    nob::Log("Compiling " + (std::filesystem::current_path() / "src").string() + "\n", nob::LogType::Info);

    if (std::filesystem::remove_all(std::filesystem::current_path() / "bin"))
    {
        nob::Log("Pre-existing binaries deleted.\n", nob::LogType::Info);
    }
    std::filesystem::create_directories(std::filesystem::current_path() / "bin" / "int");

    nob::CompileDirectory(std::filesystem::current_path() / "src", std::filesystem::current_path() / "bin" / "int");

    nob::LinkDirectory(std::filesystem::current_path() / "bin" / "int", std::filesystem::current_path() / "bin" / "Main.exe");

    return 0;
}
