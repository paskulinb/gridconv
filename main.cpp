#include <stdio.h>
#include <unistd.h> //cwd
#include <dirent.h> //dir
#include <string.h>


#include <iostream>
using namespace std;

#include "gridconv.h"
#include "gridconv.hpp"

int main(int argc, char** argv)
{
	/* get Curren Workign Diretory */
	char cwd[500];
	getcwd(cwd, 500);
	
	/* Check input ARGUMENTS */
	if (argc >= 2) {
		
		if ((strcmp("--help", argv[1]) == 0) || (strcmp("-h", argv[1]) == 0)) {
			
			GridConverter::help();
			return 0;
		}
		
		/* argv[1] should be INPUT FILE PATH
		 * argv[2] should be OUTPUT FILE TYPE (png, png+pgw, int8)
		 * argv[3] should be TRANSFORMATION string
		 */
		string infile_path = string(argv[1]);
		string outfile_type = (argv[2]) ? string(argv[2]) : string("");
		string transform = (argv[3]) ? string(argv[3]) : string("");

		cout << "Input File Path: " << infile_path << endl;
		cout << "Output File Type: " << outfile_type << endl;
//return 1;

		Grid grid;
		GridConverter gc;
		
		if (!grid.openGrid(string(cwd) + "/" + infile_path)) {
			cout << "failed: " << grid.getLastError() << endl;
			return 1;
		}
		
		if (outfile_type == "png") {
			Map* map = gc.parseMapString(transform);
			gc.createPngFile(grid, *map);
			delete map;
			return 0;
		}
		
		if (outfile_type == "png+pgw") {
			Map* map = gc.parseMapString(transform);
			gc.createPngFile(grid, *map);
			gc.createWorldFile(grid, "pgw");
			delete map;
			return 0;
		}
		
		if (outfile_type == "int8") {
			Map* map = gc.parseMapString(transform);
			gc.createBinFile<int8_t>(grid, *map, "int8");
			delete map;
			return 0;
		}
		
		if (outfile_type == "int16") {
			Map* map = gc.parseMapString(transform);
			gc.createBinFile<int16_t>(grid, *map, "int16");
			delete map;
			return 0;
		}
		
		if (outfile_type == "int32") {
			Map* map = gc.parseMapString(transform);
			gc.createBinFile<int32_t>(grid, *map, "int32");
			delete map;
			return 0;
		}
		
		if (outfile_type == "uint8") {
			Map* map = gc.parseMapString(transform);
			gc.createBinFile<unsigned int8_t>(grid, *map, "uint8");
			delete map;
			return 0;
		}

		if (outfile_type == "uint16") {
			Map* map = gc.parseMapString(transform);
			gc.createBinFile<unsigned int16_t>(grid, *map, "uint16");
			delete map;
			return 0;
		}

		if (outfile_type == "uint32") {
			Map* map = gc.parseMapString(transform);
			gc.createBinFile<unsigned int32_t>(grid, *map, "uint32");
			delete map;
			return 0;
		}

		if (outfile_type == "float32") {
			Map* map = gc.parseMapString(transform);
			gc.createBinFile<float>(grid, *map, "float32");
			delete map;
			return 0;
		}
	}

	return 1;
}

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
   
    /*Replace popen and pclose with _popen and _pclose for Windows.*/
}
