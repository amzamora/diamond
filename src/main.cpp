#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage:\n");
		printf("    diamond source_file_path\n");
		exit(EXIT_FAILURE);
	}

	// Read file
	std::ifstream in;
	in.open(argv[1]);
	std::stringstream stream;
	stream << in.rdbuf();
	std::string source_file = stream.str();

	std::cout << source_file;

	return 0;
}