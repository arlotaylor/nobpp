#ifndef NOBPP_HEADER
#define NOBPP_HEADER
// nobpp header

#include <filesystem>
#include <functional>

namespace nob
{
    struct Command
    {
        std::string text;
        std::filesystem::path path = std::filesystem::current_path();

        void Run(bool suppressOutput = true);
    };

    Command operator+(Command a, std::string b);
    Command operator-(Command a, std::string b);
    Command operator+(Command a, std::filesystem::path b);
    Command operator-(Command a, std::filesystem::path b);

    template<typename T> void ParallelForEach(std::vector<T> vec, std::function<void(T)> fn, bool runAsync = true);


    struct CompileCommand : public Command
    {
        CompileCommand(std::filesystem::path path = std::filesystem::current_path());
        CompileCommand(Command cmd);
    };

    struct LinkCommand : public Command
    {
        LinkCommand(std::filesystem::path path = std::filesystem::current_path());
        LinkCommand(Command cmd);
    };

    struct LibraryCommand : public Command
    {
        LibraryCommand(std::filesystem::path path = std::filesystem::current_path());
        LibraryCommand(Command cmd);
    };

    struct SourceFile { std::filesystem::path path; };
    struct ObjectFile { std::filesystem::path path; };
    struct IncludeDirectory { std::filesystem::path path; };

    struct StaticLibraryFile { std::filesystem::path path; };  // is either .lib or .a
    struct DynamicLibraryFile { std::filesystem::path path; };  // is either .dll, .so or .dylib

    struct ExecutableFile { std::filesystem::path path; };
    struct AddLinkCommand { LinkCommand lc; };  // it is really annoying that this has to exist, but CompilerCommand + LinkCommand is ambiguous because LinkCommand casts to a path

    enum class CompilerFlag { OptimizeSpeed, OptimizeSpace, KeepLinker, Debug, PositionIndependentCode };
    enum class LinkerFlag { OutputDynamicLibrary, Debug };
    struct CustomCompilerFlag { std::string flag; };
    struct CustomLinkerFlag { std::string flag; };


    CompileCommand operator+(CompileCommand a, SourceFile b);
    CompileCommand operator+(CompileCommand a, ObjectFile b);
    CompileCommand operator+(CompileCommand a, IncludeDirectory b);
    CompileCommand operator+(CompileCommand a, CompilerFlag b);
    CompileCommand operator+(CompileCommand a, CustomCompilerFlag b);
    CompileCommand operator+(CompileCommand a, AddLinkCommand b);

    LinkCommand operator+(LinkCommand a, ObjectFile b);
    LinkCommand operator+(LinkCommand a, StaticLibraryFile b);
    LinkCommand operator+(LinkCommand a, DynamicLibraryFile b);
    LinkCommand operator+(LinkCommand a, ExecutableFile b);
    LinkCommand operator+(LinkCommand a, LinkerFlag b);
    LinkCommand operator+(LinkCommand a, CustomLinkerFlag b);

    LibraryCommand operator+(LibraryCommand a, ObjectFile b);
    LibraryCommand operator+(LibraryCommand a, StaticLibraryFile b);

    extern CompileCommand DefaultCompileCommand;
    extern LinkCommand DefaultLinkCommand;
    extern bool IsVerbose;

    void CompileDirectory(std::filesystem::path src, std::filesystem::path obj, CompileCommand cmd = DefaultCompileCommand, bool runAsync = false);
    void LinkDirectory(std::filesystem::path obj, std::filesystem::path exe, LinkCommand cmd = DefaultLinkCommand);

    void Init(int argc, char** argv, std::string srcName, bool askForFlags = false);
}

#endif

#define NOBPP_IMPLEMENTATION  // delete this
#ifdef NOBPP_IMPLEMENTATION
// nobpp implementation

#include <thread>
#include <iostream>
#include <string>
#include <ciso646>

#if defined(__clang__)
#define __nob_clang__
#elif defined(_MSC_VER)
#define __nob_msvc__
#elif defined(__GNUC__)
#define __nob_gcc__
#endif


namespace nob
{
    void Command::Run(bool suppressOutput)
    {
        if (IsVerbose)
        {
            suppressOutput = false;
        }

        std::cout << "[RUN] " << text;
        if (!suppressOutput)
        {
            std::cout << std::endl;
        }

        std::system(("\"" + text + "\"" + (suppressOutput ?
#ifdef _WIN32
            " >nul 2>nul"
#else
            " >/dev/null 2>/dev/null"
#endif            
            : "")).c_str());

        std::cout << std::string(suppressOutput ? " " : "") + "[RUN FINISHED]" << std::endl;
    }

    Command operator+(Command a, std::string b)
    {
        return Command{ a.text == "" ? b : a.text + " " + b, a.path };
    }

