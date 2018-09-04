#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*** For getFileSize(): ***/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/**************************/

//#include <iostream>
//using namespace std;

#include "grid.h"

const int Grid::BYTEORDER_UNDEFINED = 0;
const int Grid::BYTEORDER_LSBFIRST = 1;
const int Grid::BYTEORDER_MSBFIRST = 2;

Grid::Grid()
{
	this->byteorder = 0;
	this->cells = NULL;
	this->arch_byteorder = Grid::detectArchByteOrder();
}

Grid::~Grid()
{
	if (this->cells != NULL) delete [] this->cells;
}

int Grid::getFileSize(FILE* fp)
{
	struct stat sb;
	fstat(fileno(fp), &sb);
	
	return sb.st_size;
}

char* Grid::readHeader(char* buff)
{
	char* tok = strtok(buff, " ");

	this->byteorder = Grid::BYTEORDER_UNDEFINED;

	while (true){
		if (strcmp(tok, "ncols") == 0)             this->ncols        = atoi(strtok(NULL, " \n\r"));
		else if (strcmp(tok, "nrows") == 0)        this->nrows        = atoi(strtok(NULL, " \n\r"));
		else if (strcmp(tok, "xllcorner") == 0)    this->xllcorner    = atof(strtok(NULL, " \n\r"));
		else if (strcmp(tok, "yllcorner") == 0)    this->yllcorner    = atof(strtok(NULL, " \n\r"));
		else if (strcmp(tok, "cellsize") == 0)     this->cellsize     = atof(strtok(NULL, " \n\r"));
		else if (strcmp(tok, "NODATA_value") == 0) this->NODATA_value = atof(strtok(NULL, " \n\r"));
		else if (strcmp(tok, "byteorder") == 0)    this->byteorder    = (strcmp(strtok(NULL, " \n\r"), "LSBFIRST") == 0) ? Grid::BYTEORDER_LSBFIRST : Grid::BYTEORDER_MSBFIRST;
		else break; //finished
		
		tok = strtok(NULL, " \n\r"); //take next parameter
		if (tok == NULL) break;     //end of file? -> exit from loop
	}

/*	cout << "ncols=" << this->ncols << endl;
	cout << "nrows=" << this->nrows << endl;
	cout << "xllcorner=" << this->xllcorner << endl;
	cout << "yllcorner=" << this->yllcorner << endl;
	cout << "cellsize=" << this->cellsize << endl;
	cout << "NODATA_value=" << this->NODATA_value << endl;
	cout << "byteorder=" << this->byteorder << endl;
*/
	return tok; //return FIRST CELL DATA token
}


int Grid::readAsciiGrid()
{
	unsigned int file_size = this->getFileSize(this->fp);
	
	char* buff = new char[file_size + 1];

	if (fread(buff, file_size, 1, this->fp) != 1) {    //error reading?
		this->last_error = "readAsciiGrid: File reading error";
		return 0;
	}

	buff[file_size] = '\0'; //ensure null terminated string

	//parse HEADER
	char* cell_value = this->readHeader(buff); //first cell's value
	
	//create cells array
	unsigned int nc = this->ncols * this->nrows;
	
	if (this->cells != NULL) delete [] this->cells; //delete old if exists
	this->cells = new float[nc];                    //create new
	
	//populate cells with values
	float* ci = this->cells;
	
	*ci = atof(cell_value);                          //first cell
	while ( (cell_value = strtok(NULL, " \n\r")) ){  //rest of cells
		*(++ci) = atof(cell_value);
	}
	
	delete [] buff;

	return 1;
}


int Grid::readFloatGrid()
{
	/* *** read HEADER file *** */
	FILE* hdr;

	if (!(hdr = fopen((this->in_file_dir + "/" + this->in_file_basename + ".hdr").c_str(), "r"))) {
		this->last_error = "readFloatGrid: Error opening .hdr file";
		return 0;
	}

	unsigned int hdr_file_size = this->getFileSize(hdr);
	char* hdr_buff = new char[hdr_file_size + 1];
	
	if (fread(hdr_buff, hdr_file_size, 1, hdr) != 1) {
		this->last_error = "readFloatGrid: Error reading .hdr file";
		delete [] hdr_buff;
		return 0;
	}

	hdr_buff[hdr_file_size] = '\0';
	
	fclose(hdr);
	this->readHeader(hdr_buff);

	delete [] hdr_buff;
	/* --- END reading HEADER file --- */

	/* *** Populate cells with values *** */
	if (!(this->fp = fopen((this->in_file_dir + "/" + this->in_file_basename + ".flt").c_str(), "r"))) { //open .flt file
		this->last_error = "readFloatGrid: Error opening .flt file";
		return 0;
	}

	//read DATA file (cells values)
	unsigned int file_size = this->getFileSize(this->fp);

	//create cells array
	unsigned int nc = this->ncols * this->nrows;
	
	if (this->cells != NULL) delete [] this->cells; //delete old if exists
	this->cells = new float[nc];                    //create new
	
	//read values to cells
	if (fread(this->cells, file_size, 1, this->fp) != 1) {
		this->last_error = "readFloatGrid: Error reading .flt file";
		return 0; //error reading
	}

	return 1;
}


int Grid::openGrid(std::string filepath)
{
	unsigned int dot_pos = filepath.find_last_of('.');
	int last_slash_pos = filepath.find_last_of('/');
	
	this->in_file_dir = filepath.substr(0, last_slash_pos);
	this->in_file_basename  = filepath.substr(last_slash_pos + 1, dot_pos - last_slash_pos - 1);
	
	if (dot_pos == std::string::npos) { //user DID NOT DEFINE the file extension

		if ( (this->fp = fopen((filepath + ".txt").c_str(), "r")) ){       //try if .txt file exists
			return this->readAsciiGrid();
		}
		else if ( (this->fp = fopen((filepath + ".flt").c_str(), "r")) ){  //try if .flt file exists
			return this->readFloatGrid();
		}

	} else {
		
		std::string ext = filepath.substr(dot_pos);                          //user DEFINED the file extension
		if (ext.compare(".txt") == 0){                                  //.txt extension -> AsciiGrid format
			if ((this->fp = fopen(filepath.c_str(), "r")))
				return this->readAsciiGrid();
		}
		else if (ext.compare(".flt") == 0){                            //.flt extension -> FloatGrid format
			if ((this->fp = fopen(filepath.c_str(), "r"))){
				return this->readFloatGrid();
			}
		}
	}
	
	this->last_error = std::string("File '") + filepath.c_str() + std::string("' not found.");
	
	return 0;
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
