#ifndef GRID
#define GRID

#include <string>

/** Class representing a single GIS raster file.
 *  It supports opening 'ESRI ASCII grid' and 'ESRI float grid' file formats.
 */
class Grid
{
public:
	static const int BYTEORDER_UNDEFINED;
	static const int BYTEORDER_LSBFIRST;
	static const int BYTEORDER_MSBFIRST;
	
private:
	FILE* fp;
	std::string in_file;
	int in_filetype;
public:
	std::string in_file_dir;
	std::string in_file_basename;
private:
	std::string last_error;
	
	int arch_byteorder;

public:	
	unsigned int ncols; /**< header: number of cells in a row */
	unsigned int nrows; /**< header: number of rows */
	double xllcorner;    /**< header: longitude of lower left corner of the raster */
	double yllcorner;    /**< header: latitude of lower left corner of the raster */
	double cellsize;     /**< header: size of a single raster cell (in the same units as xllcorner and yllcorner) */
	float NODATA_value;  /**< header: some cells in GIS may have undefined values. This is represented by a specific value, meaning NODATA. */
	int byteorder;       /**< header: the byteorder of 32-bit floating point values, stored in source file */ //FLT format only     
	float* cells;        /**< pointer to memory containing cells values */

public:
	/** Constructor */
	Grid();
	/** Destructor */
	~Grid();
	/** Get the value of a specific raster cell.
	 *  @param index cell index [0 .. ncols*nrows-1]
	 *  @return cell value
	 */
	float getCellValue(int index);
	/** Get textual error description of last file openning.
	 *  @return Last error description
	 */
	std::string getLastError();

private:
	/** Find the size of the file to be oppened.
	 *  How much memory do I need to allocate to store all raster cell values (ncols*nrows)?
	 *  @return File size in bytes
	 */
	int getFileSize(FILE* fp);

public:
	/** Open a raster data file.
	 *  Two GIS raster fileformats are supported: ESRI ASCII grid and ESRI float grid.
	 *  The fileformat can be specified by file extension in 'filepath' parameter.
	 *  ASCII grid has .txt extension, Float grid has .flt extensiton.
	 *  If no extension is defined, the ASCII grid presence will be checked first.
	 *  If not found, then Float file will be opened if existent.
	 *  @param filepath path to file
	 *  @return 1: OK <br>0: ERROR
	 */
	int openGrid(std::string filepath);
	
	/** ESRI ASCII grid file format parsing procedure.
	 * 
	 *  @return 1: OK <br>0: ERROR
	 */
	int readAsciiGrid();
	
	/** ESRI FLOAT grid file format parsing procedure.
	 * 
	 *  @return 1: OK <br>0: ERROR
	 */
	int readFloatGrid();

private:
	/** Grid header parsing procedure.
	 *  The function parses the header (text).
	 *  In ASCII grid format the header is in the same file as cells data.
	 *  In Float grid format the header is in separate file wiht .hdr extension
	 *  @param buff pointer to string, containing the header of the raster data.
	 *  @return Returns the pointer to start of cells data in ASCII grid file. At Folat grid fileformat this is not needed.
	 */
	char* readHeader(char* buff);
public:
	/** Swaps 4 bytes.
	 *  Needed for compatibility between differen micropricessor architectures (MSB/LSB).
	 *  Source raster file may be created on a computer of different architecture the machine, where the file is beeing read.
	 *  @param val input 32-bit floating point numeric value
	 *  @return 32-bit floating point value (swapped bytes)
	 */
	static float swap4Bytes(float val);
	
	/** Detects the byte order of the arhitecture runnig this code/program.
	 *  @return BYTEORDER_LSBFIRST or BYTEORDER_MSBFIRST
	 *  @see swap4Bytes()
	 */
	static int detectArchByteOrder();
};
#endif