    Command operator-(Command a, std::string b)
    {
        return Command{ a.text == "" ? b : a.text + b, a.path };
    }

    Command operator+(Command a, std::filesystem::path b)
    {
        return a + ("\"" + b.string() + "\"");
    }

    Command operator-(Command a, std::filesystem::path b)
    {
        return a - ("\"" + b.string() + "\"");
    }


    template<typename T>
    void ParallelForEach(std::vector<T> vec, std::function<void(T)> fn, bool runAsync)
    {
        if (runAsync)
        {
            std::vector<std::thread> threads;
            for (T& i : vec)
            {
                threads.push_back(std::thread(fn, i));
            }
            for (std::thread& t : threads)
            {
                t.join();
            }
        }
        else
        {
            for (T& i : vec)
            {
                fn(i);
            }
        }
    }


    CompileCommand::CompileCommand(std::filesystem::path path)
        : Command({
#if defined(NOBPP_COMPILER_COMMAND)
            NOBPP_COMPILER_COMMAND,
#elif defined(__nob_msvc__)
            "cl -c -EHsc",
#elif defined(__nob_gcc__)
            "g++ -c",  // todo: verify
#elif defined(__nob_clang__)
            "clang++ -c",  // todo: verify
#else
            ([]() {std::cout << "Error: CompileCommand used with unknown compiler. "
                               "Please define the NOBPP_COMPILER macro with the location of the compiler.\n";return "";})(),
#endif
            path })
    {

    }

    inline CompileCommand::CompileCommand(Command cmd)
        : Command(cmd)
    {
    }

    LinkCommand::LinkCommand(std::filesystem::path path)
        : Command({
#if defined(NOBPP_LINKER_COMMAND)
            NOBPP_LINKER_COMMAND,
#elif defined(__nob_msvc__)
            "cl -link",
#elif defined(__nob_gcc__)
            "g++",
#elif defined(__nob_clang__)
            "clang++",
#else
            ([]() {std::cout << "Error: CompileCommand used with unknown compiler. "
                               "Please define the NOBPP_COMPILER macro with the location of the compiler.\n";return "";})(),
#endif
            path })
    {

    }

    inline LinkCommand::LinkCommand(Command cmd)
        : Command(cmd)
    {
    }

    nob::LibraryCommand::LibraryCommand(std::filesystem::path path)
        : Command({
#if defined(NOBPP_LINKER_COMMAND)
            NOBPP_LINKER_COMMAND,
#elif defined(__nob_msvc__)
            "lib",
#elif defined(__nob_gcc__)
            "ar -rcs",
#elif defined(__nob_clang__)
            "ar -rcs",
#else
            ([]() {std::cout << "Error: LibraryCommand used with unknown compiler. "
                               "Please define the NOBPP_COMPILER macro with the location of the compiler.\n";return "";})(),
#endif
            path })
    {
    }

    nob::LibraryCommand::LibraryCommand(Command cmd)
        : Command(cmd)
    {
    }


// #define UNSUPPORTED() std::cout << "Error: This feature is not yet supported for the compiler you are using.\n"; return a


    CompileCommand operator+(CompileCommand a, SourceFile b)
    {
#if defined(__nob_msvc__)
        return a + b.path;
#elif defined(__nob_gcc__)
        return a + b.path;
#elif defined(__nob_clang__)
        return a + b.path;
#else
        return a + b.path;
#endif
    }

    CompileCommand operator+(CompileCommand a, ObjectFile b)
    {
#if defined(__nob_msvc__)
        return a + std::string("-Fo") - b.path;
#elif defined(__nob_gcc__)
        return a + std::string("-o") + b.path;
#elif defined(__nob_clang__)
        return a + std::string("-o") + b.path;
#else
        return a + std::string("-o") + b.path;
#endif
    }

    CompileCommand operator+(CompileCommand a, IncludeDirectory b)
    {
#if defined(__nob_msvc__)
        return a + std::string("-I") - b.path;
#elif defined(__nob_gcc__)
        return a + std::string("-I") - b.path;
#elif defined(__nob_clang__)
        return a + std::string("-I") - b.path;
#else
        return a + std::string("-I") - b.path;
#endif
    }

