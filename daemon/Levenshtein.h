// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#include <string>

#define MATRIX_SIZE 64

class Levenshtein {
public:
	static void clear();
	static void initialize();
	static double similarity(const std::string &source, const std::string &target);

private:
	static bool initialized;
	static int **matrix;
	static int matrix_size;

	static void createMatrix(int size);
	static void deleteMatrix();
	static void resizeMatrix(int size);
};
#endif
