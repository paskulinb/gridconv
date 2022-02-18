#ifndef GRIDCONV_HPP
#define GRIDCONV_HPP

template<typename T>
int GridConverter::createBinFile(Grid& grid, Map& map, std::string out_filepath)
{
	int ncells = grid.nrows * grid.ncols;
	
	T* file = new T[ncells];
	
	for (unsigned int cell=0; cell<ncells; cell++) {
		double ov = map.getValue(grid.getCellValue(cell));
		//cout << ov << endl;
		*(file + cell) = (T)ov; //cast to wanted output type
	}

	FILE* fh;
	if (!(fh = fopen(out_filepath.c_str(), "w"))) {
		cout << "GridConverter::createBinFile: Error creating file" << endl;
		fclose (fh);
		return 0;
	}
	
	fwrite ( file, grid.nrows * grid.ncols * sizeof(T), 1, fh );
	fclose (fh);

	return 1;
}
#endif
