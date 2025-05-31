/* NobPP Header-Only C++ Build System (https://github.com/arlotaylor/nobpp)
*
* This project is heavily inspired by Tsoding's nob.h. (see https://github.com/tsoding/nob.h)
* The goal of nobpp.hpp is to rewrite nob.h in C++ and specifcally for C++ applications. Much
* of the functionality of nob.h is implemented in std::filesystem, so this library implements
* only a subset of nob.h's functionality. For this reason, nobpp.hpp requires C++17.
* 
* nobpp.hpp also introduces an extra set of cross-platform functions to simplify things.
* 
* To use this library, simply create a build.cpp file in the same directory as this file with
* the following lines:
* 
*   #define NOBPP_IMPLEMENTATION
*   #include "nobpp.hpp"
* 
* If you wish to enable auto-rebuild and command line arguments, add the following line to the
* start of your main function:
* 
*   nob::Init(argv, argc, __FILE__);
* 
* This line will rebuild the binary if the source file has been edited since the last compile.
* It also initialises some default commands, if flags like -debug or -silent are supplied.
* 
* nobpp.hpp consists of two 'layers' of functionality. The first uses the struct nob::Command
* to execute commands. Arguments are passed to these commands with overloads of the + operator
* (or - if you do not wish for a space to be placed between two arguments). Currently strings
* and std::filesystem::path's can be used as arguments.
* 
* The second layer involves the nob::CompileCommand, nob::LinkCommand and nob::LibraryCommand
* structs and executes commands specific to a C++ compiler. There are a range of higher-level
* argument types that can be supplied to these commands, and when run, these will run commands
* specific to the compiler used (GCC, MSVC and Clang are supported).
* 
* To compile the build.cpp file, use the command below that corresponds to your compiler:
* 
* MSVC:  cl -EHsc build.cpp -std:c++17
* GCC:   g++ build.cpp -std=c++17
* Clang: clang++ build.cpp -std=c++17
* 
* Note: For the MSVC cl.exe to work you must run it via the Developer Command Prompt for VS.
* This also means that the resulting binary should be run via this command prompt.
*/


#ifndef NOBPP_HEADER
#define NOBPP_HEADER
// nobpp header

#include <filesystem>
#include <functional>
#include <bitset>

namespace nob
{
    struct Command
    {
        std::string text;
        std::filesystem::path path = std::filesystem::current_path();

        float latestInput = 1.f;
        float earliestOutput = FLT_MAX;

        int Run(bool suppressOutput = false, bool plainErrors = false);
        void UpdateInputTime(std::filesystem::path file, bool skipOnFail = false);
        void UpdateOutputTime(std::filesystem::path file, bool skipOnFail = false);
    };

    Command operator+(Command a, std::string b);
    Command operator-(Command a, std::string b);
    Command operator+(Command a, std::filesystem::path b);
    Command operator-(Command a, std::filesystem::path b);
    Command operator+(Command a, Command b);
    Command operator-(Command a, Command b);

    template<typename T> void ParallelForEach(std::vector<T> vec, std::function<void(T)> fn, bool runAsync = true);

    bool OpenFileDialog(std::filesystem::path& out, std::filesystem::path startingFolder = std::filesystem::current_path(), bool isFolder = false);

    std::string AddEscapes(std::string inp);
    std::string RemoveEscapes(std::string inp);

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
    struct MacroDefinition { std::string macro; std::string definition; };

    struct PrecompiledHeader;

    PrecompiledHeader CreatePrecompiledHeader(CompileCommand cmd, std::filesystem::path header, std::filesystem::path pch);
    PrecompiledHeader UsePrecompiledHeader(std::filesystem::path header, std::filesystem::path pch);

    struct PrecompiledHeader
    {
    private:
        friend PrecompiledHeader CreatePrecompiledHeader(CompileCommand cmd, std::filesystem::path header, std::filesystem::path pch);
        friend PrecompiledHeader UsePrecompiledHeader(std::filesystem::path header, std::filesystem::path pch);
        PrecompiledHeader(std::filesystem::path headerp, std::filesystem::path pchp);
    public:
        std::filesystem::path header;
        std::filesystem::path pch;
    };

    struct StaticLibraryFile { std::filesystem::path path; };  // is either .lib or .a
    struct DynamicLibraryFile { std::filesystem::path path; };  // is either .dll, .so or .dylib

    struct ExecutableFile { std::filesystem::path path; };
    struct AddLinkCommand { LinkCommand lc; };  // it is really annoying that this has to exist, but CompilerCommand + LinkCommand is ambiguous because LinkCommand casts to a path

    enum class CompilerFlag
    {
        OptimizeSpeed, OptimizeSpace,
        KeepLinker,
        Debug,
        PositionIndependentCode,
        CPPVersion14, CPPVersion17, CPPVersion20,
        NoObjectFile,
    };
    enum class LinkerFlag { OutputDynamicLibrary, Debug };
    struct CustomCompilerFlag { std::string flag; };
    struct CustomLinkerFlag { std::string flag; };

    CompileCommand operator+(CompileCommand a, SourceFile b);
    CompileCommand operator+(CompileCommand a, ObjectFile b);
    CompileCommand operator+(CompileCommand a, IncludeDirectory b);
    CompileCommand operator+(CompileCommand a, MacroDefinition b);
    CompileCommand operator+(CompileCommand a, PrecompiledHeader b);
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

    void CompileDirectory(std::filesystem::path src, std::filesystem::path obj, CompileCommand cmd = DefaultCompileCommand, bool runAsync = false);
    void LinkDirectory(std::filesystem::path obj, std::filesystem::path exe, LinkCommand cmd = DefaultLinkCommand);

    enum CLArgument
    {
        NoRebuild = 0,
        NoInitScript,
        Configure,
        Debug,
        Silent,
        Clean,
        Count,
    };

    extern std::bitset<CLArgument::Count> CLFlags;
    extern std::vector<std::string> OtherCLArguments;
    extern std::filesystem::path ThisExecutablePath;

    class Init
    {
    public:
        Init(int argc, char** argv, std::string srcName);
        ~Init();
    };

    enum LogType
    {
        None = -1,
        Info,
        Run,
        Error,
    };

    void Log(std::string s, LogType t = LogType::None);

    struct ConfigurationFile
    {
        std::filesystem::path file;
        std::string compilerName = "";

