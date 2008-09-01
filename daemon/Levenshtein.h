#ifndef LEVENSHTEIN_H
/* defines */
#define LEVENSHTEIN_H
/* initial size matrix */
#define MATRIX_SIZE 64

/* includes */
#include <string>

/* namespaces */
using namespace std;

/* Levenshtein */
class Levenshtein {
	public:
		/* constructors */
		Levenshtein();

		/* destructors */
		~Levenshtein();

		/* methods */
		double similarity(const string &source, const string &target);

	private:
		/* variables */
		int **matrix;
		int matrix_size;

		/* methods */
		void createMatrix(int size);
		void deleteMatrix();
		void resizeMatrix(int size);
};
#endif