    CompileCommand operator+(CompileCommand a, CompilerFlag b)
    {
#if defined(NOBPP_CUSTOM_COMPILER_FLAG_HANDLER)
        return a + NOBPP_CUSTOM_COMPILER_FLAG_HANDLER(b);
#else
        switch (b)
        {
#if defined(__nob_msvc__)
        case CompilerFlag::OptimizeSpeed: return a + std::string("-O2"); break;
        case CompilerFlag::OptimizeSpace: return a + std::string("-O1"); break;
        case CompilerFlag::Debug: return a + std::string("-Zi"); break;
#elif defined(__nob_gcc__)
        case CompilerFlag::OptimizeSpeed: return a + std::string("-O2"); break;
        case CompilerFlag::OptimizeSpace: return a + std::string("-Os"); break;
        case CompilerFlag::Debug: return a + std::string("-g"); break;
#elif defined(__nob_clang__)
        case CompilerFlag::OptimizeSpeed: return a + std::string("-O2"); break;
        case CompilerFlag::OptimizeSpace: return a + std::string("-Os"); break;
        case CompilerFlag::Debug: return a + std::string("-g"); break;
#endif

#if defined(_WIN32)
        case CompilerFlag::PositionIndependentCode: return a; break;
#else
        case CompilerFlag::PositionIndependentCode: return a + CustomCompilerFlag{ "-fPIC" }; break;
#endif


        case CompilerFlag::KeepLinker:
        {
            size_t loc = a.text.find(" -c");
            if (loc == std::string::npos)
            {
                return a;
            }
            else
            {
                a.text = a.text.substr(0, loc) + a.text.substr(loc + 3);
                return a;
            }
        }
        default:
            std::cout << "CompilerFlag is not supported by your compiler.\n"; return a;
            break;
        }
#endif
    }

    CompileCommand operator+(CompileCommand a, CustomCompilerFlag b)
    {
        return (Command)a + b.flag;
    }

    CompileCommand operator+(CompileCommand a, AddLinkCommand b)
    {
#if defined(__nob_msvc__)
        return a + CompilerFlag::KeepLinker + b.lc.text.substr(b.lc.text.find("-link"));
#elif defined(__nob_gcc__)
        return a + CompilerFlag::KeepLinker + b.lc.text.substr(b.lc.text.find(" "));
#elif defined(__nob_clang__)
        return a + CompilerFlag::KeepLinker + b.lc.text.substr(b.lc.text.find(" "));
#else
        return a + CompilerFlag::KeepLinker + b.lc.text.substr(b.lc.text.find(" "));
#endif
    }

    LinkCommand operator+(LinkCommand a, ObjectFile b)
    {
#if defined(__nob_msvc__)
        size_t pos = a.text.find("-link");
        a.text = a.text.substr(0, pos) + "\"" + b.path.string() + "\" " + a.text.substr(pos);
        return a;
#elif defined(__nob_gcc__)
        return a + b.path;
#elif defined(__nob_clang__)
        return a + b.path;
#else
        return a + b.path;
#endif
    }

    LinkCommand operator+(LinkCommand a, StaticLibraryFile b)
    {
        if (b.path.extension().string() == "a" or b.path.extension().string() == "lib")
        {
#if defined(_WIN32)
            b.path.replace_extension("lib");
#else
            b.path.replace_extension("a");
#endif
        }

#if defined(__nob_msvc__)
        return a + std::string("-libpath:") - b.path.parent_path() + b.path;
#elif defined(__nob_gcc__)
        return a + std::string("-L") - b.path.parent_path() + b.path;
#elif defined(__nob_clang__)
        return a + std::string("-L") - b.path.parent_path() + b.path;
#else
        return a + std::string("-L") - b.path.parent_path() + b.path;
#endif
    }

    LinkCommand operator+(LinkCommand a, DynamicLibraryFile b)
    {
#if defined(_WIN32)
        b.path.replace_extension("dll");
#elif defined(__APPLE__)
        b.path.replace_extension("dylib");
#else
        b.path.replace_extension("so");
#endif
        return a + StaticLibraryFile{ b.path };
    }

    LinkCommand operator+(LinkCommand a, ExecutableFile b)
    {
#if defined(__nob_msvc__)
        return a + std::string("-out:") - b.path;
#elif defined(__nob_gcc__)
        return a + std::string("-o") + b.path;
#elif defined(__nob_clang__)
        return a + std::string("-o") + b.path;
#else
        return a + std::string("-o") + b.path;
#endif
    }

    LinkCommand operator+(LinkCommand a, LinkerFlag b)
    {
#if defined(NOBPP_CUSTOM_LINKER_FLAG_HANDLER)
        return a + NOBPP_CUSTOM_LINKER_FLAG_HANDLER(b);
#else
        switch (b)
        {
#if defined(__nob_msvc__)
        case LinkerFlag::OutputDynamicLibrary: return a + std::string("-dll"); break;
        case LinkerFlag::Debug: return a + std::string("-debug"); break;
#elif defined(__nob_gcc__)
        case LinkerFlag::OutputDynamicLibrary: return a + std::string("-shared"); break;
        case LinkerFlag::Debug: return a + std::string("-g"); break;
#elif defined(__nob_clang__)
        case LinkerFlag::OutputDynamicLibrary: return a + std::string("-shared"); break;
        case LinkerFlag::Debug: return a + std::string("-g"); break;
#endif
        default:
            std::cout << "LinkerFlag is not supported by your compiler.\n"; return a;
            break;
        }
#endif
    }

