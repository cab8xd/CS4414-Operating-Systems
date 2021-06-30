/*
 * cab8xd
*/

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>


// instantiates empty vector for tokens.
std::vector<std::string> tokens;

struct CommandBlock{
  std::string command, /* the command ("/bin/ls")*/
              inputf,  /* opt.  input file */
              outputf; /* opt. output file */
  pid_t pidc = 0;
  std::vector<std::string> arguments; // vector list of arguments
  std::vector<char*> argumentsc; // for execv
};

void printBlock(CommandBlock block){
 std::cout << "--Printing block--- "
           << std::endl
           << "command- "
           << block.command
           << std::endl
           << "  inputf- "
           << block.inputf
           << std::endl
           << "  outputf- "
           << block.outputf
           << std::endl
           << "Arguments: ";
  for(size_t i = 0; i < block.arguments.size(); i++) {
   std::cout << block.arguments.at(i) << ", ";
 }
  std::cout << std::endl;
}

void printBlocks( std::vector<CommandBlock> blocks){
 for(size_t i = 0; i < blocks.size(); i++) {
   printBlock(blocks.at(i));
 }
}


// Helper function for printing tokens.
void printV(void) {
 for(size_t i = 0; i < tokens.size(); i++) {
   std::cout << tokens.at(i) << ", ";
 }
  std::cout << std::endl;
}

void parse_and_run_command(const std::string &command) {
    // instantiates empy vector for blocks.
    std::vector<CommandBlock> blocks;
    // checks if the length of command is less than 100. Q: Should this measure the command or total tokens?
    if(command.length() > 100){
       std::cerr << "invalid command" << std::endl;
       return;
     }

    // parsing commands
     std::istringstream s(command);
     std::string token;
     while( s >> token ) {
        tokens.push_back(token);
     }

     // sort tokens into arguments for command blocks.
     CommandBlock block;
     bool first = true;
     bool valid = true; 
     for(size_t i = 0; i < tokens.size(); i++) {

        std::string t = tokens.at(i);


       if(t == "|"){ // See a pipe, make a new struct for the vector.
          if(i == tokens.size()-1  
          || tokens.at(i + 1) == "|"
          || i == 0) 
          { 
            if(valid){std::cerr <<"invalid command"; valid = false;}
            }
          else{
          // if first is true: bad
          if (first) {
            std::cerr << "invalid command\n";
            return;
          }
          CommandBlock newBlock;
          blocks.push_back(block);
          block = newBlock;
          first = true;
         }
       }

     else if(t == "<"){
         if(i == tokens.size()-1 
          || tokens.at(i + 1) == ">" 
          || tokens.at(i + 1) == "<" 
          || tokens.at(i + 1) == "|") 
         {
            if(valid){std::cerr <<"invalid command"; valid = false;}
           } 
         else{
          i++;
          t = tokens.at(i);
          block.inputf = t;
         }
       }
     else if(t == ">"){
         if(i == tokens.size()-1 
          || tokens.at(i + 1) == ">" 
          || tokens.at(i + 1) == "<" 
          || tokens.at(i + 1) == "|") 
         {
           if(valid){std::cerr <<"invalid command"; valid = false;}
         } 
         else{
          i++;
          t = tokens.at(i);
          block.outputf = t;
         }
       }
       else{
         if(first){ block.command = t; first = false; }
         block.arguments.push_back(t);
         }
      }
      // if first is true: bad
      if (first) {
        std::cerr << "invalid command\n";
        return;
      }
      // if block.command isn't set, bad
      blocks.push_back(block);

     // converting blocks for execution. 
      for(size_t i = 0; i < blocks.size(); i++){
          for(size_t j = 0; j < blocks.at(i).arguments.size(); j++){
           blocks.at(i).argumentsc.push_back( const_cast<char*>( blocks.at(i).arguments.at(j).c_str() ) );
          }
        blocks.at(i).argumentsc.push_back(NULL);
      }




    int status = 0;

   
    int read_fdl = -1;
    // for every block in blocks
  if(valid){
     int read_fd = -1;
     int write_fd = -1;
    for(size_t i = 0; i < blocks.size(); i++){
     int pipe_fd[2] = {-1,-1};
   

      //bool isPipe = false;
      if(i > 0) read_fdl = read_fd;
      
       // if an error occurs. UNSURE
       if(blocks.at(i).command == "exit") {
        exit(0);
       }

     if( i < blocks.size() - 1){
      if(pipe(pipe_fd) < 0) {std::cerr << "pipe error";}
       read_fd = pipe_fd[0]; // 0 like std in.
       write_fd = pipe_fd[1];
       //isPipe = true;
      }

      pid_t pid = fork();
     if(pid < 0) { std::cerr << "fork error";}

     // Child process.
     if(pid == 0){
       // If not the first command, input rediretion for pipe.
       if(i > 0) {
         dup2(read_fdl, 0);
         close(read_fdl);
        }
       // If not the last command, output redirection for pipe.
       if( i != blocks.size() - 1){
         close(read_fd);
         dup2(write_fd, 1);
         close(write_fd);
       }
       // Output rediect (>)
       if(blocks.at(i).outputf != "") {
         int fd = open(blocks.at(i).outputf.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
         if(fd == -1) std::cerr << "open error\n";

         // close the write end of the pipe if it exists. 
         // if(isPipe) close(write_fd);

         dup2(fd, 1); // to std out
         close(fd);
        }
       // Input redirect (<)
      if(blocks.at(i).inputf != "") {
         // std::cerr << blocks.at(i).inputf << '\n';
         int fd = open(blocks.at(i).inputf.c_str(), O_RDWR); // What kind of argument?
         if(fd == -1) std::cerr << "open error\n";

         // close the read end of the pipe if it exists
         // if(isPipe) close(read_fd);

         dup2(fd, 0); // to std in
         close(fd);
        }
        execv( blocks.at(i).command.c_str(), blocks.at(i).argumentsc.data() );
         //   }
       std::cerr << "Command not found\n.";
     }

     else { // the parent
           blocks.at(i).pidc = pid;
           if(write_fd > -1) close(write_fd);
           // closing write and the LAST read -- need a variable to store the file descriptor of the last read
           if(read_fdl > -1) close(read_fdl);
           }
   }
  }

// waiting for all processes to finish lmaooo
  // for every block, wait for its child pid. 
  for(size_t i = 0; i < blocks.size(); i++){
   if(blocks.at(i).pidc > 0){
     waitpid(blocks.at(i).pidc, &status, 0); // confused about arguments and &status in the pseudocode...
     int es = WEXITSTATUS(status);
     std::cout<< blocks.at(i).arguments[0] << " exit status: " << es << "\n"; 
   }
  }
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
