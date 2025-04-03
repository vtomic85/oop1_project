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
    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm localTime;

#ifdef _WIN32 // Windows
    localtime_s(&localTime, &currentTime);
#else // Linux, macOS
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
    if (args.size() < 3) // If not enough arguments
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

    // If input is a quoted string (with quotes around it)
    if (args[2].front() == '"')
    {
        isQuotedString = true;
        input = args[2].substr(1); // Remove the starting quote

        // Concatenate all parts of the quoted string
        for (size_t i = 3; i < args.size(); ++i)
        {
            input += " " + args[i];
            if (args[i].back() == '"')
            {
                input = input.substr(0, input.size() - 1); // Remove the closing quote
                break;
            }
        }
    }
    else
    {
        // Assume it's a filename, attempt to open the file
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

    // If option is -w (word count)
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
    // If option is -c (character count)
    else if (option == "-c")
    {
        // Remove the closing quote if it's there
        if (!input.empty() && input.back() == '"')
            input = input.substr(0, input.size() - 1); // Remove the closing quote

        // Trim trailing spaces, to avoid counting an extra space at the end
        size_t end = input.find_last_not_of(" \n\r\t");
        if (end != std::string::npos)
            input = input.substr(0, end + 1);

        out << input.length() << std::endl; // Number of characters (including spaces)
    }
}

// Helper function to remove surrounding quotes from a string
std::string removeQuotes(const std::string& str)
{
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
    {
        return str.substr(1, str.size() - 2); // Remove quotes
    }
    return str; // Return the string unchanged if no quotes
}

void trReplace(std::vector<std::string> &args, std::ostream &out)
{
    if (args.size() < 3)
    {
        std::cerr << "Usage: tr \"text\" \"from\" \"to\" or tr filename \"from\" \"to\"" << std::endl;
        return;
    }

    std::string text;
    std::string from = removeQuotes(args[2]); // Remove quotes if present
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

    // Extract individual line numbers from the -nXYZ format
    std::string numberString = args[1].substr(2); // Skip "-n"
    std::set<int> targetLines;

    for (char c : numberString)
    {
        if (isdigit(c))
        {
            targetLines.insert(c - '0'); // Convert character to integer
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
        // Join arguments to reconstruct the quoted string
        std::string input;
        for (size_t i = 2; i < args.size(); ++i)
        {
            if (i > 2)
                input += " ";
            input += args[i];
        }

        // Remove surrounding quotes
        if (input.front() == '"' && input.back() == '"')
        {
            input = input.substr(1, input.size() - 2);
        }

        inputStream.str(input);
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
        inputStream << file.rdbuf(); // Read file content into stringstream
    }

    std::string line;
    int lineNumber = 1;
    while (std::getline(inputStream, line))
    {
        if (targetLines.count(lineNumber)) // Check if this line is in the requested set
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
            quotedArg = token; // Keep the quotes for now
        }
        else if (token.back() == '"' && inQuotes)
        {
            inQuotes = false;
            quotedArg += " " + token; // Keep the closing quote
            args.push_back(quotedArg);
        }
        else if (inQuotes)
        {
            quotedArg += " " + token; // Continue accumulating the quoted string
        }
        else
        {
            args.push_back(token); // Normal token
        }
    }

    if (args.empty())
        return;

    // Now, use the updated args vector
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
        wordCount(args, out); // Call wordCount with args as is
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
