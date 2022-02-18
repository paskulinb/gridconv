#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include <iostream>
//using namespace std;

#include "gridconv.h"
#include "gridconv.hpp"
#include "map.h"

GridConverter::GridConverter()
{
}



string GridConverter::getLastError()
{
	return this->last_error;
}


void GridConverter::help()
{
	cout <<
"GRIDCONVerter, vrsion 0.0.1 (alpha) November 2016,\n\
  by Borut Paskulin\n\
\n\
Converts GIS raster dataset to PNG image or custom binary file.\n\
The trasformation ruleset is given through a 'trasform string'.\n\
Accepted input raster formats: 1) ESRI float gird, 2) ESRI ASCII grid\n\
Supported output file formats: a) PNG image, b) custom binary file\n\
\n\
Usage:\n\
  gridconv <path_to_file> <output_format> <transformation_string>\n\
  gridconv --option\n\
Arguments:\n\
  path_to_file  File to open. Input format: (flt|txt)\n\n\
  output_format (png|png+pgw|int8|int16|int32|uint8|uint16|uint32|float32)\n\n\
  transformation_string String, describing ruleset. Put in single quotes.\n\n\
Options:\n\
  -h --help	This help\n";
}

// Creates a new Map instance and sets its transformation ruleset as defined in (string)'trasform' parameter.
//
// Parameter 'trasform' is string, containing rules, separated by '|' character.
// A single rule is comprised of input range/single-value and output range/single-value/math-expression, separated by ':' character.
// Range example 1: <input/output> "[100,200]" -> min_val=100, max_val=200
// Range example 2: <input/output> "[100,]"    -> min_val=100, max_val=DBL_MAX (maximum value of type double)
// Range example 3: <input/output> "[,200]"    -> min_val=DBL_MIN (minimum value of type double), max_val=200
// Single value example: <input/output> "[55]" -> min_val=max_val=55
// Math expression example: <output> "(CV/10)+5" -> every input value will be inserted in place of CV; the result will be output value
// 1. Split string to particular rule strings
// 2. Resolve praticular rule string to 'rule' struct
// 3. Create new TransformationValueToValue instance and return the pointer to it
Map* GridConverter::parseMapString(string map_str)
{
	char* trans = new char [map_str.length()+1];
	strcpy (trans, map_str.c_str());
//cout << "map_str:" << trans << endl;
	
	Map* map = new Map;
	
	//First rule:
	char* trans_save = NULL; //for strtok_r() need
	char* rule = strtok_r(trans, "|", &trans_save);

	while (rule != NULL)
	{
		//Resolve rule 
		char* rule_input = strtok(rule, ":");
		char* rule_output = strtok(NULL, "\0");
//cout << "rule:" << rule_input << "\t" << rule_output << endl;

		// INPUT RANGE
		// examples: [9] or [2,9] or [12,] or [,43]
		Range in = parseRangeString(rule_input);
//rule = strtok_r(NULL, "|", &trans_save); continue;		

		// OUTPUT RANGE/MATH_EXPRESSION/COLOR_RANGE
		// Is output defined as a rage ('[') or a math expression ('\"') or a color-range ('(')?
		char* brk = strpbrk(rule_output, "[\"(");
		
		if (brk == NULL) break; //can't find output method -> cancel this rule

		if (*brk == '[') { //it is output RANGE
			// range examples: [123.4,433.0] or [45] or [,121] or [-23.4,]
			Range out = parseRangeString(brk++);
			map->addMapper(new Mapper(in, out));
		}
		
		else if (*brk == '\"') { //it is output MATH_EXPRESSION
			// math expression example: "(CV-40)/100"
			char* fnString = strtok(brk, "\""); //find the end of math. expression string
			if (!fnString) break; //can't determine the end of math. expression string -> cancel this rule
			map->addMapper(new Mapper(in, (string)fnString));
		}
		
		else if (*brk == '(') { //it is output COLOR_RANGE
			// color range example: rgba(0,0,0,50)(80,0,255,50)
			ColorRange out = parseColorRangeString(brk++);
			map->addMapper(new Mapper(in, out));
		}
		
		//Next rule:
		rule = strtok_r(NULL, "|", &trans_save);
	}

	delete[] trans;
	return map;
}

// Parses string representing a range/sinlge-value
//
// Scan characters one by one expecting: NUMERIC DIGITS (-0123456789) following COMMA (,) following NUMERIC DIGITS (-0123456789)
// If COMMA is found BEFORE any NUMERIC DIGITS, the val1 will remain DBL_MIN.
// If NO COMMA is found, the val1 and val2 will get the value represented by first(the only) NUMERIC DIGITS string found (single value)
// If AFTER COMMA no numeric string is found, val2 will remain DBL_MAX
Range GridConverter::parseRangeString(string range_str)
{
	char * cstr = new char [range_str.length()+1];
	std::strcpy (cstr, range_str.c_str());
	char* p = cstr; //*p traverses over string
	
	double val1 = -DBL_MAX;
	double val2 = DBL_MAX;
	bool val1_found = false;
	bool comma_found = false;

	while (*p != '\0')
	{
		if (!comma_found && *p==',') {
			comma_found = true;
			p++;
			continue;
		}

		if (isdigit(*p)>0 || *p=='-') { //start of positive/negative numeric string
			
			if (!comma_found) {
				if (!val1_found) {
					val1 = atof(p); //value before comma
					val1_found = true;
				}
			} else {
				val2 = atof(p); //value after comma
				break;
			}
		}
		p++;
	}
	if (!comma_found) val2 = val1;
	
	Range out;
	out.val1 = val1;
	out.val2 = val2;
	return out;
}

