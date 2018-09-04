#include "map.h"
#include <string.h>
#include <stdlib.h>
#include <cfloat>    //DBL_MIN,DBL_MAX
#include <cmath>

#include <iostream>
using namespace std;

parser_t parser;  //fastmathparser

const int Mapper::OUT_METHOD_UNDEFINED = 0;
const int Mapper::OUT_METHOD_RANGE = 1;
const int Mapper::OUT_METHOD_MEXPR = 2;
const int Mapper::OUT_METHOD_COLOR = 3;


Map::Map()
{
	this->default_value = 0;
	this->default_color = Color(0,0,0,0);
}

Map::~Map()
{
	//Clear memory: Delte all Mappers
	for (std::vector<Mapper *>::iterator it = this->map_value.begin(); it!=this->map_value.end(); ++it) {
		delete *it;
	}
	for (std::vector<Mapper *>::iterator it = this->map_color.begin(); it!=this->map_color.end(); ++it) {
		delete *it;
	}
}


int Map::addMapper(Mapper* mapper)
{
	if (mapper->getOutputMethod() == Mapper::OUT_METHOD_RANGE || mapper->getOutputMethod() == Mapper::OUT_METHOD_MEXPR) {
		this->map_value.push_back(mapper);
		return 1;
	}
	else if (mapper->getOutputMethod() == Mapper::OUT_METHOD_COLOR) {
		this->map_color.push_back(mapper);
		return 1;
	}
	return 0;
}

double Map::getValue(double input_value)
{
	for (std::vector<Mapper *>::iterator it = this->map_value.begin(); it!=this->map_value.end(); ++it) {

		double value;
		if( (*it)->getValue(input_value, value) ) return value;

	}

	return this->default_value;
}

Color Map::getColor(double input_value)
{
	for (std::vector<Mapper *>::iterator it = this->map_color.begin(); it!=this->map_color.end(); ++it) {
		
		Color color;
		if( (*it)->getColor(input_value, color) ) return color;
	}
//cout << input_value << " ";
	return this->default_color;
}

void Map::setDefaultColor(Color default_color)
{
	this->default_color = default_color;
}

void Map::setDefaultValue(double default_value)
{
	this->default_value = default_value;
}


/************************************************************/
/*** Mapper class definition ***/
/************************************************************/

Mapper::Mapper(Range in, Range out)
: in_range(in),out_range(out),out_method(Mapper::OUT_METHOD_RANGE)
{
}

Mapper::Mapper(Range in, string math_expr)
: in_range(in),math_expr_str(math_expr),out_method(Mapper::OUT_METHOD_MEXPR)
{
	this->setMathExpression(math_expr_str);
}

Mapper::Mapper(Range in, ColorRange colors)
: in_range(in),out_color_range(colors),out_method(Mapper::OUT_METHOD_COLOR)
{
}

int Mapper::setMathExpression(string math_expr_str)
{
	this->symbol_table.add_variable("CV", this->CV);
	this->symbol_table.add_constants();

	this->expression.register_symbol_table(this->symbol_table);

	this->math_expr_str = math_expr_str;
	parser.compile(this->math_expr_str,this->expression);
}

int Mapper::getOutputMethod(void)
{
	return this->out_method;
}

bool Mapper::getValue(double in_value, double& out_value)
{
	if (in_value >= this->in_range.val1 && in_value <= this->in_range.val2) {
	
		if (this->out_method == Mapper::OUT_METHOD_RANGE) {
			
			if (this->in_range.val1 == this->in_range.val2) { //single input value
				out_value = this->out_range.val1;
				return true;
			}
			
			if (this->out_range.val1 == this->out_range.val2) { //single(constant) output value
				out_value = this->out_range.val1;
				return true;
			}
			
			double ratio = (double)(in_value - this->in_range.val1)/(double)(this->in_range.val2 - this->in_range.val1);
			double output_value = this->in_range.val1 + (this->in_range.val2 - this->in_range.val1) * ratio;
			out_value = output_value;
			return true;
		}

		if (this->out_method == Mapper::OUT_METHOD_MEXPR) {
			this->CV = in_value;
			out_value = this->expression.value();
			return true;
		}
	}
	return false;
}

bool Mapper::getColor(double in_value, Color& out_color)
{
//cout << value << "[" << this->in_range.val1 << "," << this->in_range.val2 << "]" << endl;
	if (in_value >= this->in_range.val1 && in_value <= this->in_range.val2) {
		
		if (this->in_range.val1 == this->in_range.val2) { //single input value
//cout << "=r=" << this->out_color.color1 << endl;
			out_color = this->out_color_range.color1;
			return true;
		}
		
		if (this->out_color_range.color1 == this->out_color_range.color2) { //single output color
//cout << "=c=" << this->out_color.color1 << endl;
			out_color = this->out_color_range.color1;
			return true;
		}

		//color range
		double ratio = (double)(in_value - in_range.val1)/(double)(in_range.val2 - in_range.val1);

		unsigned short r = (unsigned short)roundf((double)this->out_color_range.color1.r + (double)(this->out_color_range.color2.r - this->out_color_range.color1.r) * ratio);
		unsigned short g = (unsigned short)roundf((double)this->out_color_range.color1.g + (double)(this->out_color_range.color2.g - this->out_color_range.color1.g) * ratio);
		unsigned short b = (unsigned short)roundf((double)this->out_color_range.color1.b + (double)(this->out_color_range.color2.b - this->out_color_range.color1.b) * ratio);
		unsigned short a = (unsigned short)roundf((double)this->out_color_range.color1.a + (double)(this->out_color_range.color2.a - this->out_color_range.color1.a) * ratio);
//cout << "!=" << Color(r,g,b,a) << endl;

		out_color = Color(r,g,b,a);
		return true;
	}
	return false;
}


Color::Color(void) : r(0),g(0),b(0),a(0)
{
}

Color::Color(unsigned short r, unsigned short g, unsigned short b, unsigned short a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = (a>127) ? 127 : a;
}

bool Color::operator==(const Color& c) const {
	
	if (this->r == c.r && 
	    this->g == c.g &&
	    this->b == c.b &&
	    this->a == c.a) return true;

	return false;
}

Range::Range(double val1, double val2)
: val1(val1), val2(val2)
{
}

ColorRange::ColorRange(void)
{
	this->color1 = Color();
	this->color2 = Color();
}

ColorRange::ColorRange(Color color1, Color color2)
: color1(color1), color2(color2)
{
}

