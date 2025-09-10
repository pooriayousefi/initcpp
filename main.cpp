#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <map>
#include <cctype>

namespace fs = std::filesystem;

class CppProjectInitializer
{
private:
    std::string project_path_;
    std::string project_name_;
    
    // Helper method to execute system commands
    int execute_command(const std::string& command) const
    {
        std::cout << "Executing: " << command << std::endl;
        return std::system(command.c_str());
    }
    
    // Helper method to replace placeholders in templates
    std::string replace_placeholders(std::string content, const std::map<std::string, std::string>& replacements) const
    {
        for (const auto& [placeholder, replacement] : replacements)
        {
            size_t pos = 0;
            while ((pos = content.find(placeholder, pos)) != std::string::npos)
            {
                content.replace(pos, placeholder.length(), replacement);
                pos += replacement.length();
            }
        }
        return content;
    }
    
    // Write content to file
    void write_file(const fs::path& file_path, const std::string& content) const
    {
        std::ofstream file(file_path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not create file: " + file_path.string());
        }
        file << content;
        file.close();
        std::cout << "Created: " << file_path << std::endl;
    }
    
    // Helper method to sanitize project name for C++ identifiers
    std::string sanitize_cpp_name(const std::string& name) const
    {
        std::string sanitized = name;
        // Replace hyphens and other invalid characters with underscores
        for (char& c : sanitized)
        {
            if (!std::isalnum(c) && c != '_')
            {
                c = '_';
            }
        }
        // Ensure it starts with a letter or underscore
        if (!sanitized.empty() && std::isdigit(sanitized[0]))
        {
            sanitized = "_" + sanitized;
        }
        return sanitized;
    }

public:
    CppProjectInitializer(const std::string& project_path) 
        : project_path_(project_path)
    {
        fs::path path(project_path_);
        project_name_ = sanitize_cpp_name(path.stem().string());
    }
    
    void create_directory_structure()
    {
        std::cout << "Creating directory structure..." << std::endl;
        
        std::vector<std::string> directories = {
            project_path_,
            project_path_ + "/include",
            project_path_ + "/src", 
            project_path_ + "/build",
            project_path_ + "/build/debug",
            project_path_ + "/build/release",
            project_path_ + "/tests",
            project_path_ + "/.vscode"
        };
        
        for (const auto& dir : directories)
        {
            fs::create_directories(dir);
            std::cout << "Created directory: " << dir << std::endl;
        }
    }
    
    void setup_vcpkg()
    {
        std::cout << "Setting up vcpkg from template..." << std::endl;
        
        const std::string vcpkg_path = project_path_ + "/.vcpkg";
        const std::string template_vcpkg_path = fs::current_path().string() + "/templates/.vcpkg";
        
        // Check if template vcpkg exists
        if (!fs::exists(template_vcpkg_path))
        {
            std::cout << "Warning: Template vcpkg not found at " << template_vcpkg_path << std::endl;
            std::cout << "Skipping vcpkg setup. You can manually set up vcpkg if needed." << std::endl;
            return;
        }
        
        // Copy template vcpkg to project directory
        std::cout << "Copying vcpkg from template directory..." << std::endl;
        std::string copy_cmd = "cp -r " + template_vcpkg_path + " " + vcpkg_path;
        if (execute_command(copy_cmd) != 0)
        {
            std::cout << "Warning: Failed to copy vcpkg from template. You can manually set up vcpkg if needed." << std::endl;
            return;
        }
        
        // Create vcpkg.json manifest
        std::string vcpkg_manifest = "{\n";
        vcpkg_manifest += "  \"name\": \"" + project_name_ + "\",\n";
        vcpkg_manifest += "  \"version\": \"1.0.0\",\n";
        vcpkg_manifest += "  \"description\": \"" + project_name_ + " C++ project\",\n";
        vcpkg_manifest += "  \"homepage\": \"\",\n";
        vcpkg_manifest += "  \"documentation\": \"\",\n";
        vcpkg_manifest += "  \"supports\": \"linux\",\n";
        vcpkg_manifest += "  \"dependencies\": [\n";
        vcpkg_manifest += "    \n";
        vcpkg_manifest += "  ],\n";
        vcpkg_manifest += "  \"builtin-baseline\": \"2024.09.30\"\n";
        vcpkg_manifest += "}";
        
        write_file(project_path_ + "/vcpkg.json", vcpkg_manifest);
        
        std::cout << "vcpkg setup complete from template!" << std::endl;
    }
    
