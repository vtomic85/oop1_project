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
#include <set>
#include <functional>

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

void printDate(std::ostream &out)
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif

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

    std::string option = args[1]; // First argument should be -w or -c
    if (option != "-w" && option != "-c")
    {
        std::cerr << "Invalid option. Use -w for word count or -c for character count." << std::endl;
        return;
    }

    std::string input;
    bool isQuotedString = false;

    if (args[2].front() == '"')
    {
        isQuotedString = true;
        input = args[2].substr(1);

        for (size_t i = 3; i < args.size(); ++i)
        {
            input += " " + args[i];
            if (args[i].back() == '"')
            {
                input = input.substr(0, input.size() - 1);
                break;
            }
        }
    }
    else
    {
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

    if (option == "-w")
    {
        int wordCount = 0;
        std::istringstream stream(input);
        std::string word;
        while (stream >> word)
        {
            wordCount++;
        }
        out << wordCount << std::endl;
    }
    else if (option == "-c")
    {
        if (!input.empty() && input.back() == '"')
            input = input.substr(0, input.size() - 1);

        size_t end = input.find_last_not_of(" \n\r\t");
        if (end != std::string::npos)
            input = input.substr(0, end + 1);

        out << input.length() << std::endl;
    }
}

std::string removeQuotes(const std::string &str)
{
    if (str.empty())
        return str;

    if (str.front() == '"' && str.back() == '"')
        return str.substr(1, str.size() - 2); // Remove the surrounding quotes
    return str;
}

void trReplace(std::vector<std::string> &args, std::ostream &out)
{
    if (args.size() < 4)
    {
        std::cerr << "Usage: tr \"text\" \"from\" \"to\" or tr filename \"from\" \"to\"" << std::endl;
        return;
    }

    std::string text;
    std::string from = removeQuotes(args[2]);                        // Remove quotes if present
    std::string to = (args.size() > 3) ? removeQuotes(args[3]) : ""; // Default to empty string if missing

    // If the first parameter is a quoted string
    if (args[1].front() == '"' && args[1].back() == '"')
    {
        text = removeQuotes(args[1]); // Remove the surrounding quotes from the string
    }
    else
    {
        // Otherwise, treat it as a filename
        std::ifstream file(args[1]);
        if (!file)
        {
            std::cerr << "Error: Cannot open file " << args[1] << std::endl;
            return;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        text = buffer.str();
    }

    // Handle replacing or removing the 'from' substring
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos)
    {
        if (!to.empty())
        {
            // Replace 'from' with 'to'
            text.replace(pos, from.length(), to);
            pos += to.length(); // Move past the replaced substring
        }
        else
        {
            // If 'to' is empty, remove 'from'
            text.erase(pos, from.length());
        }
    }

    out << text << std::endl;
}

void headLines(const std::vector<std::string> &args, std::ostream &out)
{
    if (args.size() < 3)
    {
        std::cerr << "Usage: head -nXYZ filename.ext or head -nXYZ \"some text\"\n";
        return;
    }

    std::string numberString = args[1].substr(2);
    std::set<int> targetLines;

    for (char c : numberString)
    {
        if (isdigit(c))
        {
            targetLines.insert(c - '0');
        }
        else
        {
            std::cerr << "Invalid line number format.\n";
            return;
        }
    }

    std::stringstream inputStream;
    bool isQuotedText = args[2].front() == '"';

    if (isQuotedText)
    {
        std::string input;
        for (size_t i = 2; i < args.size(); ++i)
        {
            if (i > 2)
                input += " ";
            input += args[i];
        }

        if (input.front() == '"' && input.back() == '"')
        {
            input = input.substr(1, input.size() - 2);
        }

        inputStream.str(input);
    }
    else
    {
        std::ifstream file(args[2]);
        if (!file)
        {
            std::cerr << "Error: Cannot open file " << args[2] << std::endl;
            return;
        }
        inputStream << file.rdbuf();
    }

    std::string line;
    int lineNumber = 1;
    while (std::getline(inputStream, line))
    {
        if (targetLines.count(lineNumber))
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
            quotedArg = token;
        }
        else if (token.back() == '"' && inQuotes)
        {
            inQuotes = false;
            quotedArg += " " + token;
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
    std::ostringstream currentInput;
    for (size_t i = 0; i < pipes.size(); ++i)
    {
        std::string command;
        std::getline(pipes[i], command);
        if (i == 0)
            executeCommand(command, out); // For the first command
        else
        {
            out.seekp(0, std::ios::beg);
            out.str("");
            executeCommand(command, out); // For subsequent commands
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

        processPipes(input);
    }
}
