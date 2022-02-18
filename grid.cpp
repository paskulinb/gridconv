#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*** For getFileSize(): ***/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/**************************/

#include <iostream>
//using namespace std;
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "grid.h"

const int Grid::BYTEORDER_UNDEFINED = 0;
const int Grid::BYTEORDER_LSBFIRST = 1;
const int Grid::BYTEORDER_MSBFIRST = 2;

Grid::Grid()
{
	this->byteorder = 0;
	this->cells = nullptr;
	this->arch_byteorder = Grid::detectArchByteOrder();
}

Grid::~Grid()
{
	if (this->cells != nullptr) delete [] this->cells;
}

int Grid::getFileSize(FILE* fp)
{
	struct stat sb;
	fstat(fileno(fp), &sb);
	
	return sb.st_size;
}


int Grid::openGrid(std::string filepath)
{

    auto path = fs::path(filepath);
    auto ext = path.extension();

	if (ext == "") { //user DID NOT DEFINE the file extension

        if (fs::exists(filepath + ".txt")) {
			return this->readAsciiGrid(filepath + ".txt");
        }
        else if (fs::exists(filepath + ".flt")) {
			return this->readFloatGrid(filepath + ".flt");
        }
	}
    else if (ext == ".txt") {                            //.txt extension -> AsciiGrid format
        return this->readAsciiGrid(filepath);
    }
    else if (ext == ".flt") {                            //.flt extension -> FloatGrid format
        return this->readFloatGrid(filepath);
    }
	
	this->last_error = std::string("File '") + filepath + std::string("' not found.");
	
	return 0;
}


char* Grid::parseHeader(char* buff)
{
	char* tok = strtok(buff, " ");

	this->byteorder = Grid::BYTEORDER_UNDEFINED;

	while (true){
		if (strcmp(tok, "ncols") == 0)             this->ncols        = atoi(strtok(nullptr, " \n\r"));
		else if (strcmp(tok, "nrows") == 0)        this->nrows        = atoi(strtok(nullptr, " \n\r"));
		else if (strcmp(tok, "xllcorner") == 0)    this->xllcorner    = atof(strtok(nullptr, " \n\r"));
		else if (strcmp(tok, "yllcorner") == 0)    this->yllcorner    = atof(strtok(nullptr, " \n\r"));
		else if (strcmp(tok, "cellsize") == 0)     this->cellsize     = atof(strtok(nullptr, " \n\r"));
		else if (strcmp(tok, "NODATA_value") == 0) this->NODATA_value = atof(strtok(nullptr, " \n\r"));
		else if (strcmp(tok, "byteorder") == 0)    this->byteorder    = (strcmp(strtok(nullptr, " \n\r"), "LSBFIRST") == 0) ? Grid::BYTEORDER_LSBFIRST : Grid::BYTEORDER_MSBFIRST;
		else break; //finished
		
		tok = strtok(nullptr, " \n\r"); //take next parameter
		if (tok == nullptr) break;     //end of file? -> exit from loop
	}

	/*std::cout << "ncols=" << this->ncols << std::endl;
	std::cout << "nrows=" << this->nrows << std::endl;
	std::cout << "xllcorner=" << this->xllcorner << std::endl;
	std::cout << "yllcorner=" << this->yllcorner << std::endl;
	std::cout << "cellsize=" << this->cellsize << std::endl;
	std::cout << "NODATA_value=" << this->NODATA_value << std::endl;
	std::cout << "byteorder=" << this->byteorder << std::endl;
    */

	return tok; //return FIRST CELL DATA token (if ASCII GRID file)
}


