#include "command.hpp"

int main(int argc, char** argv) {
    CommandExecutor* cmd = new CommandExecutor();
    cmd->exec();
}