    void copy_template_headers()
    {
        std::cout << "Copying template header files..." << std::endl;
        
        const fs::path template_dir = fs::current_path() / "templates";
        const fs::path include_dir = project_path_ + "/include";
        
        // List of template header files to copy
        std::vector<std::string> template_headers = {
            "asyncops.hpp",
            "raiiiofsw.hpp", 
            "stringformers.hpp",
            "utilities.hpp"
        };
        
        for (const auto& header : template_headers)
        {
            fs::path source_file = template_dir / header;
            fs::path dest_file = include_dir / header;
            
            if (fs::exists(source_file))
            {
                try
                {
                    fs::copy_file(source_file, dest_file);
                    std::cout << "Copied: " << header << std::endl;
                }
                catch (const std::exception& e)
                {
                    std::cout << "Warning: Failed to copy " << header << ": " << e.what() << std::endl;
                }
            }
            else
            {
                std::cout << "Warning: Template header " << header << " not found." << std::endl;
            }
        }
    }

    void create_source_files()
    {
        std::cout << "Creating source files..." << std::endl;
        
        // Main source file template
        std::string main_cpp_template = "#include <iostream>\n";
        main_cpp_template += "#include <exception>\n\n";
        main_cpp_template += "// entry-point\n";
        main_cpp_template += "int main()\n";
        main_cpp_template += "{\n";
        main_cpp_template += "    try\n";
        main_cpp_template += "    {\n";
        main_cpp_template += "        std::cout << \"Hello from " + project_name_ + "!\" << std::endl;\n";
        main_cpp_template += "        \n";
        main_cpp_template += "        // start here ...\n";
        main_cpp_template += "        \n";
        main_cpp_template += "        return 0;\n";
        main_cpp_template += "    }\n";
        main_cpp_template += "    catch (const std::exception& e)\n";
        main_cpp_template += "    {\n";
        main_cpp_template += "        std::cerr << \"Error: \" << e.what() << std::endl;\n";
        main_cpp_template += "        return 1;\n";
        main_cpp_template += "    }\n";
        main_cpp_template += "}\n";
        
        write_file(project_path_ + "/src/main.cpp", main_cpp_template);
        
        // Example header file
        std::string example_header = "#pragma once\n\n";
        example_header += "#include <string>\n\n";
        example_header += "namespace " + project_name_ + "\n";
        example_header += "{\n";
        example_header += "    class ExampleClass\n";
        example_header += "    {\n";
        example_header += "    private:\n";
        example_header += "        std::string name_;\n";
        example_header += "        \n";
        example_header += "    public:\n";
        example_header += "        explicit ExampleClass(const std::string& name);\n";
        example_header += "        ~ExampleClass() = default;\n";
        example_header += "        \n";
        example_header += "        // Copy constructor and assignment\n";
        example_header += "        ExampleClass(const ExampleClass& other) = default;\n";
        example_header += "        ExampleClass& operator=(const ExampleClass& other) = default;\n";
        example_header += "        \n";
        example_header += "        // Move constructor and assignment\n";
        example_header += "        ExampleClass(ExampleClass&& other) noexcept = default;\n";
        example_header += "        ExampleClass& operator=(ExampleClass&& other) noexcept = default;\n";
        example_header += "        \n";
        example_header += "        std::string get_name() const;\n";
        example_header += "        void set_name(const std::string& name);\n";
        example_header += "    };\n";
        example_header += "}\n";
        
        write_file(project_path_ + "/include/" + project_name_ + ".hpp", example_header);
        
        // Example implementation file
        std::string example_impl = "#include \"" + project_name_ + ".hpp\"\n\n";
        example_impl += "namespace " + project_name_ + "\n";
        example_impl += "{\n";
        example_impl += "    ExampleClass::ExampleClass(const std::string& name) : name_(name)\n";
        example_impl += "    {\n";
        example_impl += "    }\n";
        example_impl += "    \n";
        example_impl += "    std::string ExampleClass::get_name() const\n";
        example_impl += "    {\n";
        example_impl += "        return name_;\n";
        example_impl += "    }\n";
        example_impl += "    \n";
        example_impl += "    void ExampleClass::set_name(const std::string& name)\n";
        example_impl += "    {\n";
        example_impl += "        name_ = name;\n";
        example_impl += "    }\n";
        example_impl += "}\n";
        
        write_file(project_path_ + "/src/" + project_name_ + ".cpp", example_impl);
        
        // Test file
        std::string test_template = "#include <iostream>\n";
        test_template += "#include <cassert>\n";
        test_template += "#include \"" + project_name_ + ".hpp\"\n\n";
        test_template += "int main()\n";
        test_template += "{\n";
        test_template += "    try\n";
        test_template += "    {\n";
        test_template += "        " + project_name_ + "::ExampleClass example(\"test\");\n";
        test_template += "        assert(example.get_name() == \"test\");\n";
        test_template += "        \n";
        test_template += "        example.set_name(\"updated\");\n";
        test_template += "        assert(example.get_name() == \"updated\");\n";
        test_template += "        \n";
        test_template += "        std::cout << \"All tests passed!\" << std::endl;\n";
        test_template += "        return 0;\n";
        test_template += "    }\n";
        test_template += "    catch (const std::exception& e)\n";
        test_template += "    {\n";
        test_template += "        std::cerr << \"Test failed: \" << e.what() << std::endl;\n";
        test_template += "        return 1;\n";
        test_template += "    }\n";
        test_template += "}\n";
        
        write_file(project_path_ + "/tests/test_main.cpp", test_template);
    }
    