        std::string extraCompilerDefaults = "";
        std::string extraLinkerDefaults = "";
        enum class ConfigUIMode { Basic, Fancy } uiMode = ConfigurationFile::ConfigUIMode::Basic;
        enum class ConfigFileDialogMode { BasicCL, FancyCL, OSCall } fileDialogMode = ConfigurationFile::ConfigFileDialogMode::BasicCL;
        LogType minimumLogLevel = LogType::Info;
        bool IsSummaryMode = false;
        enum class ConfigRecompileMode { Always, Ask, Never } recompileMode = ConfigurationFile::ConfigRecompileMode::Always;
        std::string initScript = "";

        CompileCommand GetCommand(SourceFile sf, ExecutableFile ef);
        static ConfigurationFile GetDefaultConfig();
    };

    bool FindConfigFile(std::filesystem::path& fileOut);
    ConfigurationFile LoadConfigFile(std::filesystem::path file);
    void SaveConfigFile(std::filesystem::path file, ConfigurationFile config);

    int AskMultipleChoiceQuestion(std::string question, std::string info, std::vector<std::string> answers, int defaultVal);
    std::string AskShortAnswerQuestion(std::string question);

    ConfigurationFile GenerateConfigFile();
}

#endif


#define NOBPP_IMPLEMENTATION
#ifdef NOBPP_IMPLEMENTATION
// nobpp implementation

#include <thread>
#include <iostream>
#include <string>
#include <fstream>
#include <ciso646>
#include <cstdlib>

// ------------------------------ MACRO DEFINITIONS ----------------------------------
#if defined(__clang__)
  #define __nob_clang__
#elif defined(_MSC_VER)
  #define __nob_msvc__
#elif defined(__GNUC__)
  #define __nob_gcc__
#endif


#ifdef NOBPP_CONFIGURED

  #if !defined(NOBPP_COMPILER_NAME) \
   || !defined(NOBPP_EXTRA_DEFAULT_COMPILER_ARGS) \
   || !defined(NOBPP_EXTRA_DEFAULT_LINKER_ARGS) \
   || !defined(NOBPP_UI_MODE) \
   || !defined(NOBPP_FILE_DIALOG_MODE) \
   || !defined(NOBPP_MINIMUM_LOG_LEVEL) \
   || !defined(NOBPP_SUMMARY_MODE) \
   || !defined(NOBPP_RECOMPILE_MODE) \
   || !defined(NOBPP_INIT_SCRIPT)
    #error  // poorly defined configuration
  #else
    #if (NOBPP_UI_MODE != 0 && NOBPP_UI_MODE != 1) \  // basic, pretty
     || (NOBPP_MINIMUM_LOG_LEVEL != 0 && NOBPP_MINIMUM_LOG_LEVEL != 1 && NOBPP_MINIMUM_LOG_LEVEL != 2 && NOBPP_MINIMUM_LOG_LEVEL != -1) \  // trace, info, error, none
     || (NOBPP_SUMMARY_MODE != 0 && NOBPP_SUMMARY_MODE != 1) \  // no, yes
     || (NOBPP_RECOMPILE_MODE != 0 && NOBPP_RECOMPILE_MODE != 1 && NOBPP_RECOMPILE_MODE != 2) \  // auto, ask, never
     || (NOBPP_FILE_DIALOG_MODE != 0 && NOBPP_FILE_DIALOG_MODE != 1 && NOBPP_FILE_DIALOG_MODE != 2) \  // command line, pretty command line, os-specific
      #error  // invalid macros
    #endif

    #if NOBPP_FILE_DIALOG_MODE == 1 && NOBPP_UI_MODE != 1
      #error  // pretty file dialogs are not supported in basic mode
    #endif
  #endif
#else

  #if defined(__nob_msvc__)
    #define NOBPP_COMPILER_NAME "cl"
  #elif defined(__nob_clang__)
    #define NOBPP_COMPILER_NAME "clang++"
  #elif defined(__nob_gcc__)
    #define NOBPP_COMPILER_NAME "g++"
  #else
    #define NOBPP_COMPILER_NAME ""
  #endif

  #define NOBPP_EXTRA_DEFAULT_COMPILER_ARGS ""
  #define NOBPP_EXTRA_DEFAULT_LINKER_ARGS ""
  #define NOBPP_UI_MODE 0
  #define NOBPP_FILE_DIALOG_MODE 0
  #define NOBPP_MINIMUM_LOG_LEVEL 0
  #define NOBPP_SUMMARY_MODE 0
  #define NOBPP_RECOMPILE_MODE 1
  #define NOBPP_INIT_SCRIPT ""
#endif

// TODO: change this?
#if defined(_WIN32) && !defined(NOBPP_CUSTOM_LOG)
#define NOMINMAX
#include <Windows.h>  // for logging :(
#include <shobjidl.h>  // for file dialog :(
#endif

// --------------------------- NOBPP CORE ---------------------------
namespace nob
{
// --------------------------- BASIC COMMAND -----------------------------

    int Command::Run(bool suppressOutput, bool plainErrors)
    {
        if (!CLFlags[CLArgument::Clean] and earliestOutput != FLT_MAX and latestInput < earliestOutput)
        {
            Log("Command skipped.\n", LogType::Run);
            return 0;
        }

        if (CLFlags[CLArgument::Silent])
        {
            suppressOutput = true;
        }

        Log(text, LogType::Run);

        if (!suppressOutput)
        {
            Log("\n");
        }

        int ret = std::system(
            ("\"" + text + "\"" + (suppressOutput ?
        #ifdef _WIN32
            " >nul"
        #else
            " >/dev/null"
        #endif            
            : "") + (plainErrors ? std::string("") : (" 2>" + (std::filesystem::temp_directory_path() / "nob_error_log.txt").string()))).c_str()
        );

        if (!plainErrors)
        {
            {
                std::ifstream errorFile((std::filesystem::temp_directory_path() / "nob_error_log.txt"));
                std::string temp;
                while (std::getline(errorFile, temp))
                {
                    Log(temp + "\n", LogType::Error);
                }
            }
        }

        Log("Done\n", LogType::Run);

        return ret;
    }

