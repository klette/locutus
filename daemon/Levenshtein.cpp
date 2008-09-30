#include "Levenshtein.h"

using namespace std;

/* constructors/destructor */
Levenshtein::Levenshtein() {
	createMatrix(MATRIX_SIZE);
}

Levenshtein::~Levenshtein() {
	deleteMatrix();
}

/* methods */
double Levenshtein::similarity(const string &source, const string &target) {
	int sl = source.length();
	int tl = target.length();
	if (sl == 0 || tl == 0)
		return 0.0;
	int size = max(sl, tl);
	if (size + 1 > matrix_size)
		resizeMatrix(size + 1);

	for (int a = 1; a <= sl; ++a) {
		char s = source[a - 1];
		/* make A-Z lowercase */
		if (s >= 'A' && s <= 'Z')
			s += 32;
		for (int b = 1; b <= tl; ++b) {
			char t = target[b - 1];
			/* make A-Z lowercase */
			if (t >= 'A' && t <= 'Z')
				t += 32;
			int cost = (s == t ? 0 : 1);

			int above = matrix[a - 1][b];
			int left = matrix[a][b - 1];
			int diag = matrix[a - 1][b - 1];
			int cell = min(above + 1, min(left + 1, diag + cost));
			if (a > 2 && b > 2) {
				int trans = matrix[a - 2][b - 2] + 1;
				if (source[a - 2] != t)
					++trans;
				if (s != target[b - 2])
					++trans;
				if (cell > trans)
					cell = trans;
			}
			matrix[a][b] = cell;
		}
	}
	return 1.0 - (double) matrix[sl][tl] / (double) size;
}

/* private methods */
void Levenshtein::createMatrix(int size) {
	matrix_size = size;
	matrix = new int*[matrix_size];
	for (int a = 0; a < matrix_size; ++a) {
		matrix[a] = new int[matrix_size];
		matrix[a][0] = a;
		matrix[0][a] = a;
	}
}

void Levenshtein::deleteMatrix() {
	for (int a = 0; a < matrix_size; ++a)
		delete [] matrix[a];
	delete [] matrix;
}

void Levenshtein::resizeMatrix(int size) {
	deleteMatrix();
	createMatrix(size);
}
