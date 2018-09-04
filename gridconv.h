#ifndef GRIDCONV
#define GRIDCONV

#include <string>
#include "gd.h"  //gd2 graphic library
#include "grid.h"
#include "map.h"


/** Create Binary and Image+World (PNG+PGW) files from ESRI Grid files according to trasformation, defined by Map.
 *  Binary file types can be defined by user. (The createBinFile() method is defined as template function.)
 */
class GridConverter
{
private:
	std::string last_error;

public:
	/** Constructor. */
	GridConverter();

	/** Creates a binary file where each trasformed GRID cell value is represented by a numeric value.
	 *  The type of numeric  value is defined by template <T> type.
	 *  Each GRID cell value is calculated by Map instance.
	 *  The output file is created in same directory as Grid source file is.
	 *  This method is defined in gridconv.hpp file.
	 *  @param [in] grid Reference to Grid instance
	 *  @param [in] map Reference to Map instance
	 *  @param file_extension Optional file extension for created file
	 *  @return
	 *	- 1 if successfull
	 *	- 0 on error
	 */
	template<typename T> int createBinFile(Grid& grid, Map& map, string file_extension = "bin"); //defined in "gridconv.hpp"

	/** Creates a image file where each trasformed GRID cell value is represented by a corresponding color.
	 *  Each GRID cell value is converted by Map instance to corresponding Color.
	 *  The output file is created in same directory as Grid source file is.
	 *  @param [in] grid Reference to Grid instance
	 *  @param [in] map Reference to Map instance
	 *  @return
	 *	- 1 if successfull
	 *	- 0 on error
	 *  @see createWorldFile()
	 */
	int createPngFile(Grid& grid, Map& map);

	/** Creates a world file belonging to image file.
	 *  @param [in] grid Reference to Grid instance
	 *  @param file_extension File extension for created file ("pgw" for png image)
	 *  @return
	 *	- 1 if successfull
	 *	- 0 on error
	 *  @see createPngFile()
	 */
	int createWorldFile(Grid& grid, string file_extension);

	/** Get textual description of last error.
	 *  @return description of the last error.
	 */
	string getLastError();

	/** Print command-line style help to stdout.
	 */
	static void help();

	/** Parses as textual representation(configuration) of Map instance.
	 *  @param map_str Textual representation(configuration) of Map instance
	 *  @return Map instance.
	 */
	Map* parseMapString(string map_str);

private:

	Range parseRangeString(string range_str);

	ColorRange parseColorRangeString(string color_range_str);
};
#endif
