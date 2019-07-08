#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <mpi.h>

using namespace std;

/**The wind is encoded w.r.t it's direction as:
    North West (NW) = 0
    North(N) = 1
    North East(NE) = 2
    East(E) = 3
    South East(SE) = 4
    South(S) = 5
    South West(SW) = 6
    West(W) = 7
 **/

/**The fuel represent the combustion type, in particular dark green, green, light green**/
enum fuel {VS = 0, VM = 1, VC = 2};

/**encoding of the wind type**/
vector<string> wind {"NW", "N", "NE", "E", "SE", "S", "SW", "W"};

/**Each combustion type has an associeted value of burn**/
const int burn[3] = {20, 120, 240};

/**Probability that a lighting fall on a combustion cell**/
const float lighting = 0.00000002;

/**Probability that in an empty cell born a combustion type**/
const float growth = 0.002;

/**Size of the forest**/
const int cellSize = 10;
const int width = 160;
const int height =160;

int indexDirectionWind = 3;
string directionWind = wind[indexDirectionWind];
int currentIteration = 0;

map <string, vector<pair<int, int> > > windNeighbourds = {
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
	const float max = growth;

	float range = max - min;
	float random = range * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) + min;

	if(random < lighting)
		return true;
	return false;
}
bool fuelGrowth() {
	const float min = growth/2;
	const float max = (growth*200);

	float range = max - min;

	float random = range * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) + min;

	if(random < growth)
		return true;
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
void copyMatrix(int processCurrentGeneration[], int processNextGeneration[], int size) {
	for (int i = 0; i < size; i++)
		processCurrentGeneration[i] = processNextGeneration[i];
}
void printMatrix(int** m) {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			cout << m[i][j] << "      ";
		}
		cout << endl;
	}
}