    void create_build_system()
    {
        std::cout << "Creating build system..." << std::endl;
        
        // Create C++ build system executable
        std::string build_cpp = "#include <iostream>\n";
        build_cpp += "#include <string>\n";
        build_cpp += "#include <vector>\n";
        build_cpp += "#include <cstdlib>\n";
        build_cpp += "#include <filesystem>\n\n";
        build_cpp += "namespace fs = std::filesystem;\n\n";
        build_cpp += "class BuildSystem\n";
        build_cpp += "{\n";
        build_cpp += "private:\n";
        build_cpp += "    std::string build_type_;\n";
        build_cpp += "    std::string output_type_;\n";
        build_cpp += "    \n";
        build_cpp += "    int execute_command(const std::string& command) const\n";
        build_cpp += "    {\n";
        build_cpp += "        std::cout << \"Executing: \" << command << std::endl;\n";
        build_cpp += "        return std::system(command.c_str());\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "public:\n";
        build_cpp += "    BuildSystem() : build_type_(\"debug\"), output_type_(\"executable\")\n";
        build_cpp += "    {\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "    void set_build_type(const std::string& type)\n";
        build_cpp += "    {\n";
        build_cpp += "        build_type_ = type;\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "    void set_output_type(const std::string& type)\n";
        build_cpp += "    {\n";
        build_cpp += "        output_type_ = type;\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "    int build()\n";
        build_cpp += "    {\n";
        build_cpp += "        std::string build_dir = \"build/\" + build_type_;\n";
        build_cpp += "        fs::create_directories(build_dir);\n";
        build_cpp += "        \n";
        build_cpp += "        std::vector<std::string> source_files;\n";
        build_cpp += "        \n";
        build_cpp += "        // Collect all source files\n";
        build_cpp += "        for (const auto& entry : fs::recursive_directory_iterator(\"src\"))\n";
        build_cpp += "        {\n";
        build_cpp += "            if (entry.is_regular_file() && entry.path().extension() == \".cpp\")\n";
        build_cpp += "            {\n";
        build_cpp += "                source_files.push_back(entry.path().string());\n";
        build_cpp += "            }\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        std::string compile_flags;\n";
        build_cpp += "        std::string link_flags;\n";
        build_cpp += "        std::string output_name;\n";
        build_cpp += "        \n";
        build_cpp += "        if (build_type_ == \"debug\")\n";
        build_cpp += "        {\n";
        build_cpp += "            compile_flags = \"-g -O0 -DDEBUG\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (build_type_ == \"release\")\n";
        build_cpp += "        {\n";
        build_cpp += "            compile_flags = \"-O3 -DNDEBUG\";\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        // Common flags\n";
        build_cpp += "        compile_flags += \" -std=c++23 -Wall -Wextra -Wpedantic -Iinclude\";\n";
        build_cpp += "        \n";
        build_cpp += "        // Add vcpkg paths if available\n";
        build_cpp += "        if (fs::exists(\".vcpkg/installed/x64-linux/include\"))\n";
        build_cpp += "        {\n";
        build_cpp += "            compile_flags += \" -I.vcpkg/installed/x64-linux/include\";\n";
        build_cpp += "            link_flags = \"-L.vcpkg/installed/x64-linux/lib\";\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        if (output_type_ == \"executable\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/\" + \"" + project_name_ + "\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (output_type_ == \"static\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/lib\" + \"" + project_name_ + "\" + \".a\";\n";
        build_cpp += "            compile_flags += \" -c\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (output_type_ == \"dynamic\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/lib\" + \"" + project_name_ + "\" + \".so\";\n";
        build_cpp += "            compile_flags += \" -fPIC\";\n";
        build_cpp += "            link_flags += \" -shared\";\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        std::cout << \"Building " + project_name_ + " (\" << build_type_ << \", \" << output_type_ << \")...\" << std::endl;\n";
        build_cpp += "        \n";
        build_cpp += "        if (output_type_ == \"static\")\n";
        build_cpp += "        {\n";
        build_cpp += "            // Compile to object files first\n";
        build_cpp += "            std::vector<std::string> object_files;\n";
        build_cpp += "            for (const auto& source : source_files)\n";
        build_cpp += "            {\n";
        build_cpp += "                std::string obj_file = build_dir + \"/\" + fs::path(source).stem().string() + \".o\";\n";
        build_cpp += "                object_files.push_back(obj_file);\n";
        build_cpp += "                \n";
        build_cpp += "                std::string compile_cmd = \"g++ \" + compile_flags + \" \" + source + \" -o \" + obj_file;\n";
        build_cpp += "                if (execute_command(compile_cmd) != 0)\n";
        build_cpp += "                {\n";
        build_cpp += "                    return 1;\n";
        build_cpp += "                }\n";
        build_cpp += "            }\n";
        build_cpp += "            \n";
        build_cpp += "            // Create static library\n";
        build_cpp += "            std::string ar_cmd = \"ar rcs \" + output_name;\n";
        build_cpp += "            for (const auto& obj : object_files)\n";
        build_cpp += "            {\n";
        build_cpp += "                ar_cmd += \" \" + obj;\n";
        build_cpp += "            }\n";
        build_cpp += "            \n";
        build_cpp += "            if (execute_command(ar_cmd) == 0)\n";
        build_cpp += "            {\n";
        build_cpp += "                std::cout << \"Static library built: \" << output_name << std::endl;\n";
        build_cpp += "                return 0;\n";
        build_cpp += "            }\n";
        build_cpp += "            return 1;\n";
        build_cpp += "        }\n";
        build_cpp += "        else\n";
        build_cpp += "        {\n";
        build_cpp += "            // Build executable or dynamic library\n";
        build_cpp += "            std::string build_cmd = \"g++ \" + compile_flags + \" \";\n";
        build_cpp += "            for (const auto& source : source_files)\n";
        build_cpp += "            {\n";
        build_cpp += "                build_cmd += source + \" \";\n";
        build_cpp += "            }\n";
        build_cpp += "            build_cmd += link_flags + \" -o \" + output_name;\n";
        build_cpp += "            \n";
        build_cpp += "            if (output_type_ == \"executable\")\n";
        build_cpp += "            {\n";
        build_cpp += "                build_cmd += \" -static\";  // Static executable\n";
        build_cpp += "            }\n";
        build_cpp += "            \n";
        build_cpp += "            if (execute_command(build_cmd) == 0)\n";
        build_cpp += "            {\n";
        build_cpp += "                if (output_type_ == \"executable\")\n";
        build_cpp += "                {\n";
        build_cpp += "                    std::cout << \"Executable built: \" << output_name << std::endl;\n";
        build_cpp += "                }\n";
        build_cpp += "                else\n";
        build_cpp += "                {\n";
        build_cpp += "                    std::cout << \"Dynamic library built: \" << output_name << std::endl;\n";
        build_cpp += "                }\n";
        build_cpp += "                return 0;\n";
        build_cpp += "            }\n";
        build_cpp += "            return 1;\n";
        build_cpp += "        }\n";
        build_cpp += "    }\n";
        build_cpp += "};\n\n";
        build_cpp += "int main(int argc, char* argv[])\n";
        build_cpp += "{\n";
        build_cpp += "    try\n";
        build_cpp += "    {\n";
        build_cpp += "        BuildSystem builder;\n";
        build_cpp += "        \n";
        build_cpp += "        // Parse command line arguments\n";
        build_cpp += "        for (int i = 1; i < argc; ++i)\n";
        build_cpp += "        {\n";
        build_cpp += "            std::string arg = argv[i];\n";
        build_cpp += "            if (arg == \"--debug\" || arg == \"-d\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_build_type(\"debug\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--release\" || arg == \"-r\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_build_type(\"release\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--executable\" || arg == \"--e\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"executable\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--static\" || arg == \"--s\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"static\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--dynamic\" || arg == \"--d\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"dynamic\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--help\" || arg == \"-h\")\n";
        build_cpp += "            {\n";
        build_cpp += "                std::cout << \"Usage: \" << argv[0] << \" [options]\\n\";\n";
        build_cpp += "                std::cout << \"Options:\\n\";\n";
        build_cpp += "                std::cout << \"  --debug, -d      Build in debug mode\\n\";\n";
        build_cpp += "                std::cout << \"  --release, -r    Build in release mode\\n\";\n";
        build_cpp += "                std::cout << \"  --executable, --e Build static executable (default)\\n\";\n";
        build_cpp += "                std::cout << \"  --static, --s    Build static library\\n\";\n";
        build_cpp += "                std::cout << \"  --dynamic, --d   Build dynamic library\\n\";\n";
        build_cpp += "                std::cout << \"  --help, -h       Show this help message\\n\";\n";
        build_cpp += "                return 0;\n";
        build_cpp += "            }\n";
        build_cpp += "            else\n";
        build_cpp += "            {\n";
        build_cpp += "                std::cerr << \"Unknown option: \" << arg << std::endl;\n";
        build_cpp += "                return 1;\n";
        build_cpp += "            }\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        int result = builder.build();\n";
        build_cpp += "        if (result == 0)\n";
        build_cpp += "        {\n";
        build_cpp += "            std::cout << \"Build completed!\" << std::endl;\n";
        build_cpp += "        }\n";
        build_cpp += "        return result;\n";
        build_cpp += "    }\n";
        build_cpp += "    catch (const std::exception& e)\n";
        build_cpp += "    {\n";
        build_cpp += "        std::cerr << \"Build failed: \" << e.what() << std::endl;\n";
        build_cpp += "        return 1;\n";
        build_cpp += "    }\n";
        build_cpp += "}\n";
        
        write_file(project_path_ + "/builder.cpp", build_cpp);
    }
    
