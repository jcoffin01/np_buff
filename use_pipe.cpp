#include "np_buf.hpp"
#include <vector>
#include <string>

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);

    try {
		char const *pipe_name = R"(\\.\pipe\test)";

		Mode mode = args[1] == "server" ? Mode::Server : Mode::Client;

		np_stream pipe(pipe_name, mode);

		std::cerr << "Created stream\n";

		if (mode == Mode::Server) {
			std::string s;
			std::getline(pipe, s);
            std::cout << "Server received \"" << s << "\"\n";
			std::cout << "Writing to pipe\n";
            pipe << "echo: " << s << std::flush;

            std::cerr << "Write completed\n";
		} else {
			std::cerr << "writing to pipe\n";
			pipe << "Testing...\n" << std::flush;

			std::string s;

			std::cerr << "Reading from pipe\n";
			std::getline(pipe, s);
			std::cout << "Client received: \"" << s << "\"\n";
		}
	}
	catch(std::exception const &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