void transitionFunction(int processCurrentGeneration[],int processNextGeneration[],int neighbourdDataOfLeftProcess[],int neighbourdDataOfRightProcess[], int size) {

	for (int i = 0; i < size; i++) {
		int curr_row = i / height;
		int curr_col = i % height;
		//Born of fuel into an empty cell
		if(processCurrentGeneration[i] == -1) {
			if(fuelGrowth())
				processNextGeneration[i] = fuel(rand() % 3);
		}

		//Fuel cell (1. Struck by lighting. 2. One of its neighbourds burn and i am in the same wind direction
		else if(processCurrentGeneration[i] >= 0 && processCurrentGeneration[i] <= 2) {
			if(struckByLightning()) {
				processNextGeneration[i] = burn[processCurrentGeneration[i]];
				//cout<<"HIT " << endl;
			}
			else{
				vector<pair<int,int> > neighbourds = windNeighbourds.at(wind[indexDirectionWind]);

				for(int z = 0; z < neighbourds.size(); z++) {

					int Nrow = curr_row + neighbourds[z].second;
					int Ncolumn = curr_col + neighbourds[z].first;

					if(Ncolumn == -1)
						Ncolumn = height - 1;
					if(Ncolumn >= height)
						Ncolumn = 0;

					if(Nrow < 0) {                                                                                                                                                //dati processo a sinistra
						if(neighbourdDataOfRightProcess[Ncolumn] >= burn[0]) {
							processNextGeneration[i] = burn[processCurrentGeneration[i]];
							break;
						}
					}

					else if(Nrow >= size / height) {                                                                                                                                                 //dati processo a destra
						if(neighbourdDataOfLeftProcess[Ncolumn] >= burn[0]) {
							processNextGeneration[i] = burn[processCurrentGeneration[i]];
							break;
						}
					}

					else{
							if(processCurrentGeneration[(height * Nrow) + (Ncolumn)] >= burn[0]) {
								processNextGeneration[i] = burn[processCurrentGeneration[i]];
								break;
							}
					}
				}
			}
		}
		else {
			if(processCurrentGeneration[i] < burn[2])
				processNextGeneration[i] = processCurrentGeneration[i] + burn[0];
			else
				processNextGeneration[i] = -1;
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

int main(int argc, char** argv) {


	al_init();
	al_install_keyboard();

	ALLEGRO_EVENT_QUEUE* queue;
	ALLEGRO_DISPLAY* disp;
	ALLEGRO_EVENT event;
	ALLEGRO_COLOR color;


	int currentGeneration[width][height];
	for(int i = 0; i < width; i++)
		for(int j = 0; j < height; j++)
			currentGeneration[i][j] = -1;

	int numberOfProcess, myRank;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	srand(time(NULL) + myRank);

	const int BLOCKROWS = width/numberOfProcess;                             /* number of rows in _block_ */
	const int BLOCKCOLS = height;                             /* number of cols in _block_ */

    if(myRank == 0) {
        queue = al_create_event_queue();
        disp = al_create_display(width*cellSize, height*cellSize);

        al_register_event_source(queue, al_get_keyboard_event_source());
        al_register_event_source(queue, al_get_display_event_source(disp));
        al_init_primitives_addon();
    }

	int processCurrentGeneration[BLOCKROWS*BLOCKCOLS];
	int processNextGeneration[BLOCKROWS*BLOCKCOLS];
	for(int i = 0; i < BLOCKROWS*BLOCKCOLS; i++)
		processNextGeneration[i] = -1;

	MPI_Datatype rowtype;
	MPI_Type_contiguous(BLOCKROWS*BLOCKCOLS, MPI_INT, &rowtype);
	MPI_Type_commit(&rowtype);

	MPI_Datatype singleNeighbourdRowType;
	MPI_Type_contiguous(BLOCKCOLS, MPI_INT, &singleNeighbourdRowType);
	MPI_Type_commit(&singleNeighbourdRowType);

	int neighbourdDataOfRightProcess[BLOCKCOLS];
	int neighbourdDataOfLeftProcess[BLOCKCOLS];

	MPI_Status status;

	MPI_Comm linearArrayTopology;
	int dimensions = 2, left, right, reorder = false;
	int periods[dimensions], topologyDimensions[dimensions], coords[dimensions];

	topologyDimensions[0] = numberOfProcess;
	topologyDimensions[1] = 1;
	periods[0] = 1, periods[1] = 0;

	MPI_Cart_create(MPI_COMM_WORLD, dimensions, topologyDimensions, periods, reorder, &linearArrayTopology);

	MPI_Scatter(&currentGeneration, 1, rowtype, processCurrentGeneration, BLOCKROWS*BLOCKCOLS, MPI_INT, 0, linearArrayTopology);


	MPI_Cart_shift(linearArrayTopology, 0, 1, &left, &right);
	MPI_Cart_coords(linearArrayTopology, myRank, dimensions, coords);


	while(currentIteration < 10000)
	{
		MPI_Send(&processCurrentGeneration, 1, singleNeighbourdRowType, left, 0, linearArrayTopology);
		MPI_Send(&processCurrentGeneration[(BLOCKROWS*BLOCKCOLS)-BLOCKCOLS], 1, singleNeighbourdRowType, right, 0, linearArrayTopology);

		MPI_Recv(&neighbourdDataOfLeftProcess, BLOCKCOLS, MPI_INT, left, 0, linearArrayTopology, &status);
		MPI_Recv(&neighbourdDataOfRightProcess, BLOCKCOLS, MPI_INT, right, 0, linearArrayTopology, &status);

		transitionFunction(processCurrentGeneration, processNextGeneration, neighbourdDataOfRightProcess, neighbourdDataOfLeftProcess, BLOCKROWS*BLOCKCOLS);
		copyMatrix(processCurrentGeneration, processNextGeneration, BLOCKROWS*BLOCKCOLS);

		MPI_Gather(&processCurrentGeneration, 1, rowtype, currentGeneration, 1, rowtype, 0, linearArrayTopology);

        if(myRank == 0)
        {
                al_peek_next_event(queue, &event);

                if((event.type == ALLEGRO_EVENT_KEY_DOWN) || (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)){
									MPI_Abort(linearArrayTopology, 1);
								}

								al_clear_to_color(al_map_rgb(0, 0, 0));


            for(int i = 0; i < width; i++) {
                for(int j = 0; j < height; j++) {
                   if(currentGeneration[i][j] == -1)
                        color = al_map_rgb(0, 0, 0);
                    else if(currentGeneration[i][j] >= burn[0])
                        color = al_map_rgb(255, 255 - currentGeneration[i][j], 0);
                    else if(currentGeneration[i][j] == fuel(0))
                        color = al_map_rgb(0,102, 0);
                    else if(currentGeneration[i][j] == fuel(1))
                        color = al_map_rgb(0, 153, 0);
                    else if(currentGeneration[i][j] == fuel(2))
                        color = al_map_rgb(0, 255, 0);

                    al_draw_filled_rectangle(i*cellSize, j*cellSize, i*cellSize + cellSize, j*cellSize + cellSize, color);
                }
            }
            al_flip_display();
        }
	//	if(currentIteration % 50 == 0)
		//	changeWindDirection();
		//MPI_Barrier(linearArrayTopology);
		currentIteration++;
	}

    if(myRank == 0) {
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
    }

	MPI_Finalize();

	return 0;
}
