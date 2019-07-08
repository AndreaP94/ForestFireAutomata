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

const float min_l = lighting/2;
const float max_l = growth;
const float range_l = max_l - min_l;

const float min_g = growth/2;
const float max_g = (growth*200);
const float range_g = max_g - min_g;

bool enableGraphic = false;
/**Size of the forest**/
const int cellSize = 10;
const int width = 160;
const int height = 160;

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

map <int, ALLEGRO_COLOR> colorToDisplay;

bool struckByLightning() {
	float random = range_l * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) + min_l;
	return random < lighting ? true : false;
}
bool fuelGrowth() {
	float random = range_g * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) + min_g;
	return random < growth ? true : false;
}

void copyMatrix(int processCurrentGeneration[], int processNextGeneration[], int size) {
	for (int i = 0; i < size; i++)
		processCurrentGeneration[i] = processNextGeneration[i];
}

void transitionFunction(int processCurrentGeneration[],int processNextGeneration[],int neighbourdDataOfLeftProcess[],
                        int neighbourdDataOfRightProcess[], int size) {

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

	ALLEGRO_EVENT_QUEUE* queue;
	ALLEGRO_DISPLAY* disp;
	ALLEGRO_EVENT event;
	ALLEGRO_COLOR color;

	if(enableGraphic) {
		al_init();
		al_install_keyboard();
	}

	int currentGeneration[width][height];
	int numberOfProcess, myRank;

	MPI_Init(&argc, &argv);
	float start = MPI_Wtime();

	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcess);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	if(numberOfProcess % 2 == 0){
		cout<<"You have to specify an odd number of Process" << endl;
		MPI_Finalize();
		return 0;
	}

	srand(time(NULL) + myRank);

	bool iAmTheMaster = false;
	if(myRank == 0) {
		iAmTheMaster = true;
	}

	const int BLOCKROWS = (width/(numberOfProcess - 1));                              /* number of rows in _block_ */
	const int BLOCKCOLS = height;                             /* number of cols in _block_ */
	int processCurrentGeneration[BLOCKROWS*BLOCKCOLS];
	int processNextGeneration[BLOCKROWS*BLOCKCOLS];
	int neighbourdDataOfRightProcess[BLOCKCOLS];
	int neighbourdDataOfLeftProcess[BLOCKCOLS];

	if(iAmTheMaster ) {
		for(int i = 0; i < width; i++)
			for(int j = 0; j < height; j++)
				currentGeneration[i][j] = -1;

		if(enableGraphic) {
			colorToDisplay =  {
				{-1,    al_map_rgb(0, 0, 0)},
				{0,     al_map_rgb(0,102, 0)},
				{1,     al_map_rgb(0, 153, 0)},
				{2,     al_map_rgb(0, 255, 0)},
				{20,    al_map_rgb(255, 235, 0)},
				{40,    al_map_rgb(255, 215, 0)},
				{60,    al_map_rgb(255, 195, 0)},
				{80,    al_map_rgb(255, 175, 0)},
				{100, al_map_rgb(255, 155, 0)},
				{120, al_map_rgb(255, 135, 0)},
				{140, al_map_rgb(255, 115, 0)},
				{160, al_map_rgb(255, 95, 0)},
				{180, al_map_rgb(255, 75, 0)},
				{200, al_map_rgb(255, 55, 0)},
				{220, al_map_rgb(255, 35, 0)},
				{240, al_map_rgb(255, 15, 0)}
			};

			queue = al_create_event_queue();
			disp = al_create_display(width*cellSize, height*cellSize);

			al_register_event_source(queue, al_get_keyboard_event_source());
			al_register_event_source(queue, al_get_display_event_source(disp));
			al_init_primitives_addon();
		}
	}

	else {
		for(int i = 0; i < BLOCKROWS*BLOCKCOLS; i++) {
			processNextGeneration[i] = -1;
			processCurrentGeneration[i] = -1;

			if(i < BLOCKCOLS) {
				neighbourdDataOfRightProcess[i] = -1;
				neighbourdDataOfLeftProcess[i] = -1;
			}
		}
	}

	MPI_Datatype rowtype;
	MPI_Type_contiguous(BLOCKROWS*BLOCKCOLS, MPI_INT, &rowtype);
	MPI_Type_commit(&rowtype);

	MPI_Datatype singleNeighbourdRowType;
	MPI_Type_contiguous(BLOCKCOLS, MPI_INT, &singleNeighbourdRowType);
	MPI_Type_commit(&singleNeighbourdRowType);

	MPI_Status status;
	MPI_Comm linearArrayTopology;
	MPI_Comm comm;
	MPI_Group origin_group, new_group;
	MPI_Comm_group(MPI_COMM_WORLD, &origin_group);

	int ex[1] = {0};
	MPI_Group_excl(origin_group, 1, ex, &new_group);
	MPI_Comm_create(MPI_COMM_WORLD, new_group, &comm);

	if(!iAmTheMaster) {
		MPI_Comm_rank(comm, &myRank);
		MPI_Comm_size(comm, &numberOfProcess);
	}

	int dimensions = 2, left, right, reorder = false;
	int periods[dimensions], topologyDimensions[dimensions], coords[dimensions];

	topologyDimensions[0] = numberOfProcess;
	topologyDimensions[1] = 1;
	periods[0] = 1, periods[1] = 0;

	if(!iAmTheMaster) {
		MPI_Cart_create(comm, dimensions, topologyDimensions, periods, reorder, &linearArrayTopology);
		MPI_Cart_shift(linearArrayTopology, 0, 1, &left, &right);
		MPI_Cart_coords(linearArrayTopology, myRank, dimensions, coords);
	}

	while(currentIteration < 1000)
	{
		if(!iAmTheMaster) {
			MPI_Send(&processCurrentGeneration, 1, singleNeighbourdRowType, left, 0, linearArrayTopology);
			MPI_Send(&processCurrentGeneration[(BLOCKROWS*BLOCKCOLS)-BLOCKCOLS], 1, singleNeighbourdRowType, right, 0, linearArrayTopology);

			MPI_Recv(&neighbourdDataOfLeftProcess, BLOCKCOLS, MPI_INT, left, 0, linearArrayTopology, &status);
			MPI_Recv(&neighbourdDataOfRightProcess, BLOCKCOLS, MPI_INT, right, 0, linearArrayTopology, &status);

			transitionFunction(processCurrentGeneration, processNextGeneration, neighbourdDataOfRightProcess, neighbourdDataOfLeftProcess, BLOCKROWS*BLOCKCOLS);
			copyMatrix(processCurrentGeneration, processNextGeneration, BLOCKROWS*BLOCKCOLS);

			MPI_Send(&processCurrentGeneration, 1, rowtype, 0, 12, MPI_COMM_WORLD);

			if(currentIteration % 50 == 0)
				MPI_Recv(&indexDirectionWind, 1, MPI_INT, 0, 11,MPI_COMM_WORLD, &status);
		}

		if(iAmTheMaster)
		{

			for(int i = 1; i < numberOfProcess; i++)
				MPI_Recv(&currentGeneration[BLOCKROWS * (i - 1)], 1, rowtype, i,12, MPI_COMM_WORLD, &status);


			if(enableGraphic) {
				al_peek_next_event(queue, &event);

				if((event.type == ALLEGRO_EVENT_KEY_DOWN) || (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)) {
					MPI_Abort(linearArrayTopology, 1);
				}

				al_clear_to_color(al_map_rgb(0, 0, 0));

				for(int i = 0; i < width; i++) {
					for(int j = 0; j < height; j++) {
						al_draw_filled_rectangle(i*cellSize, j*cellSize, i*cellSize + cellSize, j*cellSize + cellSize,
						                         colorToDisplay.at(currentGeneration[i][j]));
					}
				}

				al_flip_display();
			}
			if(currentIteration % 50 == 0) {
				changeWindDirection();
				for(int i = 1; i < numberOfProcess; i++)
					MPI_Send(&indexDirectionWind, 1, MPI_INT, i, 11, MPI_COMM_WORLD);
			}
		}
		currentIteration++;
	}

	if(iAmTheMaster && enableGraphic) {
		al_destroy_display(disp);
		al_destroy_event_queue(queue);
	}

	MPI_Type_free(&rowtype);
	MPI_Type_free(&singleNeighbourdRowType);

	float end = MPI_Wtime();
	cout<<"TOTAL TIME " << end - start << endl;
	MPI_Finalize();
	return 0;
}