    void create_vscode_config()
    {
        std::cout << "Creating VSCode configuration..." << std::endl;
        
        std::string vscode_settings = "{\n";
        vscode_settings += "    \"C_Cpp.default.compilerPath\": \"/usr/bin/g++\",\n";
        vscode_settings += "    \"C_Cpp.default.intelliSenseMode\": \"linux-gcc-x64\",\n";
        vscode_settings += "    \"C_Cpp.default.cppStandard\": \"c++23\",\n";
        vscode_settings += "    \"C_Cpp.default.cStandard\": \"c17\",\n";
        vscode_settings += "    \"C_Cpp.default.includePath\": [\n";
        vscode_settings += "        \"${workspaceFolder}/include\",\n";
        vscode_settings += "        \"${workspaceFolder}/.vcpkg/installed/x64-linux/include\"\n";
        vscode_settings += "    ],\n";
        vscode_settings += "    \"C_Cpp.default.defines\": [],\n";
        vscode_settings += "    \"editor.formatOnSave\": true,\n";
        vscode_settings += "    \"files.associations\": {\n";
        vscode_settings += "        \"*.hpp\": \"cpp\",\n";
        vscode_settings += "        \"*.cpp\": \"cpp\",\n";
        vscode_settings += "        \"*.h\": \"c\",\n";
        vscode_settings += "        \"*.c\": \"c\"\n";
        vscode_settings += "    },\n";
        vscode_settings += "    \"C_Cpp.clang_format_style\": \"{ BasedOnStyle: LLVM, IndentWidth: 4, ColumnLimit: 100, BreakBeforeBraces: Allman }\"\n";
        vscode_settings += "}";
        
        write_file(project_path_ + "/.vscode/settings.json", vscode_settings);
        
        std::string vscode_tasks = "{\n";
        vscode_tasks += "    \"version\": \"2.0.0\",\n";
        vscode_tasks += "    \"tasks\": [\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Compile Build System\",\n";
        vscode_tasks += "            \"command\": \"g++\",\n";
        vscode_tasks += "            \"args\": [\"-std=c++23\", \"builder.cpp\", \"-o\", \"builder\"],\n";
        vscode_tasks += "            \"group\": \"build\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        },\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Build Debug Executable\",\n";
        vscode_tasks += "            \"command\": \"./builder\",\n";
        vscode_tasks += "            \"args\": [\"--debug\", \"--executable\"],\n";
        vscode_tasks += "            \"group\": {\n";
        vscode_tasks += "                \"kind\": \"build\",\n";
        vscode_tasks += "                \"isDefault\": true\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"dependsOn\": \"Compile Build System\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        },\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Build Release Executable\",\n";
        vscode_tasks += "            \"command\": \"./builder\",\n";
        vscode_tasks += "            \"args\": [\"--release\", \"--executable\"],\n";
        vscode_tasks += "            \"group\": \"build\",\n";
        vscode_tasks += "            \"dependsOn\": \"Compile Build System\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        },\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Build Static Library\",\n";
        vscode_tasks += "            \"command\": \"./builder\",\n";
        vscode_tasks += "            \"args\": [\"--release\", \"--static\"],\n";
        vscode_tasks += "            \"group\": \"build\",\n";
        vscode_tasks += "            \"dependsOn\": \"Compile Build System\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        }\n";
        vscode_tasks += "    ]\n";
        vscode_tasks += "}";
        
        write_file(project_path_ + "/.vscode/tasks.json", vscode_tasks);
    }
    