ColorRange GridConverter::parseColorRangeString(string color_range_str)
{
	char* cr = new char [color_range_str.length()+1];
	strcpy (cr, color_range_str.c_str());
	
	//COLORS 'RANGE'
	/* Example: rgb(0,255,0) or rgba(0,255,0,80) or rgb(0,255,0)(80,80,80) or rgba(0,255,0,128)(80,80,80,128)
	 * 1. find first bracket '()'
	 * 2. take comma separated values (3 for rgb or 4 for rgba)
	 * 3. check for possible second bracket '()' and teake rgb/rgba values
	 */
//		cout << "colors: " << string(rule_colors) << endl;
	char* p = cr;
	char num[] = "0123456789";

	char* color1 = strpbrk(cr, num);
	char* color2 = strpbrk(color1, "("); //find start of second color if present
	if (color2) color2 = strpbrk(color2, num); //move to first numeric character
	strtok(color1, ")"); //terminate string by '\0'
	strtok(color2, ")"); //terminate string by '\0'

	//Split to r-g-b-a
	char* val;
	Color c1;
	Color c2;
	
	if (color1) {
		if ((val = strtok(color1, ","))) c1.r = atoi(val);
		if ((val = strtok(NULL, ","))) c1.g = atoi(val);
		if ((val = strtok(NULL, ","))) c1.b = atoi(val);
		if ((val = strtok(NULL, ","))) c1.a = atoi(val);
//			printf("color1: %d %d %d %d\n", c1.r, c1.g, c1.b, c1.a);

	}
	
	if (color2) {
		if ((val = strtok(color2, ","))) c2.r = atoi(val);
		if ((val = strtok(NULL, ","))) c2.g = atoi(val);
		if ((val = strtok(NULL, ","))) c2.b = atoi(val);
		if ((val = strtok(NULL, ","))) c2.a = atoi(val);
//			printf("color2: %d %d %d %d\n", c2.r, c2.g, c2.b, c2.a);
	}
	else {
		c2 = c1;
	}

	//cout << "colors: " << c1 << c2 << endl;

	ColorRange color_range(c1, c2);
	return color_range;
}

int GridConverter::createPngFile(Grid& grid, Map& map, std::string out_filepath)
{

	gdImagePtr im = gdImageCreate(grid.ncols, grid.nrows); //color pallete image

	gdImageColorAllocateAlpha(im, 0, 0, 0, 127); //First color index in pallete represents background color in gd2lib. (alpha=127 -> fully transparent)

	/*for (int c=0; c<32; c++) {                                                                                                                                      
		gdImageColorAllocate (im, c, c, c);
	}*/

	for (unsigned int row = 0; row < grid.nrows; row++) {
		for (unsigned int col = 0; col < grid.ncols; col++) {
			double val = grid.getCellValue(row*grid.ncols + col);
			Color cp = map.getColor(val);
//			cout << val << "\t" << cp << endl;
			gdImageSetPixel( im, col, row, gdImageColorResolveAlpha(im, cp.r, cp.g, cp.b, cp.a) );
		}
	}

	FILE* fh_png;
	if (!(fh_png = fopen(out_filepath.c_str(), "w"))) {
		cout << "GridConverter::createPngFile: Error creating .png file" << endl;
		return 0;
	}
	gdImagePng(im, fh_png);
	/* ... Use the image ... */
	
	gdImageDestroy(im);

	return 1;
}


int GridConverter::createWorldFile(Grid& grid, string out_filepath)
{
	FILE* fh_wf;
	if (!(fh_wf = fopen(out_filepath.c_str(), "w"))) {
		cout << "GridConverter::createWorldFile: Error creating world file" << endl;
		fclose (fh_wf);
		return 0;
	}
	
	fprintf (fh_wf, "%f\n", grid.cellsize); //pixel size X direction
	fputs ("0.00000000000000\n", fh_wf); //rotation
	fputs ("0.00000000000000\n", fh_wf); //rotation
	fprintf (fh_wf, "-%f\n", grid.cellsize); //pixel size Y direction
	fprintf (fh_wf, "%f\n", grid.xllcorner + grid.cellsize/2); //X map coordinates of the center of the upper-left pixel
	fprintf (fh_wf, "%f", grid.yllcorner + (double)grid.nrows*grid.cellsize - grid.cellsize/2); //Y map coordinates of the center of the upper-left pixel

	fclose (fh_wf);

	return 1;
}
