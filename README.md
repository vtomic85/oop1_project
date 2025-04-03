# OOP1 Project
## Pre-requirements
You will need a G++ compiler. Download it from https://winlibs.com/#download
When you unpack the archive, locate the **bin** folder, and add it to the **PATH** in your system environment variables. Restart the computer so the changes take effect.
Check if the compiler has been installed successfully by opening a console and running `g++ --version`

## Running the application
Open the console, navigate to the folder where `main.cpp` is located, and run `g++ -std=c++17 ".\main.cpp" -o command_prompt.exe`
This will run the compiler, use version 17 of C++ (since some of the used functions work only from that version up), compile `main.cpp` and generate an executable file `command_prompt.exe`.
Run your application by typing `.\command_prompt.exe`

## Making changes
When you make changes in the code, remember to save your file and to run the compiler again in order to re-generate the .exe file