    void create_readme()
    {
        std::cout << "Creating README..." << std::endl;
        
        std::string readme_content = "# " + project_name_ + "\n\n";
        readme_content += "A minimalistic C++ project with vcpkg package management and command-line build system.\n\n";
        readme_content += "## Features\n\n";
        readme_content += "- Modern C++23 support\n";
        readme_content += "- Local vcpkg package manager integration (copied from template)\n";
        readme_content += "- Command-line build system (no CMake/Makefile required)\n";
        readme_content += "- Support for static executables, static libraries, and dynamic libraries\n";
        readme_content += "- VSCode configuration\n";
        readme_content += "- Template header files (asyncops.hpp, raiiiofsw.hpp, stringformers.hpp, utilities.hpp)\n";
        readme_content += "- Pythonic naming convention (PascalCase for classes, snake_case for everything else)\n";
        readme_content += "- Allman indentation style\n\n";
        readme_content += "## Project Structure\n\n";
        readme_content += "```\n";
        readme_content += project_name_ + "/\n";
        readme_content += "├── include/                 # Header files (including template headers)\n";
        readme_content += "├── src/                     # Source files\n";
        readme_content += "├── tests/                   # Test files\n";
        readme_content += "├── build/                   # Build outputs\n";
        readme_content += "│   ├── debug/              # Debug builds\n";
        readme_content += "│   └── release/            # Release builds\n";
        readme_content += "├── .vcpkg/                 # Local vcpkg installation (copied from template)\n";
        readme_content += "├── .vscode/                # VSCode configuration\n";
        readme_content += "├── builder.cpp             # Build system source\n";
        readme_content += "├── vcpkg.json             # Package manifest\n";
        readme_content += "└── README.md              # This file\n";
        readme_content += "```\n\n";
        readme_content += "## Build Instructions\n\n";
        readme_content += "### Initial Setup\n\n";
        readme_content += "1. Compile the build system:\n";
        readme_content += "```bash\n";
        readme_content += "g++ -std=c++23 builder.cpp -o builder\n";
        readme_content += "```\n\n";
        readme_content += "### Building the Project\n\n";
        readme_content += "#### Build static executable (default):\n";
        readme_content += "```bash\n";
        readme_content += "./builder --release --executable\n";
        readme_content += "```\n\n";
        readme_content += "#### Build in debug mode:\n";
        readme_content += "```bash\n";
        readme_content += "./builder --debug --executable\n";
        readme_content += "```\n\n";
        readme_content += "#### Build static library:\n";
        readme_content += "```bash\n";
        readme_content += "./builder --release --static\n";
        readme_content += "```\n\n";
        readme_content += "#### Build dynamic library:\n";
        readme_content += "```bash\n";
        readme_content += "./builder --release --dynamic\n";
        readme_content += "```\n\n";
        readme_content += "### Build Options\n\n";
        readme_content += "- `--debug, -d`: Build in debug mode (with debugging symbols)\n";
        readme_content += "- `--release, -r`: Build in release mode (optimized)\n";
        readme_content += "- `--executable, --e`: Build static executable (default)\n";
        readme_content += "- `--static, --s`: Build static library\n";
        readme_content += "- `--dynamic, --d`: Build dynamic library\n\n";
        readme_content += "## Package Management\n\n";
        readme_content += "The project uses a local vcpkg copy from the template directory for fast setup.\n\n";
        readme_content += "Add packages to `vcpkg.json` and run:\n";
        readme_content += "```bash\n";
        readme_content += "./.vcpkg/vcpkg install\n";
        readme_content += "```\n\n";
        readme_content += "## Template Headers\n\n";
        readme_content += "The following header files are automatically copied from the template:\n";
        readme_content += "- `asyncops.hpp`: Async operations and coroutines utilities\n";
        readme_content += "- `raiiiofsw.hpp`: RAII filesystem wrappers\n";
        readme_content += "- `stringformers.hpp`: String formatting and manipulation utilities\n";
        readme_content += "- `utilities.hpp`: General utility functions\n\n";
        readme_content += "## Development\n\n";
        readme_content += "The project follows these conventions:\n";
        readme_content += "- **Classes/Structs**: PascalCase (e.g., `ExampleClass`)\n";
        readme_content += "- **Methods/Variables/Constants**: snake_case (e.g., `get_name()`, `project_name_`)\n";
        readme_content += "- **Indentation**: Allman style (braces on new lines)\n\n";
        readme_content += "## Requirements\n\n";
        readme_content += "- GCC 11+ or Clang 14+ with C++23 support\n";
        readme_content += "- Linux x64 (Ubuntu/Debian)\n\n";
        readme_content += "## License\n\n";
        readme_content += "This project is provided as-is for educational and development purposes.\n";
        
        write_file(project_path_ + "/README.md", readme_content);
    }
    
