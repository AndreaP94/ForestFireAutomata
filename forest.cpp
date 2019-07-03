#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include <cstdlib>
#include <vector>
#include <ctime>

using namespace std;

//enum wind {NW = 0, N = 1, NE = 2, E = 3, SE = 4, S = 5, SW = 6, W = 7};
enum fuel {VS = 0, VM = 1, VC = 2};

vector<string> wind {"NW", "N", "NE", "E", "SE", "S", "SW", "W"};
const int burn[3] = {33, 66, 99};

const float lighting = 0.00002;
const float growth = 0.002;

const int width = 15;
const int height = 10;

string directionWind = wind[0];
int indexDirectionWind = 0;
int currentIteration = 0;

map <string, vector<pair<int, int>>> windNeighbourds = {
	{wind[0], { {0, 1}, {1, 1}, {1, 0} } },
	{wind[1], { {1, -1}, {1, 0}, {1, 1} } },
	{wind[2], { {1, -1}, {0, -1}, {1, 0} } },
	{wind[3], { {-1, -1}, {0, -1}, {1, -1} } },
	{wind[4], { {-1, 0}, {-1, -1}, {0, -1} } },
	{wind[5], { {-1, -1}, {-1, 0}, {-1, 1} } },
	{wind[6], { {-1, 0}, {-1, 1}, {0, 1} } },
	{wind[7], { {-1, 1}, {0, 1}, {1, 1} } }
};




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
    for (int j = 0; j < height; j++)
      A[i][j] = B[i][j] = -1;
}
void copyMatrix(int** currentGeneration, int** nextGeneration) {
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			currentGeneration[i][j] = nextGeneration[i][j];
}
void printMatrix(int** m) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      cout << m[i][j] << " ";
    }
    cout << "\n";
  }
}

void transitionFunction(int** currentGeneration, int** nextGeneration) {

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if(currentGeneration[i][j] == -1) {
				if(fuelGrowth()) {
					nextGeneration[i][j] = fuel(rand() % 3);
				}
			}
			else if(currentGeneration[i][j] >= 0 && currentGeneration[i][j] <= 2) {
				if(struckByLightning()) {
					nextGeneration[i][j] = burn[currentGeneration[i][j]];
				}
				else{
					vector<pair<int,int>> neighbourds = windNeighbourds.at(wind[indexDirectionWind]);

					for(int z = 0; z < neighbourds.size(); z++){

							int Nrow = i + neighbourds[z].first;
							int Ncolumn = j + neighbourds[z].second;

							if(Nrow < 0){
								Nrow = width - 1;
							}
							if(Ncolumn < 0){
								Ncolumn = height - 1;
							}
							if(Nrow < 0 || Nrow >= width || Ncolumn < 0 || Ncolumn >= height)
								cout << " SONO QUI";
								else
							if(currentGeneration[Nrow][Ncolumn] > 33){
								nextGeneration[i][j] = burn[currentGeneration[i][j]];
								break;
							}
					}


				}
				//controllo sui vicini
			}
			else {
				if(currentGeneration[i][j] < burn[2])
					nextGeneration[i][j] = currentGeneration[i][j] + burn[0];
				else
					nextGeneration[i][j] = -1;
			}
		}
	}
}

void changeWindDirection() {
	int a[2] = {-1, 1};
	int index = rand() % 2;

	if(indexDirectionWind + a[index] < 0) {
		indexDirectionWind = 7;
	} else if(indexDirectionWind + a[index] > 7) {
		indexDirectionWind = 0;
	} else {
		indexDirectionWind += a[index];
	}

	directionWind = wind[indexDirectionWind];
}

int main() {
	srand(time(NULL));


	int** currentGeneration = allocateMatrix();
	int** nextGeneration = allocateMatrix();

	initializeMatrices(currentGeneration, nextGeneration);

	for (int i = 0; i < 20; i++) {

		if(currentIteration % 2 != 0)
			changeWindDirection();

		cout << currentIteration << " WIND = " << wind[indexDirectionWind] << '\n';
		//printMatrix(currentGeneration);
		transitionFunction(currentGeneration, nextGeneration);
		copyMatrix(currentGeneration, nextGeneration);
		//swapMatrix(&currentGeneration, &nextGeneration);
		cout << "\n";

		currentIteration++;
	}




	//printMatrix(currentGeneration);
	/*
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