    void Command::UpdateInputTime(std::filesystem::path file, bool skipOnFail)
    {
        if (std::filesystem::exists(file))
        {
            float modified = std::chrono::duration<float>(std::filesystem::last_write_time(file).time_since_epoch()).count();
            latestInput = std::max(latestInput, modified);
        }
        else if (!skipOnFail)
        {
            latestInput = FLT_MAX;
        }
    }

    void Command::UpdateOutputTime(std::filesystem::path file, bool skipOnFail)
    {
        if (std::filesystem::exists(file))
        {
            float modified = std::chrono::duration<float>(std::filesystem::last_write_time(file).time_since_epoch()).count();
            earliestOutput = std::min(earliestOutput, modified);
        }
        else if (!skipOnFail)
        {
            earliestOutput = 0.f;
        }
    }

    Command operator+(Command a, std::string b)
    {
        return Command{ a.text == "" ? b : a.text + " " + b, a.path, a.latestInput, a.earliestOutput };
    }

    Command operator-(Command a, std::string b)
    {
        return Command{ a.text == "" ? b : a.text + b, a.path, a.latestInput, a.earliestOutput };
    }

    Command operator+(Command a, std::filesystem::path b)
    {
        return a + ("\"" + b.string() + "\"");
    }

    Command operator-(Command a, std::filesystem::path b)
    {
        return a - ("\"" + b.string() + "\"");
    }

    Command operator+(Command a, Command b)
    {
        return a + std::string("&&") + b.text;
    }

    Command operator-(Command a, Command b)
    {
        return a + b.text;
    }


// ------------------------ CORE HELPER FUNCTIONS -------------------------

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

    std::string AddEscapes(std::string inp)
    {
        std::string ret = "";
        for (int i = 0; i < inp.size(); i++)
        {
            switch (inp[i])
            {
            case '\'': ret += "\\\'"; break;
            case '\"': ret += "\\\""; break;
            case '\?': ret += "\\\?"; break;
            case '\\': ret += "\\\\"; break;
            case '\a': ret += "\\a"; break;
            case '\b': ret += "\\b"; break;
            case '\f': ret += "\\f"; break;
            case '\n': ret += "\\n"; break;
            case '\r': ret += "\\r"; break;
            case '\t': ret += "\\t"; break;
            case '\v': ret += "\\v"; break;
            default: ret += inp[i]; break;
            }
        }
        return ret;
    }

    std::string RemoveEscapes(std::string inp)
    {
        std::string ret = "";
        for (int i = 0; i < inp.size(); i++)
        {
            if (inp[i] == '\\')
            {
                i++;
                switch (inp[i])
                {
                case '\'': ret += "\'"; break;
                case '\"': ret += "\""; break;
                case '\?': ret += "\?"; break;
                case '\\': ret += "\\"; break;
                case '\a': ret += "a"; break;
                case '\b': ret += "b"; break;
                case '\f': ret += "f"; break;
                case '\n': ret += "n"; break;
                case '\r': ret += "r"; break;
                case '\t': ret += "t"; break;
                case '\v': ret += "v"; break;
                default: Log("Error: unrecognized escape code cannot be removed."); break;
                }
            }
            else
            {
                ret += inp[i];
            }
        }
        return ret;
    }


// ------------------------ UI HELPER FUNCTIONS -------------------------

    bool OpenFileDialog(std::filesystem::path& out, std::filesystem::path startingFolder, bool isFolder)
    {
      #ifdef _WIN32
        #define BREAK_ON_FAIL(x) if (FAILED(x)) break

        bool ret = false;

        IFileOpenDialog *fileDialog = nullptr;
        do
        {
            BREAK_ON_FAIL(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)(&fileDialog)));

            if (isFolder)
            {
                FILEOPENDIALOGOPTIONS options = 0;
                BREAK_ON_FAIL(fileDialog->GetOptions(&options));
                options |= FOS_PICKFOLDERS;
                BREAK_ON_FAIL(fileDialog->SetOptions(options));
            }

            HRESULT hr = fileDialog->Show(NULL);
            if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED) || FAILED(hr))
                break;

            IShellItemArray *items;
            BREAK_ON_FAIL(fileDialog->GetResults(&items));
            DWORD itemCount = 0;
            BREAK_ON_FAIL(items->GetCount(&itemCount));

            if (itemCount != 1)
            {
                Log("Wrong number of items.", LogType::Error);
                break;
            }

            IShellItem *item;
            if (SUCCEEDED(items->GetItemAt(0, &item)))
            {
                PWSTR path;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                {
                    out = path;
                    ret = true;
                    CoTaskMemFree(path);
                }
                item->Release();
            }

            items->Release();

        } while (false);

        if (fileDialog)
            fileDialog->Release();
        
        return ret;

        #undef BREAK_ON_FAIL
      #else
        Log("not supported sorry\n");
        return {};
      #endif
    }

