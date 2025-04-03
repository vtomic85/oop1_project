#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <map>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

std::string promptLabel = "cmd> ";

void echo(const std::vector<std::string> &args, std::ostream &out)
{
    for (size_t i = 1; i < args.size(); ++i)
        out << args[i] << " ";
    out << std::endl;
}

void changePrompt(const std::vector<std::string> &args)
{
    if (args.size() > 1)
        promptLabel = args[1] + "> ";
}

void printTime(std::ostream &out)
{
    std::time_t now = std::time(nullptr);
    std::tm *timeinfo = std::localtime(&now);
    char buffer[9];
    std::strftime(buffer, 9, "%H:%M:%S", timeinfo);
    out << buffer << std::endl;
}

void printDate(std::ostream& out) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    
    // Convert to local time
    std::tm localTime;
    
    #ifdef _WIN32  // Windows
        localtime_s(&localTime, &currentTime);
    #else         // Linux, macOS
        localtime_r(&currentTime, &localTime);
    #endif

    // Print formatted date and time
    out << std::put_time(&localTime, "%Y-%m-%d") << std::endl;
}

void touchFile(const std::vector<std::string> &args)
{
    if (args.size() < 2)
        return;
    std::ofstream file(args[1]);
    if (!file)
        std::cout << "Error: Cannot create file." << std::endl;
}

void removeFile(const std::vector<std::string> &args)
{
    if (args.size() < 2)
        return;
    if (std::remove(args[1].c_str()) != 0)
        std::cerr << "Error: Cannot remove file." << std::endl;
}

void wordCount(const std::vector<std::string> &args, std::ostream &out)
{
    if (args.size() < 3)
    {
        std::cerr << "Usage: wc -w|-c \"text\" or wc -w|-c filename" << std::endl;
        return;
    }

    std::string input;
    bool isFile = true;

    // Check if the input is a quoted string (i.e., if it starts and ends with a quote)
    if (args[2].front() == '"')
    {
        isFile = false;
        input = args[2].substr(1); // Start after the opening quote

        // Concatenate the rest of the arguments until we find the closing quote
        for (size_t i = 3; i < args.size(); ++i)
        {
            input += " " + args[i];
            if (args[i].back() == '"')
            {
                input = input.substr(0, input.size() - 1); // Remove the ending quote
                break;
            }
        }
    }
    else
    {
        // Assume it's a filename
        std::ifstream file(args[2]);
        if (!file)
        {
            std::cerr << "Error: Cannot open file " << args[2] << std::endl;
            return;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        input = buffer.str();
    }

    // Perform word or character count
    if (args[1] == "-w")
    {
        std::istringstream iss(input);
        int count = 0;
        std::string word;
        while (iss >> word)
            count++;
        out << count << std::endl;
    }
    else if (args[1] == "-c")
    {
        out << input.length() << std::endl;
    }
    else
    {
        std::cerr << "Invalid option. Use -w for word count or -c for character count." << std::endl;
    }
}

void trReplace(std::vector<std::string> &args, std::ostream &out)
{ // Pass by reference
    if (args.size() < 3)
        return;
    std::string &text = args[1];
    std::string from = args[2];
    std::string to = (args.size() > 3) ? args[3] : ""; // Default to empty string if missing

    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos)
    {
        text.replace(pos, from.length(), to);
        pos += to.length(); // Move past the replaced substring
    }

    out << text << std::endl;
}

void headLines(const std::vector<std::string> &args, std::ostream &out)
{
    if (args.size() < 3)
        return;

    // Extract individual digits as separate numbers (keeping the single-digit approach)
    std::string numbers = args[1].substr(1);
    std::vector<int> lines;
    for (char c : numbers)
    {
        if (isdigit(c))
            lines.push_back(c - '0'); // Convert character to integer
    }

    // Join remaining arguments into a single string (handles multi-word input)
    std::ostringstream inputStream;
    for (size_t i = 2; i < args.size(); ++i)
    {
        if (i > 2)
            inputStream << " ";
        inputStream << args[i];
    }
    std::istringstream iss(inputStream.str());

    std::string line;
    int lineNumber = 1;
    while (std::getline(iss, line))
    {
        if (std::find(lines.begin(), lines.end(), lineNumber) != lines.end())
        {
            out << line << std::endl;
        }
        lineNumber++;
    }
}

void executeCommand(const std::string &command, std::ostream &out);

void executeBatch(const std::vector<std::string> &commands)
{
    for (const auto &cmd : commands)
    {
        executeCommand(cmd, std::cout);
    }
}

void executeCommand(const std::string &command, std::ostream &out)
{
    std::istringstream iss(command);
    std::vector<std::string> args;
    std::string token;
    bool inQuotes = false;
    std::string quotedArg;

    while (iss >> token)
    {
        if (token.front() == '"' && !inQuotes)
        {
            inQuotes = true;
            quotedArg = token.substr(1); // Remove the opening quote
        }
        else if (token.back() == '"' && inQuotes)
        {
            inQuotes = false;
            quotedArg += " " + token.substr(0, token.size() - 1); // Remove closing quote
            args.push_back(quotedArg);
        }
        else if (inQuotes)
        {
            quotedArg += " " + token;
        }
        else
        {
            args.push_back(token);
        }
    }

    if (args.empty())
        return;

    if (args[0] == "echo")
        echo(args, out);
    else if (args[0] == "prompt")
        changePrompt(args);
    else if (args[0] == "time")
        printTime(out);
    else if (args[0] == "date")
        printDate(out);
    else if (args[0] == "touch")
        touchFile(args);
    else if ((args[0] == "rm") || (args[0] == "truncate"))
        removeFile(args);
    else if (args[0] == "wc")
        wordCount(args, out);
    else if (args[0] == "tr")
        trReplace(args, out);
    else if (args[0].find("head") == 0)
        headLines(args, out);
    else if (args[0] == "batch")
    {
        std::vector<std::string> batchCommands;
        std::string line;
        while (std::getline(std::cin, line) && !line.empty())
        {
            batchCommands.push_back(line);
        }
        executeBatch(batchCommands);
    }
    else
        std::cerr << "Unknown command: " << args[0] << std::endl;
}

void processPipes(std::string input)
{
    std::vector<std::istringstream> pipes;
    std::istringstream stream(input);
    std::string segment;
    while (std::getline(stream, segment, '|'))
    {
        pipes.emplace_back(segment);
    }
    std::ostringstream out;
    for (size_t i = 0; i < pipes.size(); ++i)
    {
        std::string command;
        std::getline(pipes[i], command);
        executeCommand(command, out);
        if (i < pipes.size() - 1)
        {
            out.seekp(0, std::ios::beg);
            out.str("");
        }
    }
    std::cout << out.str();
}

int main()
{
    std::string input;
    while (true)
    {
        std::cout << promptLabel;
        std::getline(std::cin, input);
        if (input == "exit")
            break;
        if (input.find('|') != std::string::npos)
            processPipes(input);
        else
            executeCommand(input, std::cout);
    }
    return 0;
}