int Grid::readAsciiGrid(std::string path)
{
    FILE* fp;
    if (nullptr == (fp = fopen(path.c_str(), "r"))) return 0;

	unsigned int file_size = this->getFileSize(fp);
	
	char* buff = new char[file_size + 1];

	if (fread(buff, file_size, 1, fp) != 1) {    //error reading?
		this->last_error = "readAsciiGrid: File reading error";
        fclose(fp);
		return 0;
	}
    
    fclose(fp);

	buff[file_size] = '\0'; //ensure nullptr terminated string

//std::cout << buff << std::endl; //return 0;

	//parse HEADER
	char* cell_value = this->parseHeader(buff); //first cell's value
	
	//create cells array
	unsigned int nc = this->ncols * this->nrows;
	
	if (this->cells != nullptr) delete [] this->cells; //delete old if exists
	this->cells = new float[nc];                    //create new
	
	//populate cells with values
	float* ci = this->cells;
	
	*ci = atof(cell_value);                          //first cell
	while ( (cell_value = strtok(nullptr, " \n\r")) ){  //rest of cells
		*(++ci) = atof(cell_value);
	}
//std::cout << "cv:" << cell_value << std::endl; //return 0;
	
	delete [] buff;

	//return 0;
	return 1;
}


int Grid::readFloatGrid(std::string path)
{
	/* *** read HEADER file *** */
	FILE* hdr;

	//if (!(hdr = fopen((this->in_file_dir + "/" + this->in_file_basename + ".hdr").c_str(), "r"))) {
	if (!(hdr = fopen(fs::path(path).replace_extension(".hdr").c_str(), "r"))) {
		this->last_error = "readFloatGrid: Error opening .hdr file";
		return 0;
	}

	unsigned int hdr_file_size = this->getFileSize(hdr);
	char* hdr_buff = new char[hdr_file_size + 1];
	
	if (fread(hdr_buff, hdr_file_size, 1, hdr) != 1) {
		this->last_error = "readFloatGrid: Error reading .hdr file";
		delete [] hdr_buff;
        fclose(hdr);
		return 0;
	}

	hdr_buff[hdr_file_size] = '\0';
	
	fclose(hdr);
	this->parseHeader(hdr_buff);

	delete [] hdr_buff;
	/* --- END reading HEADER file --- */

	/* *** Populate cells with values *** */
    FILE* flt;

	//if (!(flt = fopen((this->in_file_dir + "/" + this->in_file_basename + ".flt").c_str(), "r"))) { //open .flt file
	if (!(flt = fopen(fs::path(path).replace_extension(".flt").c_str(), "r"))) { //open .flt file
		this->last_error = "readFloatGrid: Error opening .flt file";
		return 0;
	}

	//read DATA file (cells values)
	unsigned int file_size = this->getFileSize(flt);

	//create cells array
	unsigned int nc = this->ncols * this->nrows;
	
	if (this->cells != nullptr) delete [] this->cells; //delete old if exists
	this->cells = new float[nc];                    //create new
	
	//read values to cells
	if (fread(this->cells, file_size, 1, flt) != 1) {
		this->last_error = "readFloatGrid: Error reading .flt file";
		return 0; //error reading
        fclose(flt);
	}
    fclose(flt);

	return 1;
}


float Grid::swap4Bytes(float val)
{
   float retVal;
   char *floatToConvert = ( char* ) &val;
   char *returnFloat = ( char* ) &retVal;

   // swap the bytes into a temporary buffer
   returnFloat[0] = floatToConvert[3];
   returnFloat[1] = floatToConvert[2];
   returnFloat[2] = floatToConvert[1];
   returnFloat[3] = floatToConvert[0];

   return retVal;
}


int Grid::detectArchByteOrder()
{
	int num = 1;
	return (*(char *) &num == 1) ? Grid::BYTEORDER_LSBFIRST : Grid::BYTEORDER_MSBFIRST;
}

float Grid::getCellValue(int index)
{
	return (((this->byteorder==Grid::BYTEORDER_UNDEFINED) | (this->byteorder==this->arch_byteorder))) ? 
	        *(this->cells+index) : 
	        Grid::swap4Bytes(*(this->cells+index));
}

std::string Grid::getLastError()
{
	return this->last_error;
}
