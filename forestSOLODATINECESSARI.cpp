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

struct modifiedCellOfCurrentGeneration {
	int newValue;
	int row;
	int column;
};
struct modifiedValueOfNeighbourdProcess {
	int newValue;
	int position;
};
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

int currentGeneration[width][height];

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

void copyMatrix(int processCurrentGeneration[], int processNextGeneration[], int size) {

	for (int i = 0; i < size; i++)
		processCurrentGeneration[i] = processNextGeneration[i];
}

void transitionFunction(int processCurrentGeneration[],int processNextGeneration[],int neighbourdDataOfLeftProcess[],
                        int neighbourdDataOfRightProcess[], int size, int myRank, int numberOfProcess,
                        vector<modifiedCellOfCurrentGeneration> &changedValues, vector<modifiedValueOfNeighbourdProcess> &leftProcessValueChangedToSend,
                        vector<modifiedValueOfNeighbourdProcess> &rightProcessValueChangedToSend) {

	for (int i = 0; i < size; i++) {
		int curr_row = i / height;
		int curr_col = i % height;
		bool modified = false;
		//Born of fuel into an empty cell
		if(processCurrentGeneration[i] == -1) {
			if(fuelGrowth()) {
				processNextGeneration[i] = fuel(rand() % 3);
				modified = true;

			}
		}
		//Fuel cell (1. Struck by lighting. 2. One of its neighbourds burn and i am in the same wind direction
		else if(processCurrentGeneration[i] >= 0 && processCurrentGeneration[i] <= 2) {
			if(struckByLightning()) {
				processNextGeneration[i] = burn[processCurrentGeneration[i]];
				modified = true;
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
							modified = true;
							break;
						}
					}
					else if(Nrow >= size / height) {                                                                                                                                                 //dati processo a destra
						if(neighbourdDataOfLeftProcess[Ncolumn] >= burn[0]) {
							processNextGeneration[i] = burn[processCurrentGeneration[i]];
							modified = true;
							break;
						}
					}
					else{
						if(processCurrentGeneration[(height * Nrow) + (Ncolumn)] >= burn[0]) {
							processNextGeneration[i] = burn[processCurrentGeneration[i]];
							modified = true;
							break;
						}
					}
				}
			}
		}
		else {
			if(processCurrentGeneration[i] < burn[2]) {
				processNextGeneration[i] = processCurrentGeneration[i] + burn[0];
			}
			else{
				processNextGeneration[i] = -1;
			}
		}

		if(modified && myRank != 0)
		{
			modifiedCellOfCurrentGeneration changed;
			changed.row = curr_row + (myRank * (width / numberOfProcess));
			changed.column = curr_col;
			changed.newValue = processNextGeneration[i];
			changedValues.push_back(changed);
		}

		if(modified && curr_row == 0)
		{
			modifiedValueOfNeighbourdProcess changed;
			changed.position = curr_col;
			changed.newValue = processNextGeneration[i];
			leftProcessValueChangedToSend.push_back(changed);
		}

		if(modified && curr_row == (width/numberOfProcess) - 1)
		{
			modifiedValueOfNeighbourdProcess changed;
			changed.position = curr_col;
			changed.newValue = processNextGeneration[i];
			rightProcessValueChangedToSend.push_back(changed);
		}

		if(myRank == 0 && modified)
			currentGeneration[curr_row + (myRank * (width / numberOfProcess))][curr_col] = processNextGeneration[i];
	}


	for(int i = 0; i < height; i++){
		if(neighbourdDataOfLeftProcess[i] >= burn[0] && neighbourdDataOfLeftProcess[i] < burn[2]) {
			neighbourdDataOfLeftProcess[i] += burn[0];
		}
		else{
			neighbourdDataOfLeftProcess[i] = -1;
		}
		if(neighbourdDataOfRightProcess[i] >= burn[0] && neighbourdDataOfRightProcess[i] < burn[2]) {
			neighbourdDataOfRightProcess[i] += burn[0];
		}
		else{
			neighbourdDataOfRightProcess[i] = -1;
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



	for(int i = 0; i < width; i++)
		for(int j = 0; j < height; j++)
			currentGeneration[i][j] = -1;

	int numberOfProcess, myRank;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	srand(time(NULL) + myRank);

	const int BLOCKROWS = (width/numberOfProcess);                             /* number of rows in _block_ */
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
	int neighbourdDataOfRightProcess[BLOCKCOLS];
	int neighbourdDataOfLeftProcess[BLOCKCOLS];

	for(int i = 0; i < BLOCKROWS*BLOCKCOLS; i++) {
		processNextGeneration[i] = -1;
		processCurrentGeneration[i] = -1;

		if(i < BLOCKCOLS) {
			neighbourdDataOfRightProcess[i] = -1;
			neighbourdDataOfLeftProcess[i] = -1;
		}
	}

	MPI_Datatype cellOfCurrentGeneration;
	MPI_Datatype type[3] = {MPI_INT, MPI_INT, MPI_INT};
	int blocklen[3] = {1, 1, 1};
	MPI_Aint displ[3] = {0, 1 * sizeof(int), 2 * sizeof(int)};
	MPI_Type_struct( 3, blocklen, displ, type, &cellOfCurrentGeneration);
	MPI_Type_commit(&cellOfCurrentGeneration);

	MPI_Datatype cellOfNeighbourdProcess;
	MPI_Datatype type_n[2] = {MPI_INT, MPI_INT};
	int blocklen_n[2] = {1, 1};
	MPI_Aint displ_n[2] = {0, 1 * sizeof(int)};
	MPI_Type_struct(2, blocklen_n, displ_n, type_n, &cellOfNeighbourdProcess);
	MPI_Type_commit(&cellOfNeighbourdProcess);

	MPI_Status status;

	MPI_Comm linearArrayTopology;
	int dimensions = 2, left, right, reorder = false;
	int periods[dimensions], topologyDimensions[dimensions], coords[dimensions];

	topologyDimensions[0] = numberOfProcess;
	topologyDimensions[1] = 1;
	periods[0] = 1, periods[1] = 0;

	MPI_Cart_create(MPI_COMM_WORLD, dimensions, topologyDimensions, periods, reorder, &linearArrayTopology);
	MPI_Cart_shift(linearArrayTopology, 0, 1, &left, &right);
	MPI_Cart_coords(linearArrayTopology, myRank, dimensions, coords);

	while(currentIteration < 5000)
	{
		vector<modifiedCellOfCurrentGeneration> changedValues;
		vector<modifiedValueOfNeighbourdProcess> leftProcessValueChangedToSend;
		vector<modifiedValueOfNeighbourdProcess> rightProcessValueChangedToSend;

		transitionFunction(processCurrentGeneration, processNextGeneration, neighbourdDataOfRightProcess,
		                   neighbourdDataOfLeftProcess, BLOCKROWS*BLOCKCOLS, myRank, numberOfProcess, changedValues,
		                   leftProcessValueChangedToSend, rightProcessValueChangedToSend );
		copyMatrix(processCurrentGeneration, processNextGeneration, BLOCKROWS*BLOCKCOLS);

		/***********************/
		int dimleft = leftProcessValueChangedToSend.size();
		int dimright;
		MPI_Send(&dimleft, 1, MPI_INT, left, 94, linearArrayTopology);
		MPI_Send(&leftProcessValueChangedToSend[0], dimleft, cellOfNeighbourdProcess, left, 95, linearArrayTopology);

		MPI_Recv(&dimright, 1, MPI_INT, right, 94, linearArrayTopology, &status);
		modifiedValueOfNeighbourdProcess mod_r[dimright];
		MPI_Recv(&mod_r, dimright, cellOfNeighbourdProcess, right, 95, linearArrayTopology, &status);

		for(int j = 0; j < dimright; j++) {
			neighbourdDataOfRightProcess[mod_r[j].position] = mod_r[j].newValue;
		}

		dimright = rightProcessValueChangedToSend.size();
		MPI_Send(&dimright, 1, MPI_INT, right, 94, linearArrayTopology);
		MPI_Send(&rightProcessValueChangedToSend[0], dimright, cellOfNeighbourdProcess, right, 95, linearArrayTopology);

		MPI_Recv(&dimleft, 1, MPI_INT, left, 94, linearArrayTopology, &status);
		modifiedValueOfNeighbourdProcess mod_l[dimleft];
		MPI_Recv(&mod_l, dimleft, cellOfNeighbourdProcess, left, 95, linearArrayTopology, &status);

		for(int j = 0; j < dimleft; j++) {
			neighbourdDataOfLeftProcess[mod_l[j].position] = mod_l[j].newValue;
		}
		/***********************/
		int dim = changedValues.size();
		if(myRank != 0) {
			MPI_Send(&dim, 1, MPI_INT, 0, 98, linearArrayTopology);
			MPI_Send(&changedValues[0], dim, cellOfCurrentGeneration, 0, 99, linearArrayTopology);
		}

		if(myRank == 0) {
			//	int dim;
			for(int i = 1; i < numberOfProcess; i++) {
				MPI_Recv(&dim, 1, MPI_INT, i, 98, linearArrayTopology, &status);
				modifiedCellOfCurrentGeneration mod[dim];
				MPI_Recv(&mod, dim, cellOfCurrentGeneration, i, 99, linearArrayTopology, &status);

				for(int j = 0; j < dim; j++) {
					currentGeneration[mod[j].row][mod[j].column] = mod[j].newValue;
				}
			}
			al_peek_next_event(queue, &event);

			if((event.type == ALLEGRO_EVENT_KEY_DOWN) || (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)) {
				MPI_Abort(linearArrayTopology, 1);
			}

			al_clear_to_color(al_map_rgb(0, 0, 0));


			for(int i = 0; i < width; i++) {
				for(int j = 0; j < height; j++) {
					if(currentGeneration[i][j] == -1)
						color = al_map_rgb(0, 0, 0);
					else if(currentGeneration[i][j] >= burn[0]) {
						color = al_map_rgb(255, 255 - currentGeneration[i][j], 0);

						if(currentGeneration[i][j] < burn[2])
							currentGeneration[i][j] += burn[0];
						else
							currentGeneration[i][j] = -1;
					}
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


	MPI_Type_free(&cellOfCurrentGeneration);
	MPI_Type_free(&cellOfNeighbourdProcess);

	MPI_Finalize();

	return 0;
}
