#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <map>
#include <cctype>
#include <cstdio>

namespace fs = std::filesystem;

// entry-point
int main(int argc, char* argv[])
{
    std::string project_path, project_name;
    
    // Helper method to execute system commands
    auto execute_command = [](const std::string& command)
    {
        std::cout << "Executing: " << command << std::endl;
        return std::system(command.c_str());
    };
    
    // Write content to file
    auto write_file = [](const fs::path& file_path, const std::string& content)
    {
        std::ofstream file(file_path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not create file: " + file_path.string());
        }
        file << content;
        file.flush();
        file.close();
        std::cout << "Created: " << file_path << std::endl;
    };
    
    // Helper method to sanitize project name for C++ identifiers
    auto sanitize_cpp_name = [](const std::string& name)
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
    };

    // Create directory structure
    auto create_directory_structure = [&]()
    {
        std::cout << "Creating directory structure..." << std::endl;
        
        std::vector<std::string> directories = {
            project_path,
            project_path + "/include",
            project_path + "/src", 
            project_path + "/build",
            project_path + "/build/debug",
            project_path + "/build/release",
            project_path + "/tests",
            project_path + "/.vscode"
        };
        
        for (const auto& dir : directories)
        {
            fs::create_directories(dir);
            std::cout << "Created directory: " << dir << std::endl;
        }
    };
    
    // Setup vcpkg from template with full automation
    auto setup_vcpkg = [&]()
    {
        std::cout << "Setting up vcpkg with full automation..." << std::endl;
        
        const std::string vcpkg_src_path = fs::current_path().string() + "/templates/vcpkg";
        const std::string vcpkg_dst_path = project_path + "/vcpkg";
        
        // Check if template vcpkg exists
        if (!fs::exists(fs::path(vcpkg_src_path)))
        {
            std::string errmsg{"Warning: Template vcpkg not found at " };
            errmsg += vcpkg_src_path;
            errmsg += "\nSkipping vcpkg setup. You can manually set up vcpkg if needed.";
            throw std::runtime_error(errmsg.c_str());
        }
        
        // Copy template vcpkg to project directory
        std::cout << "Copying vcpkg from template directory..." << std::endl;
        std::string copy_cmd = "cp -r " + vcpkg_src_path + " " + vcpkg_dst_path;
        if (execute_command(copy_cmd) != 0)
        {
            throw std::runtime_error("Warning: Failed to copy vcpkg from template. You can manually set up vcpkg if needed.\n");
        }
        
        // Bootstrap vcpkg
        std::cout << "Bootstrapping vcpkg..." << std::endl;
        std::string bootstrap_cmd = "cd " + vcpkg_dst_path + " && ./bootstrap-vcpkg.sh";
        if (execute_command(bootstrap_cmd) != 0)
        {
            std::cout << "Warning: vcpkg bootstrap failed. You may need to run './vcpkg/bootstrap-vcpkg.sh' manually." << std::endl;
        }
        else
        {
            std::cout << "vcpkg bootstrapped successfully!" << std::endl;
        }
        
        // Get latest vcpkg baseline automatically
        std::cout << "Getting latest vcpkg baseline..." << std::endl;
        std::string baseline_cmd = "cd " + vcpkg_dst_path + " && git rev-parse HEAD";
        std::string baseline_hash;
        
        // Execute and capture output
        FILE* pipe = popen(baseline_cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                baseline_hash = std::string(buffer);
                // Remove newline
                baseline_hash.erase(baseline_hash.find_last_not_of(" \n\r\t") + 1);
            }
            pclose(pipe);
        }
        
        if (baseline_hash.empty()) {
            baseline_hash = "latest"; // fallback
            std::cout << "Warning: Could not determine vcpkg baseline, using 'latest'" << std::endl;
        } else {
            std::cout << "Using vcpkg baseline: " << baseline_hash << std::endl;
        }
        
        // Setup binary caching automatically
        std::cout << "Setting up binary caching..." << std::endl;
        std::string cache_dir = project_path + "/.vcpkg-cache";
        fs::create_directories(cache_dir);
        
        // Create vcpkg configuration for binary caching
        std::string vcpkg_config = "{\n";
        vcpkg_config += "  \"default-registry\": {\n";
        vcpkg_config += "    \"kind\": \"git\",\n";
        vcpkg_config += "    \"repository\": \"https://github.com/Microsoft/vcpkg\",\n";
        vcpkg_config += "    \"baseline\": \"" + baseline_hash + "\"\n";
        vcpkg_config += "  },\n";
        vcpkg_config += "  \"registries\": []\n";
        vcpkg_config += "}";
        
        write_file(project_path + "/vcpkg-configuration.json", vcpkg_config);
        
        // Create vcpkg.json manifest with automatic baseline
        std::string vcpkg_manifest = "{\n";
        vcpkg_manifest += "  \"$schema\": \"https://raw.githubusercontent.com/Microsoft/vcpkg/main/scripts/vcpkg.schema.json\",\n";
        vcpkg_manifest += "  \"name\": \"" + project_name + "\",\n";
        vcpkg_manifest += "  \"version\": \"1.0.0\",\n";
        vcpkg_manifest += "  \"description\": \"" + project_name + " C++ project with automated vcpkg management\",\n";
        vcpkg_manifest += "  \"homepage\": \"\",\n";
        vcpkg_manifest += "  \"documentation\": \"\",\n";
        vcpkg_manifest += "  \"supports\": \"linux\",\n";
        vcpkg_manifest += "  \"dependencies\": [\n";
        vcpkg_manifest += "    \n";
        vcpkg_manifest += "  ],\n";
        vcpkg_manifest += "  \"builtin-baseline\": \"" + baseline_hash + "\"\n";
        vcpkg_manifest += "}";
        
        write_file(project_path + "/vcpkg.json", vcpkg_manifest);
        
        // Create automatic package installer script
        std::string install_script = "#!/bin/bash\n";
        install_script += "# Automated vcpkg package installer\n";
        install_script += "# Usage: ./install-packages.sh [package1] [package2] ...\n\n";
        install_script += "set -e\n\n";
        install_script += "SCRIPT_DIR=\"$(cd \"$(dirname \"${BASH_SOURCE[0]}\")\" && pwd)\"\n";
        install_script += "VCPKG_DIR=\"$SCRIPT_DIR/vcpkg\"\n";
        install_script += "CACHE_DIR=\"$SCRIPT_DIR/.vcpkg-cache\"\n\n";
        install_script += "# Set up binary caching\n";
        install_script += "export VCPKG_BINARY_SOURCES=\"clear;files,$CACHE_DIR,readwrite\"\n\n";
        install_script += "echo \"Installing packages with binary caching...\"\n";
        install_script += "echo \"Cache directory: $CACHE_DIR\"\n\n";
        install_script += "if [ $# -eq 0 ]; then\n";
        install_script += "    echo \"Installing packages from vcpkg.json...\"\n";
        install_script += "    cd \"$SCRIPT_DIR\"\n";
        install_script += "    \"$VCPKG_DIR/vcpkg\" install\n";
        install_script += "else\n";
        install_script += "    echo \"Installing specified packages: $@\"\n";
        install_script += "    cd \"$SCRIPT_DIR\"\n";
        install_script += "    \"$VCPKG_DIR/vcpkg\" install \"$@\"\n";
        install_script += "fi\n\n";
        install_script += "echo \"Package installation complete!\"\n";
        install_script += "echo \"Binary cache size: $(du -sh \"$CACHE_DIR\" 2>/dev/null | cut -f1 || echo 'N/A')\"\n";
        
        write_file(project_path + "/install-packages.sh", install_script);
        
        // Make script executable
        execute_command("chmod +x " + project_path + "/install-packages.sh");
        
        // Create vcpkg update script
        std::string update_script = "#!/bin/bash\n";
        update_script += "# Automated vcpkg baseline updater\n";
        update_script += "# This script updates vcpkg to the latest version and updates the baseline\n\n";
        update_script += "set -e\n\n";
        update_script += "SCRIPT_DIR=\"$(cd \"$(dirname \"${BASH_SOURCE[0]}\")\" && pwd)\"\n";
        update_script += "VCPKG_DIR=\"$SCRIPT_DIR/vcpkg\"\n\n";
        update_script += "echo \"Updating vcpkg to latest version...\"\n";
        update_script += "cd \"$VCPKG_DIR\"\n";
        update_script += "git pull origin master\n";
        update_script += "./bootstrap-vcpkg.sh\n\n";
        update_script += "# Get new baseline\n";
        update_script += "NEW_BASELINE=$(git rev-parse HEAD)\n";
        update_script += "echo \"New baseline: $NEW_BASELINE\"\n\n";
        update_script += "# Update vcpkg.json with new baseline\n";
        update_script += "cd \"$SCRIPT_DIR\"\n";
        update_script += "sed -i \"s/\\\"builtin-baseline\\\": \\\".*\\\"/\\\"builtin-baseline\\\": \\\"$NEW_BASELINE\\\"/\" vcpkg.json\n";
        update_script += "sed -i \"s/\\\"baseline\\\": \\\".*\\\"/\\\"baseline\\\": \\\"$NEW_BASELINE\\\"/\" vcpkg-configuration.json\n\n";
        update_script += "echo \"vcpkg updated successfully!\"\n";
        update_script += "echo \"Updated vcpkg.json and vcpkg-configuration.json with new baseline: $NEW_BASELINE\"\n";
        update_script += "echo \"Run ./install-packages.sh to reinstall packages with the new version\"\n";
        
        write_file(project_path + "/update-vcpkg.sh", update_script);
        
        // Make script executable
        execute_command("chmod +x " + project_path + "/update-vcpkg.sh");
        
        std::cout << "vcpkg setup complete with full automation!" << std::endl;
        std::cout << "- Binary caching enabled (cache: .vcpkg-cache/)" << std::endl;
        std::cout << "- Use ./install-packages.sh to install packages" << std::endl;
        std::cout << "- Use ./update-vcpkg.sh to update vcpkg baseline" << std::endl;
    };

    // Create initial source files
    auto copy_template_headers = [&]()
    {
        std::cout << "Copying template header files..." << std::endl;
        
        auto include_src_path = fs::current_path() / fs::path("templates/include");
        auto include_dst_path = fs::path(project_path) / fs::path("include");
        
        fs::copy(include_src_path, include_dst_path, fs::copy_options::skip_existing | fs::copy_options::recursive);
        std::cout << "Template header files copied!" << std::endl;
    };

    // Create source file
    auto create_source_files = [&]()
    {
        std::cout << "Creating source files..." << std::endl;
        
        // Main source file template
        std::string main_cpp_template = "#include <iostream>\n";
        main_cpp_template += "#include <exception>\n";
        main_cpp_template += "#include <cstdlib>\n\n";
        main_cpp_template += "// entry-point\n";
        main_cpp_template += "int main()\n";
        main_cpp_template += "{\n";
        main_cpp_template += "    try\n";
        main_cpp_template += "    {\n";
        main_cpp_template += "        // start here ...\n";
        main_cpp_template += "        \n";
        main_cpp_template += "        return EXIT_SUCCESS;\n";
        main_cpp_template += "    }\n";
        main_cpp_template += "    catch (const std::exception& xxx)\n";
        main_cpp_template += "    {\n";
        main_cpp_template += "        std::cerr << \"Error: \" << xxx.what() << std::endl;\n";
        main_cpp_template += "        return EXIT_FAILURE;\n";
        main_cpp_template += "    }\n";
        main_cpp_template += "}\n";
        
        write_file(project_path + "/src/main.cpp", main_cpp_template);
    };

    // Create build system
    auto create_build_system = [&]()
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
        build_cpp += "        if (fs::exists(\"vcpkg/installed/x64-linux/include\"))\n";
        build_cpp += "        {\n";
        build_cpp += "            compile_flags += \" -Ivcpkg/installed/x64-linux/include\";\n";
        build_cpp += "            link_flags = \"-Lvcpkg/installed/x64-linux/lib\";\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        if (output_type_ == \"executable\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/\" + \"" + project_name + "\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (output_type_ == \"static\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/lib\" + \"" + project_name + "\" + \".a\";\n";
        build_cpp += "            compile_flags += \" -c\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (output_type_ == \"dynamic\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/lib\" + \"" + project_name + "\" + \".so\";\n";
        build_cpp += "            compile_flags += \" -fPIC\";\n";
        build_cpp += "            link_flags += \" -shared\";\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        std::cout << \"Building " + project_name + " (\" << build_type_ << \", \" << output_type_ << \")...\" << std::endl;\n";
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
        build_cpp += "            if (arg == \"--debug\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_build_type(\"debug\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--release\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_build_type(\"release\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--executable\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"executable\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--static\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"static\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--dynamic\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"dynamic\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--help\")\n";
        build_cpp += "            {\n";
        build_cpp += "                std::cout << \"Usage: \" << argv[0] << \" [options]\\n\";\n";
        build_cpp += "                std::cout << \"Options:\\n\";\n";
        build_cpp += "                std::cout << \"  --debug          Build in debug mode\\n\";\n";
        build_cpp += "                std::cout << \"  --release        Build in release mode\\n\";\n";
        build_cpp += "                std::cout << \"  --executable     Build static executable (default)\\n\";\n";
        build_cpp += "                std::cout << \"  --static         Build static library\\n\";\n";
        build_cpp += "                std::cout << \"  --dynamic        Build dynamic library\\n\";\n";
        build_cpp += "                std::cout << \"  --help           Show this help message\\n\";\n";
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
        
        write_file(project_path + "/builder.cpp", build_cpp);
    };
    
    // Create VSCode configuration
    auto create_vscode_config = [&]()
    {
        std::cout << "Creating VSCode configuration..." << std::endl;
        
        std::string vscode_settings = "{\n";
        vscode_settings += "    \"C_Cpp.default.compilerPath\": \"/usr/bin/g++\",\n";
        vscode_settings += "    \"C_Cpp.default.intelliSenseMode\": \"linux-gcc-x64\",\n";
        vscode_settings += "    \"C_Cpp.default.cppStandard\": \"c++23\",\n";
        vscode_settings += "    \"C_Cpp.default.cStandard\": \"c17\",\n";
        vscode_settings += "    \"C_Cpp.default.includePath\": [\n";
        vscode_settings += "        \"${workspaceFolder}/include\",\n";
        vscode_settings += "        \"${workspaceFolder}/vcpkg/installed/x64-linux/include\"\n";
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
        
        write_file(project_path + "/.vscode/settings.json", vscode_settings);
        
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
        
        write_file(project_path + "/.vscode/tasks.json", vscode_tasks);
    };

    // Create README file
    auto create_readme = [&]()
    {
        std::cout << "Creating README..." << std::endl;
        
        std::string readme_content = "# " + project_name + "\n\n";
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
        readme_content += project_name + "/\n";
        readme_content += "├── include/                 # Header files (including template headers)\n";
        readme_content += "├── src/                     # Source files\n";
        readme_content += "├── tests/                   # Test files\n";
        readme_content += "├── build/                   # Build outputs\n";
        readme_content += "│   ├── debug/              # Debug builds\n";
        readme_content += "│   └── release/            # Release builds\n";
        readme_content += "├── vcpkg/                   # Local vcpkg installation\n";
        readme_content += "├── .vcpkg-cache/            # Binary package cache\n";
        readme_content += "├── .vscode/                 # VSCode configuration\n";
        readme_content += "├── builder.cpp              # Build system source\n";
        readme_content += "├── vcpkg.json               # Package manifest\n";
        readme_content += "├── vcpkg-configuration.json # vcpkg configuration\n";
        readme_content += "├── install-packages.sh      # Automated package installer\n";
        readme_content += "├── update-vcpkg.sh          # vcpkg updater\n";
        readme_content += "└── README.md                # This file\n";
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
        readme_content += "- `--debug`: Build in debug mode (with debugging symbols)\n";
        readme_content += "- `--release`: Build in release mode (optimized)\n";
        readme_content += "- `--executable`: Build static executable (default)\n";
        readme_content += "- `--static`: Build static library\n";
        readme_content += "- `--dynamic`: Build dynamic library\n\n";
        readme_content += "## Package Management\n\n";
        readme_content += "The project uses a fully automated vcpkg setup with binary caching and automatic baseline management.\n\n";
        readme_content += "### Installing Packages\n\n";
        readme_content += "```bash\n";
        readme_content += "# Install packages from vcpkg.json\n";
        readme_content += "./install-packages.sh\n\n";
        readme_content += "# Install specific packages\n";
        readme_content += "./install-packages.sh fmt spdlog nlohmann-json\n";
        readme_content += "```\n\n";
        readme_content += "### Updating vcpkg\n\n";
        readme_content += "```bash\n";
        readme_content += "# Update vcpkg to latest version and baseline\n";
        readme_content += "./update-vcpkg.sh\n";
        readme_content += "```\n\n";
        readme_content += "### Adding Dependencies\n\n";
        readme_content += "Edit `vcpkg.json` and add packages to the dependencies array:\n";
        readme_content += "```json\n";
        readme_content += "{\n";
        readme_content += "  \"dependencies\": [\n";
        readme_content += "    \"fmt\",\n";
        readme_content += "    \"spdlog\",\n";
        readme_content += "    \"nlohmann-json\"\n";
        readme_content += "  ]\n";
        readme_content += "}\n";
        readme_content += "```\n\n";
        readme_content += "Then run `./install-packages.sh` to install them.\n\n";
        readme_content += "### Features\n\n";
        readme_content += "- **Binary Caching**: Packages are cached in `.vcpkg-cache/` for faster rebuilds\n";
        readme_content += "- **Automatic Baseline**: vcpkg baseline is determined automatically\n";
        readme_content += "- **Easy Updates**: One command to update vcpkg and baseline\n";
        readme_content += "- **Project Isolation**: Each project has its own vcpkg and cache\n\n";
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
        readme_content += "- Linux x64 (Ubuntu/Debian)\n";
        readme_content += "- Git (for vcpkg management)\n";
        readme_content += "- Internet connection (for initial package downloads)\n\n";
        readme_content += "## License\n\n";
        readme_content += "This project is provided as-is for educational and development purposes.\n";
        
        write_file(project_path + "/README.md", readme_content);
    };

    try
    {
        if (argc != 2)
        {
            std::string errmsg{ "Usage: "};
            errmsg += argv[0];
            errmsg += " <project_path>\n";
            errmsg += "Example: ";
            errmsg += argv[0];
            errmsg += " ~/projects/my-new-project\n";
            throw std::runtime_error(errmsg.c_str());
        }
        
        project_path = argv[1];
        
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
            std::string errmsg{ "Error: Directory already exists: " };
            errmsg += project_path;
            throw std::runtime_error(errmsg.c_str());
        }
        
        std::cout << "Creating C++ project: " << project_path << std::endl;
        
        fs::path path(project_path);
        project_name = sanitize_cpp_name(path.stem().string());
        
        create_directory_structure();
        setup_vcpkg();
        create_source_files();
        copy_template_headers();
        create_build_system();
        create_vscode_config();
        create_readme();
            
        std::cout << "\n=== Project Setup Complete ===" << std::endl;
        std::cout << "Project: " << project_name << std::endl;
        std::cout << "Location: " << fs::absolute(project_path) << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "1. cd " << project_path << std::endl;
        std::cout << "2. g++ -std=c++23 builder.cpp -o builder" << std::endl;
        std::cout << "3. ./builder --release --executable" << std::endl;
        std::cout << "4. ./build/release/" << project_name << std::endl;
        std::cout << "\nPackage management:" << std::endl;
        std::cout << "- Add packages to vcpkg.json" << std::endl;
        std::cout << "- Run ./install-packages.sh to install them" << std::endl;
        std::cout << "- Run ./update-vcpkg.sh to update vcpkg when needed" << std::endl;
        
        return EXIT_SUCCESS;
    }
    catch (const std::exception& xxx)
    {
        std::cerr << "Failed to create project: " << xxx.what() << std::endl;
        return EXIT_FAILURE;
    }
}