// ------------------------ COMPILE COMMANDS -------------------------

    CompileCommand::CompileCommand(std::filesystem::path path)
        : Command({
#if defined(NOBPP_COMPILER_COMMAND)
            NOBPP_COMPILER_COMMAND,
#elif defined(__nob_msvc__)
            "cl -c -EHsc -nologo",
#elif defined(__nob_gcc__)
            "g++ -c",
#elif defined(__nob_clang__)
            "clang++ -c",
#else
            ([]() {Log("Error: CompileCommand used with unknown compiler. Please define the NOBPP_COMPILER macro with the location of the compiler.\n", LogType::Info);return "";})(),
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
            "cl -nologo -link",
#elif defined(__nob_gcc__)
            "g++",
#elif defined(__nob_clang__)
            "clang++",
#else
            ([]() {Log("Error: LinkCommand used with unknown linker. Please define the NOBPP_LINKER macro with the location of the linker.\n", LogType::Info);return "";})(),
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
#if defined(NOBPP_LIBRARY_COMMAND)
            NOBPP_LIBRARY_COMMAND,
#elif defined(__nob_msvc__)
            "lib",
#elif defined(__nob_gcc__)
            "ar -rcs",
#elif defined(__nob_clang__)
            "ar -rcs",
#else
            ([]() {Log("Error: LibraryCommand used with unknown library archiver. Please define the NOBPP_LIBRARY macro with the location of the library archiver.\n", LogType::Info);return "";})(),
#endif
            path })
    {
    }

    nob::LibraryCommand::LibraryCommand(Command cmd)
        : Command(cmd)
    {
    }


    PrecompiledHeader CreatePrecompiledHeader(CompileCommand cmd, std::filesystem::path header, std::filesystem::path pch)
    {
        cmd.UpdateInputTime(header);
        cmd.UpdateOutputTime(pch);

        (cmd + IncludeDirectory{ header.parent_path() }
#if defined(__nob_msvc__)
            + std::string("-Yc") - header.filename() + std::string("-Fp") - pch
#elif defined(__nob_gcc__)

#elif defined(__nob_clang__)

#else

#endif
        ).Run();
        return PrecompiledHeader(header, pch);
    }

    PrecompiledHeader UsePrecompiledHeader(std::filesystem::path header, std::filesystem::path pch)
    {
        return PrecompiledHeader(header, pch);
    }


    PrecompiledHeader::PrecompiledHeader(std::filesystem::path headerp, std::filesystem::path pchp)
        : header(headerp), pch(pchp)
    {
    }



    CompileCommand operator+(CompileCommand a, SourceFile b)
    {
        a.UpdateInputTime(b.path);

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
        a.UpdateOutputTime(b.path);

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

    CompileCommand operator+(CompileCommand a, MacroDefinition b)
    {
#if defined(__nob_msvc__)
        return a + ("-D" + b.macro + "=\"" + AddEscapes("\"" + b.definition + "\"") + "\"");
#elif defined(__nob_gcc__)
        return a + ("-D" + b.macro + "=\"" + AddEscapes("\"" + b.definition + "\"") + "\"");
#elif defined(__nob_clang__)
        return a + ("-D" + b.macro + "=\"" + AddEscapes("\"" + b.definition + "\"") + "\"");
#else
        return a + ("-D" + b.macro + "=\"" + AddEscapes("\"" + b.definition + "\"") + "\"");
#endif
    }

    CompileCommand operator+(CompileCommand a, PrecompiledHeader b)
    {
        a.UpdateInputTime(b.header);
        a.UpdateInputTime(b.pch);

#if defined(__nob_msvc__)
        return a + IncludeDirectory{ b.header.parent_path() } + std::string("-Yu") - b.header.filename()  + std::string("-Fp") - b.pch;
#elif defined(__nob_gcc__)
        return a;
#elif defined(__nob_clang__)
        return a;
#else
        return a;
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
        case CompilerFlag::CPPVersion14: return a + std::string("-std:c++14"); break;
        case CompilerFlag::CPPVersion17: return a + std::string("-std:c++17"); break;
        case CompilerFlag::CPPVersion20: return a + std::string("-std:c++20"); break;
#elif defined(__nob_gcc__)
        case CompilerFlag::OptimizeSpeed: return a + std::string("-O2"); break;
        case CompilerFlag::OptimizeSpace: return a + std::string("-Os"); break;
        case CompilerFlag::Debug: return a + std::string("-g"); break;
        case CompilerFlag::CPPVersion14: return a + std::string("-std=c++14"); break;
        case CompilerFlag::CPPVersion17: return a + std::string("-std=c++17"); break;
        case CompilerFlag::CPPVersion20: return a + std::string("-std=c++20"); break;
#elif defined(__nob_clang__)
        case CompilerFlag::OptimizeSpeed: return a + std::string("-O2"); break;
        case CompilerFlag::OptimizeSpace: return a + std::string("-Os"); break;
        case CompilerFlag::Debug: return a + std::string("-g"); break;
        case CompilerFlag::CPPVersion14: return a + std::string("-std=c++14"); break;
        case CompilerFlag::CPPVersion17: return a + std::string("-std=c++17"); break;
        case CompilerFlag::CPPVersion20: return a + std::string("-std=c++20"); break;
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
        case CompilerFlag::NoObjectFile:
        {
            size_t loc = a.text.find(" ");

            // perform surgery so that this command is always overriden if + ObjectFile is used
            std::string tmp = a.text.substr(loc);
            a.text = a.text.substr(0, loc);
            a = a + ObjectFile{ std::filesystem::temp_directory_path() / "nobDeletedObj.o" };
            a.text += tmp;
            return a;
        }
        default:
            Log("CompilerFlag is not supported by your compiler.\n", LogType::Info); return a;
            break;
        }
#endif
    }

    CompileCommand operator+(CompileCommand a, CustomCompilerFlag b)
    {
        return (Command)a + b.flag;
    }

    LinkCommand AddDefaultOutputToLinker(CompileCommand a, LinkCommand b)
    {
        if (b.text.find(" -o") != std::string::npos)
        {
            return b;
        }
        else
        {
            for (int i = 0; i < a.text.size() - 1; i++)
            {
                if (a.text[i] == ' ' and a.text[i + 1] != ' ' and a.text[i + 1] != '-')
                {  // an argument with no - prefix is a source file
                    std::filesystem::path file;

                    if (a.text[i + 1] == '"')
                    {
                        file = a.text.substr(i + 2, a.text.substr(i + 2).find('"'));
                    }
                    else
                    {
                        file = a.text.substr(i + 1, a.text.substr(i + 1).find(' '));
                    }
#ifdef _WIN32
                    file = file.replace_extension("exe");
#else
                    file = file.replace_extension("");
#endif
                    return b + ExecutableFile{ file };
                }
            }

            return b;
        }
    }

    CompileCommand operator+(CompileCommand a, AddLinkCommand b)
    {
#if defined(__nob_msvc__)
        return a + CompilerFlag::KeepLinker + CompilerFlag::NoObjectFile + AddDefaultOutputToLinker(a, b.lc).text.substr(b.lc.text.find("-link"));
#elif defined(__nob_gcc__)
        return a + CompilerFlag::KeepLinker + CompilerFlag::NoObjectFile + AddDefaultOutputToLinker(a, b.lc).text.substr(b.lc.text.find(" "));
#elif defined(__nob_clang__)
        return a + CompilerFlag::KeepLinker + CompilerFlag::NoObjectFile + AddDefaultOutputToLinker(a, b.lc).text.substr(b.lc.text.find(" "));
#else
        return a + CompilerFlag::KeepLinker + CompilerFlag::NoObjectFile + AddDefaultOutputToLinker(a, b.lc).text.substr(b.lc.text.find(" "));
#endif
    }

    LinkCommand operator+(LinkCommand a, ObjectFile b)
    {
        a.UpdateInputTime(b.path);

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
        a.UpdateInputTime(b.path, true);

        if (b.path.extension().string() == "a" or b.path.extension().string() == "lib")
        {
#if defined(_WIN32)
            b.path.replace_extension("lib");
#else
            b.path.replace_extension("a");
#endif
        }

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
        a.UpdateOutputTime(b.path);

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
            Log("LinkerFlag is not supported by your compiler.\n", LogType::Info); return a;
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
        a.UpdateInputTime(b.path);

        return a + b.path;
    }

    LibraryCommand operator+(LibraryCommand a, StaticLibraryFile b)
    {
        a.UpdateOutputTime(b.path);

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
    std::bitset<CLArgument::Count> CLFlags;
    std::vector<std::string> OtherCLArguments;
    std::filesystem::path ThisExecutablePath;


    Command AddArgs(Command cmd, int argc, char** argv)
    {
        for (int i = 1; i < argc; i++)
        {
            if (std::string(argv[i]) == "-configure") continue;
            cmd = cmd + ("\"" + std::string(argv[i]) + "\"");
        }
        return cmd;
    }



    void ConsumeFlags(int argc, char** argv)
    {
        ThisExecutablePath = std::filesystem::path{ argv[0] };
        for (int i = 1; i < argc; i++)
        {
            if (std::string(argv[i]) == "-norebuild")
            {
                CLFlags.set(CLArgument::NoRebuild);
            }
            else if (std::string(argv[i]) == "-noinitscript")
            {
                CLFlags.set(CLArgument::NoInitScript);
            }
            else if (std::string(argv[i]) == "-configure")
            {
                CLFlags.set(CLArgument::Configure);
            }
            else if (std::string(argv[i]) == "-debug")
            {
                CLFlags.set(CLArgument::Debug);
            }
            else if (std::string(argv[i]) == "-silent")
            {
                CLFlags.set(CLArgument::Silent);
            }
            else if (std::string(argv[i]) == "-clean")
            {
                CLFlags.set(CLArgument::Clean);
            }
            else
            {
                OtherCLArguments.push_back(argv[i]);
            }
        }
    }

    ConfigurationFile ConfigurationFile::GetDefaultConfig()
    {
        return ConfigurationFile{
        #ifdef NOBPP_CONFIGURED
            { NOBPP_CONFIGURED },
        #else
            { "" },
        #endif
            NOBPP_COMPILER_NAME,
            NOBPP_EXTRA_DEFAULT_COMPILER_ARGS,
            NOBPP_EXTRA_DEFAULT_LINKER_ARGS,
            (ConfigurationFile::ConfigUIMode)NOBPP_UI_MODE,
            (ConfigurationFile::ConfigFileDialogMode)NOBPP_FILE_DIALOG_MODE,
            (LogType)NOBPP_MINIMUM_LOG_LEVEL,
            NOBPP_SUMMARY_MODE == 1,
            (ConfigurationFile::ConfigRecompileMode)NOBPP_RECOMPILE_MODE,
            NOBPP_INIT_SCRIPT
        };
    }

    bool FindConfigFile(std::filesystem::path& fileOut)
    {
        for (auto& i : std::filesystem::directory_iterator(ThisExecutablePath.parent_path()))
        {
            if (i.is_regular_file() && i.path().filename() == ".nobppconfig")
            {
                fileOut = i.path();
                return true;
            }
        }

      #ifdef _WIN32
        const char* appdata = std::getenv("LOCALAPPDATA");
        if (appdata == nullptr) return false;

        for (auto& i : std::filesystem::directory_iterator(std::filesystem::path{ appdata } / "nobpp"))
      #else
        const char* homepath = std::getenv("HOME");
        if (homepath == nullptr) return false;

        for (auto& i : std::filesystem::directory_iterator(std::filesystem::path{ homepath }))
      #endif
        {
            if (i.is_regular_file() && i.path().filename() == ".nobppconfig")
            {
                fileOut = i.path();
                return true;
            }
        }

        return false;
    }

    ConfigurationFile LoadConfigFile(std::filesystem::path file)
    {
        std::ifstream configFile(file);
        ConfigurationFile config;
        config.file = file;
        std::string temp;
        if (!std::getline(configFile, temp)) return config;
        config.compilerName = temp;
        if (!std::getline(configFile, temp)) return config;
        config.extraCompilerDefaults = temp;
        if (!std::getline(configFile, temp)) return config;
        config.extraLinkerDefaults = temp;
        if (!std::getline(configFile, temp)) return config;
        config.uiMode = (ConfigurationFile::ConfigUIMode)std::stoi(temp);
        if (!std::getline(configFile, temp)) return config;
        config.fileDialogMode = (ConfigurationFile::ConfigFileDialogMode)std::stoi(temp);
        if (!std::getline(configFile, temp)) return config;
        config.minimumLogLevel = (LogType)std::stoi(temp);
        if (!std::getline(configFile, temp)) return config;
        config.IsSummaryMode = temp == "1";
        if (!std::getline(configFile, temp)) return config;
        config.recompileMode = (ConfigurationFile::ConfigRecompileMode)std::stoi(temp);
        if (!std::getline(configFile, temp)) return config;
        config.initScript = temp;

        return config;
    }

    void SaveConfigFile(std::filesystem::path file, ConfigurationFile config)
    {
        std::ofstream fileOut(file);
        fileOut << config.compilerName << "\n" << config.extraCompilerDefaults << "\n" << config.extraLinkerDefaults << "\n"
        << (int)config.uiMode << "\n" << (int)config.fileDialogMode << "\n" << (int)config.minimumLogLevel << "\n"
        << (config.IsSummaryMode ? 1 : 0) << "\n" << (int)config.recompileMode << "\n" << config.initScript << "\n";
    }

    int AskMultipleChoiceQuestion(std::string question, std::string info, std::vector<std::string> answers, int defaultVal)
    {
        std::cout << question << "\n";
        for (int i = 0; i < answers.size(); i++)
        {
            if (i == defaultVal) std::cout << "(default) ";
            std::cout << i << ": " << answers[i] << "  ";
        }
        std::cout << "\n> ";

        std::string preout; std::getline(std::cin, preout);
        std::string out = "";
        for (char c : preout)
        {
            if (std::isalpha(c)) out += std::tolower(c);
            else out += c;
        }

        if (out == "") return defaultVal;
        else if (std::isdigit(out[0]))
        {
            int numOut = std::stoi(out);
            if (numOut >= 0 && numOut < answers.size()) return numOut;
        }
        else if (out == "i" || out == "info" || out == "?")
        {
            std::cout << "More Info: " << info << "\n";
            return AskMultipleChoiceQuestion(question, info, answers, defaultVal);
        }
        else if (out == "back" || out == "prev" || out == "previous")
        {
            return -1;
        }

        std::cout << "Unrecognized input. Try again.\n";
        return AskMultipleChoiceQuestion(question, info, answers, defaultVal);
    }

    std::string AskShortAnswerQuestion(std::string question)
    {
        std::cout << question << "\n> ";
        std::string ret; std::getline(std::cin, ret);
        return ret;
    }

    CompileCommand ConfigurationFile::GetCommand(SourceFile sf, ExecutableFile ef)
    {
        CompileCommand ret;
        ret.text = compilerName;
        ret = ret + std::string(extraCompilerDefaults);
        ret = ret + sf;
        ret = ret
        + MacroDefinition{ "NOBPP_COMPILER_NAME", compilerName }
        + MacroDefinition{ "NOBPP_EXTRA_DEFAULT_COMPILER_ARGS", extraCompilerDefaults }
        + MacroDefinition{ "NOBPP_EXTRA_DEFAULT_LINKER_ARGS", extraLinkerDefaults }
        + MacroDefinition{ "NOBPP_UI_MODE", std::to_string((int)uiMode) }
        + MacroDefinition{ "NOBPP_FILE_DIALOG_MODE", std::to_string((int)fileDialogMode) }
        + MacroDefinition{ "NOBPP_MINIMUM_LOG_LEVEL", std::to_string((int)minimumLogLevel) }
        + MacroDefinition{ "NOBPP_SUMMARY_MODE", (IsSummaryMode ? "1" : "0") }
        + MacroDefinition{ "NOBPP_RECOMPILE_MODE", std::to_string((int)recompileMode) }
        + MacroDefinition{ "NOBPP_INIT_SCRIPT", initScript };

        LinkCommand linkRet = LinkCommand{} + extraLinkerDefaults + ef;

        #ifdef _WIN32
        if (fileDialogMode == ConfigurationFile::ConfigFileDialogMode::OSCall)
        {
            linkRet = linkRet + StaticLibraryFile{ "ole32.lib" };
        }
        #endif  // TODO: implement linux and macos support here


        ret = ret + AddLinkCommand{ linkRet };
        return ret;
    }

    ConfigurationFile GenerateConfigFile()
    {
        ConfigurationFile ret;
        int out;
        for (int i = 0; i < 5; i++)
        {
            switch (i)
            {
            case 0:
                out = AskMultipleChoiceQuestion("Do you want to create a configuration, or load one?",
                    "The first time a build.cpp file is compiled, it is in an UNCONFIGURED state. When the resulting executable is run, it searches for a .nobppconfig. In this case, one was not found. This executable will then recompile itself, with macros based on the chosen configuration. If you already have a .nobppconfig file in another directory, you can enter its path here.", { "Create", "Load" }, 0);
                if (out == 1)
                {
                    std::filesystem::path path = AskShortAnswerQuestion("Enter .nobppconfig path.");
                    if (std::filesystem::is_regular_file(path))
                    {
                        return LoadConfigFile(path);
                    }
                    else if (std::filesystem::is_directory(path))
                    {
                        for (auto& p : std::filesystem::directory_iterator(path))
                        {
                            if (std::filesystem::is_regular_file(p.path()) && p.path().filename() == ".nobppconfig")
                            {
                                return LoadConfigFile(p.path());
                            }
                        }
                    }

                    std::cout << "Configuration file not found.\n";
                    i -= 1;
                }
                break;
            case 1:
                out = AskMultipleChoiceQuestion("What command invokes the compiler?", "This is the command used to invoke your compiler. The compiler used (clang, gcc and msvc are supported) is detected by the compiler used to compile this executable. If your installation has a custom alias, enter it here.", { "Detected (" + std::string(NOBPP_COMPILER_NAME) + ")", "Other" }, 0);
                if (out == 0)
                {
                    ret.compilerName = NOBPP_COMPILER_NAME;
                }
                else if (out == 1)
                {
                    ret.compilerName = AskShortAnswerQuestion("Enter compiler command name.");
                }
                break;
            case 2:
                out = AskMultipleChoiceQuestion("Do you want to change the default compiler arguments?", "This is pretty self-explanatory.", { "Yes", "No" }, 1);
                if (out == 0)
                {
                    ret.extraCompilerDefaults = AskShortAnswerQuestion("Enter the compiler arguments.");
                }
                else if (out == 1)
                {
                    ret.extraCompilerDefaults = NOBPP_EXTRA_DEFAULT_COMPILER_ARGS;
                }
                break;
            case 3:
                out = AskMultipleChoiceQuestion("Do you want to change the default linker arguments?", "This is pretty self-explanatory.", { "Yes", "No" }, 1);
                if (out == 0)
                {
                    ret.extraCompilerDefaults = AskShortAnswerQuestion("Enter the linker arguments.");
                }
                else if (out == 1)
                {
                    ret.extraLinkerDefaults = NOBPP_EXTRA_DEFAULT_LINKER_ARGS;
                }
                break;
            case 4:
                out = AskMultipleChoiceQuestion("Select the UI mode.", "The basic UI mode is the one you see now, no colour or fancy terminal stuff. It works the same on all platforms. Fancy mode offers a more fancy experience, but requires an OS-specific implementation, so it is not guaranteed to work on every platform.", { "Basic", "Fancy" }, 0);
                if (out == 0)
                {
                    ret.uiMode = ConfigurationFile::ConfigUIMode::Basic;
                }
                else if (out == 1)
                {
                    ret.uiMode = ConfigurationFile::ConfigUIMode::Fancy;
                }
                break;
            case 5:
                if (ret.uiMode == ConfigurationFile::ConfigUIMode::Fancy)
                {
                    out = AskMultipleChoiceQuestion("Select the file dialog mode.", "When (if ever) nobpp needs you to provide a file path, this setting dictates the way that you input it. Basic simply prompts you to type the path, while Fancy enables an in-terminal file picker dialog. OS-Specific calls your operating system's file picker, but is not guaranteed to work on every platform.", { "Basic", "Fancy", "OS-Specific" }, 0);
                    if (out >= 0) ret.fileDialogMode = (ConfigurationFile::ConfigFileDialogMode)out;
                }
                else
                {
                    out = AskMultipleChoiceQuestion("Select the file dialog mode.", "When (if ever) nobpp needs you to provide a file path, this setting dictates the way that you input it. Basic simply prompts you to type the path, while Fancy enables an in-terminal file picker dialog. OS-Specific calls your operating system's file picker, but is not guaranteed to work on every platform.", { "Basic", "OS-Specific" }, 0);
                    if (out >= 0) ret.fileDialogMode = (ConfigurationFile::ConfigFileDialogMode)(2*out);
                }
                break;
            case 6:
                out = AskMultipleChoiceQuestion("Select the minimum log level.", "This option silences all logs below the selected level.", { "Trace", "Info", "Error", "None" }, 0);
                if (out >= 0) ret.minimumLogLevel = (LogType)(out == 3 ? -1 : out);
                break;
            case 7:
                out = AskMultipleChoiceQuestion("Use summary mode?", "Summary mode replaces all command logs with concise descriptions." + std::string(ret.uiMode == ConfigurationFile::ConfigUIMode::Fancy ? " Since you are in Fancy mode, you can still see the full commands by selecting them." : ""), { "Yes", "No" }, 1);
                if (out == 0)
                {
                    ret.IsSummaryMode = true;
                }
                else if (out == 1)
                {
                    ret.IsSummaryMode = false;
                }
                break;
            case 8:
                out = AskMultipleChoiceQuestion("When this build script is updated, should it be recompiled?", "When the build executable runs, it compares itself to its source file. If the source file has been modified more recently, it can recompile itself. If Always is selected, this is done automatically. If Ask is selected, the program will ask before it rebuilds. If Never is selected, it will not rebuild. Note that this functionality is only available when the source file is in the same directory as this executable.", { "Always", "Ask", "Never" }, 2);
                if (out >= 0) ret.recompileMode = (ConfigurationFile::ConfigRecompileMode)out;
                break;
            case 9:
                out = AskMultipleChoiceQuestion("Do you want to add an init-script?", "You might want to run an additional program whenever this build is invoked. For example, if you are using MSVC, you will need to run vcvars64.bat if you want to compile outside of the Developer Powershell. You can add the path to that here.", { "Yes", "No" }, 1);
                if (out == 0)
                {
                    ret.initScript = AskShortAnswerQuestion("Enter the script name.");
                }
                if (out == 1)
                {
                    ret.initScript = "";
                }
                break;
            case 10:
                out = AskMultipleChoiceQuestion("Configuration complete! What do you want to do with it?", "You can still edit it by typing 'back'.", { "Save and Run", "Just Run", "Just Save" }, 0);
                if (out == 0 || out == 2)
                {
            #ifdef _WIN32
                    std::string homeName = "%LOCALAPPDATA%";
                    const char* appdata = std::getenv("LOCALAPPDATA");
                    std::filesystem::path homePath = appdata == nullptr ? std::filesystem::path{} : std::filesystem::path{ appdata };
            #else
                    std::string homeName = "$HOME";
                    const char* appdata = std::getenv("HOME");
                    std::filesystem::path homePath = appdata == nullptr ? std::filesystem::path{} : std::filesystem::path{ appdata };
            #endif
                    int loc;
                    if (appdata == nullptr)
                    {
                        int loc = AskMultipleChoiceQuestion("Where should this configuration be saved?", "If you do not save this configuration to this directory, it will not be automatically accessible if you (for some reason) need to manually rebuild this executable.", { "This Directory", "Custom" }, 0);
                    }
                    else
                    {
                        int loc = AskMultipleChoiceQuestion("Where should this configuration be saved?", "If you do not save this configuration to " + homeName + ", it will not be accessible by every nobpp build script on your system. Each build script can also access a configuration placed in its directory.", { "This Directory", homeName, "Custom" }, 0);
                    }
                    if (loc == 0)
                    {
                        SaveConfigFile(ThisExecutablePath.parent_path() / ".nobppconfig", ret);
                    }
                    else if (appdata != nullptr && loc == 1)
                    {
                        SaveConfigFile(homePath / ".nobppconfig", ret);
                    }
                    else if (loc > 0)
                    {
                        std::filesystem::path customDir = { AskShortAnswerQuestion("Enter the path of the directory where this configuration should be saved.") };
                        if (std::filesystem::is_directory(customDir))
                        {
                            SaveConfigFile(customDir / ".nobppconfig", ret);
                        }
                        else
                        {
                            out = -1;
                            i += 1;
                        }
                    }
                    else
                    {
                        out = -1;
                        i += 1;
                    }
                }

                if (out == 0 || out == 2) std::cout << "Configuration saved.\n";

                if (out == 2)
                {
                    std::exit(0);
                }
            default: break;
            }
            if (out == -1) i -= 2;
        }
        return ret;
    }

    void RenameCarefully(std::filesystem::path file, std::filesystem::path name)
    {
        if (std::filesystem::is_regular_file(file.parent_path() / name))
        {
            RenameCarefully(file.parent_path() / name, { name.stem().string() + ".old" + name.extension().string() });
        }
        std::filesystem::rename(file, file.replace_filename(name));
    }

    Init::Init(int argc, char** argv, std::string srcName)
    {
        if (argc < 1)
        {
            Log("Why no args?\n", LogType::Info);
            return;
        }

        // load the flags and arguments
        CLFlags.reset();
        OtherCLArguments.clear();
        ConsumeFlags(argc, argv);

        // get relevant files
        std::filesystem::path srcPath = ThisExecutablePath.parent_path() / srcName;
        if (!std::filesystem::is_regular_file(srcPath))
        {
            std::cout << "Source file not found. If you want " << srcName << " to be automatically recompiled when changed, it should be placed in the same folder as this executable.\n";
        }

        if (!std::filesystem::is_regular_file(ThisExecutablePath))
        {
            std::cout << "Executable file not found (for some reason). This may cause issues.\n";
        }
        else if (ThisExecutablePath.filename().string() != (srcPath.stem().string()  // if new, rename
      #ifdef _WIN32
            + ".exe"
      #endif
        ))
        {
            RenameCarefully(ThisExecutablePath, { srcPath.stem().string()
      #ifdef _WIN32
            + ".exe"
      #endif
            });
        }
        else  // otherwise delete old files
        {
            std::string stem = ThisExecutablePath.stem().string() + ".old";
            while (std::filesystem::is_regular_file(ThisExecutablePath.parent_path() / (stem + ThisExecutablePath.extension().string())))
            {
                std::filesystem::remove(ThisExecutablePath.parent_path() / (stem + ThisExecutablePath.extension().string()));
                stem += ".old";
            }
        }


      #define NOBPP_CONFIGURED
      #ifdef NOBPP_CONFIGURED
        bool shouldInitScript = false;
        bool shouldRebuild = false;
        if (NOBPP_INIT_SCRIPT != std::string("") && !CLFlags[CLArgument::NoInitScript]) shouldInitScript = true;
        if (NOBPP_RECOMPILE_MODE != 2 && !CLFlags[CLArgument::NoRebuild])
        {
            if (std::filesystem::last_write_time(ThisExecutablePath) < std::filesystem::last_write_time(srcPath))
            {
                int out = AskMultipleChoiceQuestion("Newer source file detected. Do you want to rebuild?", "This executable is configured to ask each time the program wants to rebuild.
                if (out == 0)
                {
                     // TODO: keep going
                }
            }
        }

      #else
        ConfigurationFile config;
        std::filesystem::path configPath;
        if (!CLFlags[CLArgument::Configure] && FindConfigFile(configPath))
        {
            std::cout << "Configuration found. Rerun with -configure to create a new configuration.\n";
            config = LoadConfigFile(configPath);
        }
        else
        {
            if (!CLFlags[CLArgument::Configure]) std::cout << "No configuration found.\n";
            config = GenerateConfigFile();
        }
        std::cout << "Rebuilding with configuration.\n";
        std::filesystem::path newExec = ThisExecutablePath.parent_path() / (ThisExecutablePath.stem().string() + ".new" + ThisExecutablePath.extension().string());
        Command newBinCmd = (config.GetCommand(SourceFile{ srcPath }, ExecutableFile{ newExec })) + (AddArgs(Command() + newExec, argc, argv) + std::string("-noinitscript") + std::string("-norebuild"));
        if (config.initScript != "")
        {
            newBinCmd = (Command() + std::filesystem::path{ config.initScript }) + newBinCmd;
        }
        newBinCmd.Run();
        std::exit(0);
      #endif

#ifdef NOBPP_INIT_SCRIPT
        if (!CLFlags[CLArgument::NoInitScript])  // init script can be run
        {
            Command newBinCmd;
            newBinCmd = newBinCmd + binPath + std::string("-noinitscript");
            newBinCmd = AddArgs(newBinCmd, argc, argv);
            ((Command() + std::filesystem::path{ NOBPP_INIT_SCRIPT }) + newBinCmd).Run(false, true);
            std::exit(0);
        }
#endif

        if (!CLFlags[CLArgument::NoRebuild] and std::filesystem::exists(srcPath))  // rebuild can be checked
        {
            // delete <binary>.old
            std::filesystem::path oldBinPath = binPath.parent_path() / (binPath.filename().string() + ".old");
            if (std::filesystem::exists(oldBinPath))
            {
                Log("Deleting " + oldBinPath.filename().string() + "...\n", LogType::Info);
                std::filesystem::remove(oldBinPath);
            }

            // rebuild if source is younger than binary
            if (std::filesystem::last_write_time(binPath) < std::filesystem::last_write_time(srcPath))
            {
                Log("Source change detected, rebuilding...\n", LogType::Info);

                // rename to <binary>.old
                std::filesystem::rename(binPath, oldBinPath);

                // compile and run the new binary
                (CompileCommand() + SourceFile{ srcPath } + CompilerFlag::CPPVersion17
#ifdef NOBPP_INIT_SCRIPT
                + MacroDefinition{ "NOBPP_INIT_SCRIPT", NOBPP_INIT_SCRIPT }
#endif
                + AddLinkCommand{LinkCommand()
#ifdef _WIN32
                    + StaticLibraryFile{ "ole32.lib" }
#endif
                    
                    + ExecutableFile{binPath}}).Run();

                Command newBinCmd;
                newBinCmd = newBinCmd + binPath + std::string("-norebuild");
                newBinCmd = AddArgs(newBinCmd, argc, argv);
                newBinCmd.Run(false, true);
                std::exit(0);
            }
        }


        if (CLFlags[CLArgument::Debug])
        {
            DefaultCompileCommand = DefaultCompileCommand + CompilerFlag::Debug;
            DefaultLinkCommand = DefaultLinkCommand + LinkerFlag::Debug;
        }
    }

    Init::~Init()
    {
        // std::filesystem::remove(std::filesystem::temp_directory_path() / "nob_error_log.txt");
    }

    void Log(std::string s, LogType t)
    {
        std::string prefix = t == LogType::Info ? "[INFO] " : (t == LogType::Run ? "[RUN]  " : "");

#ifdef NOBPP_CUSTOM_LOG
        NOBPP_CUSTOM_LOG((prefix + s));
#else

#ifdef _WIN32
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

        int consoleCode = 15;
        switch (t)
        {
        case LogType::Info: consoleCode = 3; break;
        case LogType::Run: consoleCode = 8; break;
        case LogType::Error: consoleCode = 4; break;
        default:
            break;
        }

        SetConsoleTextAttribute(console, consoleCode);
        std::cout << prefix + s;
        SetConsoleTextAttribute(console, 15);
#else
        std::string consoleCode = "0";
        switch (t)
        {
        case LogType::Info: consoleCode = "0;36"; break;
        case LogType::Run: consoleCode = "1;30"; break;
        case LogType::Error: consoleCode = "0;31"; break;
        default:
            break;
        }

        std::cout << "\033[" + consoleCode + "m" << prefix + s << "\033[0m";
#endif

#endif
    }
}

#ifdef NOBPP_WINDOWS_WAS_STRIPPED
#define _WIN32
#undef NOBPP_WINDOWS_WAS_STRIPPED
#endif

#endif
