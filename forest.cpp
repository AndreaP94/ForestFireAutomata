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
const int width = 300;
const int height = 200;

int indexDirectionWind = 7;
string directionWind = wind[indexDirectionWind];
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
void copyMatrix(int** currentGeneration, int** nextGeneration) {
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			currentGeneration[i][j] = nextGeneration[i][j];
}
void printMatrix(int** m) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      cout << m[i][j] << "      ";
    }
    cout << endl;
  }
}

void transitionFunction(int** currentGeneration, int** nextGeneration) {

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
            
            //Born of fuel into an empty cell
			if(currentGeneration[i][j] == -1) {
				if(fuelGrowth()) {
					nextGeneration[i][j] = fuel(rand() % 3);
				}
			}
            //Fuel cell (1. Struck by lighting. 2. One of its neighbourds burn and i am in the same wind direction
			else if(currentGeneration[i][j] >= 0 && currentGeneration[i][j] <= 2) {
				if(struckByLightning()) {
					nextGeneration[i][j] = burn[currentGeneration[i][j]];
				}
				else{
					vector<pair<int,int>> neighbourds = windNeighbourds.at(wind[indexDirectionWind]);

					for(int z = 0; z < neighbourds.size(); z++){

							int Nrow = i + neighbourds[z].second;
							int Ncolumn = j + neighbourds[z].first;
							
							if(Nrow < 0)
								Nrow = width - 1;
							if(Ncolumn < 0)
								Ncolumn = height - 1;
					        if(Nrow >= width)
                                Nrow = 0;   
                            if(Ncolumn >= height)
                                Ncolumn = 0;

							if(currentGeneration[Nrow][Ncolumn] >= burn[0]){
                             	nextGeneration[i][j] = burn[currentGeneration[i][j]];
								break;
							}
					}
				}
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
    
    al_init();
    al_install_keyboard();

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(width*cellSize, height*cellSize);
    ALLEGRO_EVENT event;
    
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    bool redraw = true;
    
	al_init_primitives_addon();

	int** currentGeneration = allocateMatrix();
	int** nextGeneration = allocateMatrix();
	initializeMatrices(currentGeneration, nextGeneration);
    
    ALLEGRO_COLOR color;
    
    while(currentIteration < 5000)
    {
    
        al_peek_next_event(queue, &event);
        
        if((event.type == ALLEGRO_EVENT_KEY_DOWN) || (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE))
            break;

        if(redraw)
        {
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
            redraw = false;
        }

        if(currentIteration % 50 == 0) 
            changeWindDirection();
                  
        transitionFunction(currentGeneration, nextGeneration);
        copyMatrix(currentGeneration, nextGeneration);
        redraw = true;
       
		currentIteration++;
	}
	
    al_destroy_display(disp);  
    al_destroy_event_queue(queue); 
	
	return 0;
}