    LinkCommand operator+(LinkCommand a, CustomLinkerFlag b)
    {
        return (Command)a + b.flag;
    }


    LibraryCommand operator+(LibraryCommand a, ObjectFile b)
    {
        return a + b.path;
    }

    LibraryCommand operator+(LibraryCommand a, StaticLibraryFile b)
    {
#if defined(__nob_msvc__)
        return a + std::string("-out:") - b.path;
#else
        return a + b.path;
#endif
    }



    void CompileDirectory(std::filesystem::path src, std::filesystem::path obj, CompileCommand cmd, bool runAsync)
    {
        std::vector<std::filesystem::path> out;

        // get all of the cpp files
        std::copy_if(std::filesystem::recursive_directory_iterator(src), std::filesystem::recursive_directory_iterator{}, std::back_inserter(out),
            [](std::filesystem::path p)
            {
                return std::filesystem::is_regular_file(p) && p.string().substr(p.string().size() - 4) == ".cpp";
            }
        );

        // compile all of the cpp files
        nob::ParallelForEach(out, std::function([obj,cmd](std::filesystem::path p)
                {
                    (cmd + SourceFile{p} + ObjectFile{obj / p.filename().replace_extension(".obj") }).Run();
                }), runAsync
        );
    }

    void LinkDirectory(std::filesystem::path obj, std::filesystem::path exe, LinkCommand cmd)
    {
        for (std::filesystem::path p : std::filesystem::directory_iterator(obj))
        {
            cmd = cmd + ObjectFile{p};
        }
        cmd = cmd + ExecutableFile{exe};
        cmd.Run();
    }


    CompileCommand DefaultCompileCommand = {};
    LinkCommand DefaultLinkCommand = {};
    bool IsVerbose = false;

    void Init(int argc, char** argv, std::string srcName, bool askForFlags)
    {
        if (argc < 1)
        {
            std::cout << "Why no args?\n";
            return;
        }

        if (!(argc > 1 and std::string(argv[1]) == "-norebuild"))
        {
            std::filesystem::path binPath = { argv[0] };
            std::filesystem::path srcPath = binPath.parent_path() / srcName;

            if (!std::filesystem::exists(binPath) or !std::filesystem::exists(srcPath))
            {
                std::cout << "Source and/or binary not found.\n";
            }
            else
            {
                // delete <binary>.old
                std::filesystem::path oldBinPath = binPath.parent_path() / (binPath.filename().string() + ".old");
                if (std::filesystem::exists(oldBinPath))
                {
                    std::cout << "Deleting " << oldBinPath.filename() << "...\n";
                    std::filesystem::remove(oldBinPath);
                }

                // rebuild if source is younger than binary
                if (std::filesystem::last_write_time(binPath) < std::filesystem::last_write_time(srcPath))
                {
                    std::cout << "Source change detected, rebuilding...\n";
                    
                    // rename to <binary>.old
                    std::filesystem::rename(binPath, oldBinPath);

                    // compile and run the new binary           // todo: add vvv as a CompilerFlag
                    (CompileCommand() + SourceFile{ srcPath } + CustomCompilerFlag{ "-std:c++17" }
                    + ObjectFile{ std::filesystem::temp_directory_path() / srcPath.filename().replace_extension(".obj") } + AddLinkCommand{LinkCommand() + ExecutableFile{binPath}}).Run();

                    Command newBinCmd;
                    newBinCmd = newBinCmd + binPath + std::string("-norebuild");
                    for (int i = 1; i < argc; i++)
                    {
                        newBinCmd = newBinCmd + std::string(argv[i]);
                    }
                    newBinCmd.Run(false);
                    std::exit(0);
                }
            }
        }

        std::vector<std::string> args;
        if (askForFlags and (argc < 2 or (argc == 2 and std::string(argv[1]) == "-norebuild")))
        {
            std::cout << "Please enter arguments: ";
            std::string temp; std::getline(std::cin, temp);
            while (true)
            {
                size_t sp = temp.find(' ');
                args.push_back(temp.substr(0, sp));
                temp = temp.substr(sp);
                if (sp == std::string::npos)
                    break;
            }
        }
        else
        {
            for (int i = 1; i < argc; i++)
            {
                args.push_back(argv[i]);
            }
        }

        for (int i = 0; i < args.size(); i++)
        {
            if (args[i] == "-norebuild")
                continue;
            else if (args[i] == "-debug")
            {
                DefaultCompileCommand = DefaultCompileCommand + CompilerFlag::Debug;
                DefaultLinkCommand = DefaultLinkCommand + LinkerFlag::Debug;
            }
            else if (args[i] == "-verbose")
            {
                IsVerbose = true;
            }
            else
            {
                std::cout << "Argument not supported: " << args[i] << "\n";
            }
        }
    }
}

#endif