    void initialize_project()
    {
        try
        {
            create_directory_structure();
            setup_vcpkg();
            create_source_files();
            copy_template_headers();
            create_build_system();
            create_vscode_config();
            create_readme();
            
            std::cout << "\n=== Project Setup Complete ===" << std::endl;
            std::cout << "Project: " << project_name_ << std::endl;
            std::cout << "Location: " << fs::absolute(project_path_) << std::endl;
            std::cout << "\nNext steps:" << std::endl;
            std::cout << "1. cd " << project_path_ << std::endl;
            std::cout << "2. g++ -std=c++23 builder.cpp -o builder" << std::endl;
            std::cout << "3. ./builder --release --executable" << std::endl;
            std::cout << "4. ./build/release/" << project_name_ << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error during project initialization: " << e.what() << std::endl;
            throw;
        }
    }
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " <project_path>" << std::endl;
            std::cerr << "Example: " << argv[0] << " ~/projects/my-new-project" << std::endl;
            return 1;
        }
        
        std::string project_path = argv[1];
        
        // Expand ~ to home directory if needed
        if (project_path.front() == '~')
        {
            const char* home = std::getenv("HOME");
            if (home)
            {
                project_path = std::string(home) + project_path.substr(1);
            }
        }
        
        if (fs::exists(project_path))
        {
            std::cerr << "Error: Directory already exists: " << project_path << std::endl;
            return 1;
        }
        
        std::cout << "Creating C++ project: " << project_path << std::endl;
        
        CppProjectInitializer initializer(project_path);
        initializer.initialize_project();
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to create project: " << e.what() << std::endl;
        return 1;
    }
}
