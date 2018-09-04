#ifndef MAP
#define MAP

#include <string>
#include <vector>
#include <cfloat>    //DBL_MIN,DBL_MAX

#include <iostream>
using namespace std;

#include "fastmathparser/exprtk.hpp"
typedef exprtk::parser<double> parser_t;


/** Class representing a range of two values. */
class Range {
public:
	/** Constructor */
	Range(double val1 = DBL_MIN, double val2 = DBL_MAX);
	double val1;
	double val2;
};

/** Class representing a color. */
class Color
{
public:
	unsigned short r;    /**< RED component of RGB color scheme */
	unsigned short g;    /**< GREEN component of RGB color scheme */
	unsigned short b;    /**< BLUE component of RGB color scheme */
	unsigned short a;    /**< ALPHA channel (transparency) */
	/** Constructor 
	 * Default color constructor rgbs = (0,0,0,0)
	 */
	Color(void);
	/** Constructor 
	 * Custom color constructor
	 */
	Color(unsigned short r, unsigned short g, unsigned short b, unsigned short a = 0);
	bool operator==(const Color& c) const;
	friend std::ostream& operator<<(ostream &output, const Color &C)
	{
		return output << "rgba(" << C.r << "," << C.g << "," << C.b << "," << C.a << ")";
	}
};


/** Class representing a range of two colors. */
class ColorRange {
public:
	/** Constructor 
	 * Creates range of two default colors rgbs = (0,0,0,0)
	 */
	ColorRange(void);
	/** Constructor 
	 * Creates range of custom colors
	 */
	ColorRange(Color color1, Color color2);
	Color color1;    /**< First color of the range */
	Color color2;    /**< Second color of the range */
};


/** A trasfomation from a single numeric value to another numeric value or color.
 *  As input value it accepts only values which fits into the input range of values.
 *  If input range values are same, then only a single value is accepted into this trasforamtion.
 *  Output depens on type of output; numeric value or color. Numeric output type
 *  can be determined by output range or by mahtematical expression.
 */

class Mapper {
public:
	static const int OUT_METHOD_UNDEFINED;
	static const int OUT_METHOD_RANGE;
	static const int OUT_METHOD_MEXPR;
	static const int OUT_METHOD_COLOR;
	
private:
	typedef exprtk::symbol_table<double> symbol_table_t;  //fastmathparser
	typedef exprtk::expression<double>     expression_t;  //fastmathparser
	symbol_table_t symbol_table;       //fastmathparser
//public:
	expression_t expression;           //fastmathparser
	double CV;                        //fastmathparser argument (cell value)

private:
	int out_method;

	Range in_range;
	Range out_range;
	ColorRange out_color_range;
	string math_expr_str; //math. expresssion string

public:

	/** Constructor.
	 *  The output value will be the result of linear interpolation of output range
	 *  according to input value realtion with the input range.
	 *  The input value must fit into input range.
	 *  @param in Input range of numeric values
	 *  @param out Output range of numeric values
	 */
	Mapper(Range in, Range out);

	/** Constructor.
	 *  The output value will be the result of mathematical expression,
	 *  given by 'math_expr' parameter.
	 *  The input value must fit into input range.
	 *  @param in Input range of numeric values
	 *  @param math_expr Mathematical expression
	 */
	Mapper(Range in, string math_expr);

	/** Constructor.
	 *  The output color will be the result of linear interpolation (r1..r2,g1..g2,b1..b2 and a1..a2) of output color range
	 *  according to input value realtion with the input range.
	 *  The input value must fit into input range.
	 *  @param in Input range of numeric values
	 *  @param colors Output range of colors
	 */
	Mapper(Range in, ColorRange colors);

private:

	/* Sets the Fastmathparser block */
	int setMathExpression(string math_expr_str);

public:

	/* Gets the output mathod: OUT_METHOD_RANGE or OUT_METHOD_MEXPR or OUT_METHOD_COLOR */
	int getOutputMethod(void);

	/** Get the calculated output value.
	 *  It works only when Mapper out_method is set to OUT_METHOD_RANGE or OUT_METHOD_MEXPR.
	 *  @param [in] in_value Input value to convert
	 *  @param [out] out_value Pointer to output value to be set
	 *  @return TRUE if input value fits into input range <br> FALSE otherwise (out_value will not be set)
	 */
	bool getValue(double in_value, double& out_value);

	/** Get the calculated output color.
	 *  It works only when Mapper out_method is set to OUT_METHOD_COLOR.
	 *  @param [in] in_value Input value to convert
	 *  @param [out] out_color Pointer to output color to be set
	 *  @return TRUE if input value fits into input range <br> FALSE otherwise (out_color will not be set)
	 */
	bool getColor(double in_value, Color& out_color);
};	



/** A set of multiple Mapper instances.
 *  It contains multiple Mapper instances. On every getValue() or getColor() call
 *  it iterates through all Mapper instances to find the first in which input_value
 *  fits. If found, it retruns trasformed value/color, otherwise it returns default value/color.
 */
class Map
{
private:
	std::vector<Mapper *> map_value;
	std::vector<Mapper *> map_color;
	
	double default_value;
	Color   default_color;

public:	
	
	/** Constructor. */
	Map();
	
	/** Destructor. */
	~Map();
	
	/** Add a new Mapper instance.
	 *  @param mapper Pointer to Mapper instance
	 *  @return 1 if successfull <br> 0 otherwise
	 */
	int addMapper(Mapper* mapper);
	
	/** Get trasformed value.
	 *  @param input_value Numeric value to trasform
	 *  @return Calculated value, if corresponding Mapper instance found, otherwise it returns default_value.
	 */
	double getValue(double input_value);
	
	/** Get color, corresponding to input_value.
	 *  @param input_value Numeric value to trasform
	 *  @return Calculated color, if corresponding Mapper instance found, otherwise it returns default_color.
	 */
	Color getColor(double input_value);

	/** Set default_value.
	 *  The default_value will be returned by getValue() method, if no corresonding Mapper instance found.
	 *  If no default_value is set, it will be 0.
	 *  @param default_value Value to which default_value is to be set
	 */
	void setDefaultValue(double default_value);

	/** Set default_color.
	 *  The default_color will be returned by getColor() method, if no corresonding Mapper instance found.
	 *  If no default_color is set, it will be (r,g,b,a)=(0,0,0,0).
	 *  @param default_color Color to which default_color is to be set
	 */
	void setDefaultColor(Color default_color);
};
#endif
