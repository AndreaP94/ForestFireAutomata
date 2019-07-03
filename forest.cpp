#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

enum wind {NW = 0, N = 1, NE = 2, E = 3, SE = 4, S = 5, SW = 6, W = 7};
enum fuel {VS = 33, VM = 66, VC = 99};

const float lighting = 0.00002;
const float growth = 0.002;

const int width = 15;
const int height = 10;

bool struckByLightning() {
	const float min = lighting/2;
	const float max = growth/2;

	float range = max - min;

	float random = range * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) + min;

	if(random < lighting)
		return true;
		//cout << "Number = " << random << "\n";

	return false;
}
bool fuelGrowth() {
	const float min = growth/2;
	const float max = (growth*10)/2;

	float range = max - min;

	float random = range * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) + min;

	if(random < growth)
		return true;
		//cout << "Number = " << random << "\n";

	return false;
}

int** allocateMatrix() {
  int **M = new int*[width];
  for(int i = 0; i < width; i++)
    M[i] = new int[height];

  return M;
}
void initializeMatrices(int **A, int **B) {
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++) {
      A[i][j] = 0;
			B[i][j] = 1;
		}
}
void swapMatrix(int*** currentGeneration, int*** nextGeneration) {
  int** tmp = *currentGeneration;
  *currentGeneration = *nextGeneration;
  *nextGeneration = tmp;
}
void printMatrix(int** m) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      cout << m[i][j] << " ";
    }
    cout << "\n";
  }
}

int main() {
	srand(time(NULL));

	map <wind, vector<pair<int, int>>> windNeighbourds = {
		{NW, { {0, 1}, {1, 1}, {1, 0} } },
		{N, { {1, -1}, {1, 0}, {1, 1} } },
		{NE, { {1, -1}, {0, -1}, {1, 0} } },
		{E, { {-1, -1}, {0, -1}, {1, -1} } },
		{SE, { {-1, 0}, {-1, -1}, {0, -1} } },
		{S, { {-1, -1}, {-1, 0}, {-1, 1} } },
		{SW, { {-1, 0}, {-1, 1}, {0, 1} } },
		{W, { {-1, 1}, {0, 1}, {1, 1} } }
	};

	int** currentGeneration = allocateMatrix();
	int** nextGeneration = allocateMatrix();

	initializeMatrices(currentGeneration, nextGeneration);

	/*
	printMatrix(nextGeneration);
	swapMatrix(&currentGeneration, &nextGeneration);
	cout << "\n";
	printMatrix(nextGeneration);
	*/

	/*
	for (int i = 0; i < 100; i++)
		struckByLightning();
		fuelGrowth();
	*/



	return 0;
}
