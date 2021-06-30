#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <list>

// instantiates empty vector for tokens.
std::vector<std::string> tokens;

struct CommandBlock{
  std::string command, /* the command ("/bin/ls")*/
              inputf,  /* opt.  input file */
              outputf; /* opt. output file */
  std::vector<std::string> arguments; // vector list of arguments
  
}

// Helper function for printing tokens.
//void printV(void) {
// for(size_t i = 0; i < tokens.size(); i++) {
 // std::cout << tokens.at(i) << ", ";
 // }
 // std::cout << std::endl;
//}

void parse_and_run_command(const std::string &command) {
    /* TODO: Implement this. */
    // checks if the length of command is less than 100. Q: Should this measure the command or total tokens?
    if(command.length() > 100){
       std::cout << "error: too many characters" << std::endl; // new line chara.
       exit(0);
     }

      printV();


     // Implement and test running commands without pipelines or redirection where the list of tokens
     // is the exact number of arguments needed. 

    // Add support for redirection.

    // Add support for pipelines.

    /* Note that this is not the correct way to test for the exit command.
       For example the command "   exit  " should also exit your shell.
     */
    if (command == "exit") {
        exit(0);
    }
    std::cerr << "Not implemented.\n";
   
  // for testing use
  tokens.clear();
}

int main(void) {
    std::string command;
    std::cout << "> ";
    while (std::getline(std::cin, command)) {
        parse_and_run_command(command);
        std::cout << "> ";
    }
    return 0;
}
