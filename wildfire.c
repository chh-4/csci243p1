/* wildfire.c
 * Author: CHARLES HENRY HUTSON IV
 * Date: OCTOBER 23 2024
 *
 * Simulates the spread of fire on a 2D array
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "display.h"

// Cell state constants
#define EMPTY ' '
#define TREE 'Y'
#define BURNING '*'
#define BURNED '.'

// Default argument values
#define DEFB 10 // Default bN N value
#define DEFC 30	// Default cN N value
#define DEFD 50 // Default dN N value
#define DEFN 25 // Default nN N value
#define DEFP -1 // Default pN N value, -1 is overlay mode
#define DEFS 10 // Default sN N value

// Macro for strtol Optarg To Integer (OTI)
char *endptr;
#define OTI(optarg) strtol(optarg, &endptr, 10)

// How many cycles trees should burn for (including the inital cycle)
#define BURNCYC 3

// Checks if a cell is a tree or burning
#define CELLCHECK(cell)				\
		if ((cell) == TREE) {		\
			neigh_trees++;		\
		}				\
		else if ((cell) == BURNING) {	\
			neigh_burn++;		\
		}				\

// The current changes
int cur_changes = 0;
// The total changes
int total_changes = 0;

/// Function to print the grid
///
/// @param sN size of the grid
/// @param grid the grid to print
/// @param cN catch fire percentage
/// @param dN density percentage
/// @param bN burning percentage
/// @param nN neighbor effect percentage
/// @param cyc_since cycles since start
static void p_grid(int sN, char grid[sN][sN], int cN, int dN, int bN, int nN, int cyc_since) {
	total_changes += cur_changes;

	if ((cur_changes != 0) || (total_changes == 0)) {
		for (int r = 0; r < sN; r++) {
		       	for (int c = 0; c < sN; c++) {
		            	printf("%c", grid[r][c]);
		     	}
		      	printf("\n");
		}

		printf("size %d, pCatch %.2f, density %.2f, pBurning %.2f, pNeighbor %.2f\n"
			"cycle %d, current changes %d, cumulative changes %d.\n", sN, ((double)cN / 100), ((double)dN / 100), ((double)bN / 100), ((double)nN / 100), cyc_since, cur_changes, total_changes
			);
		cur_changes = 0;
	}
}

/// Spread locally decides to change the states of neighbors
///
/// @param cN catch fire percentage
/// @param nN neighbor effect percentage
/// @param sN size of the grid
/// @param grid the grid
/// @param grid_copy a copy of the grid
/// @param burn_rem remaining burn cycles
/// @param r row index
/// @param c column index
/// @return EXIT_SUCCESS
static int spread(int cN, int nN, int sN, char grid[sN][sN], char grid_copy[sN][sN], int burn_rem[sN][sN], int r, int c) {
	int neigh_trees = 0; // Amount of neighboring trees
	int neigh_burn = 0; // Amount of neighboring trees burning

	if (grid_copy[r][c] == EMPTY) {
		neigh_trees = -1; // -1 shows that there is an empty space present at this location, primarily for debug purposes
	}
	else if (grid_copy[r][c] == TREE) {
		// Center
		if ((r != 0) && (r != (sN - 1)) && (c != 0) && (c != (sN - 1))) {
			CELLCHECK(grid[r + 1][c]);
			CELLCHECK(grid[r + 1][c + 1]);
			CELLCHECK(grid[r + 1][c - 1]);
			CELLCHECK(grid[r][c + 1]);
			CELLCHECK(grid[r][c - 1]);
			CELLCHECK(grid[r - 1][c]);
			CELLCHECK(grid[r - 1][c + 1]);
			CELLCHECK(grid[r - 1][c - 1]);
		}
		// Northern edge
		else if ((r == 0) && (c != 0) && (c != (sN - 1))) {
			CELLCHECK(grid[r][c - 1]);
			CELLCHECK(grid[r][c + 1]);
			CELLCHECK(grid[r + 1][c - 1]);
			CELLCHECK(grid[r + 1][c]);
			CELLCHECK(grid[r + 1][c + 1]);
		}
		// North-Eastern edge
		else if ((r == 0) && (c == (sN - 1))) {
			CELLCHECK(grid[r][c - 1]);
			CELLCHECK(grid[r + 1][c - 1]);
			CELLCHECK(grid[r + 1][c]);
		}
		// Eastern edge
		else if ((c == (sN - 1)) && (r != 0) && (r != (sN - 1))) {
			CELLCHECK(grid[r + 1][c - 1]);
			CELLCHECK(grid[r][c - 1]);
			CELLCHECK(grid[r - 1][c - 1]);
			CELLCHECK(grid[r + 1][c]);
			CELLCHECK(grid[r - 1][c]);
		}
		// South-Eastern edge
		else if ((r == (sN - 1)) && (c == (sN - 1))) {
			CELLCHECK(grid[r - 1][c - 1]);
			CELLCHECK(grid[r][c - 1]);
			CELLCHECK(grid[r - 1][c]);
		}
		// Southern edge
		else if ((c == (sN - 1)) && (r != 0) && (r != (sN - 1))) {
			CELLCHECK(grid[r - 1][c - 1]);
			CELLCHECK(grid[r - 1][c]);
			CELLCHECK(grid[r - 1][c + 1]);
			CELLCHECK(grid[r][c + 1]);
			CELLCHECK(grid[r][c - 1]);
		}
		// South-Western edge
		else if ((c == 0) && (r == (sN - 1))) {
			CELLCHECK(grid[r - 1][c]);
			CELLCHECK(grid[r - 1][c + 1]);
			CELLCHECK(grid[r][c + 1]);
		}
		// Western edge
		else if ((c == 0) && (r != 0) && (r != (sN - 1))) {
			CELLCHECK(grid[r - 1][c]);
			CELLCHECK(grid[r - 1][c + 1]);
			CELLCHECK(grid[r][c + 1]);
			CELLCHECK(grid[r + 1][c]);
			CELLCHECK(grid[r + 1][c + 1]);
		}
		// North-Western edge
		else if ((c == 0) && (r == 0)) {
			CELLCHECK(grid[r + 1][c]);
			CELLCHECK(grid[r][c + 1]);
			CELLCHECK(grid[r + 1][c + 1]);
		}
	}

	int totalTrees = neigh_trees + neigh_burn;
	if (totalTrees > 0) {
		double propTBurning = ((double)neigh_burn / totalTrees); // The percentage of neighboring trees that are burning
		double nPer = ((double)nN / 100); // nN in decimal percentage format
		double cPer = ((double)cN / 100); // cN in decimal percentage format
		if (propTBurning > nPer) {
			if (cPer > ((double)rand() / RAND_MAX)) {
				grid[r][c] = BURNING;
				burn_rem[r][c] = 1; // Start the burn cycle
				cur_changes++;
			}
		}
	}
	return EXIT_SUCCESS;
}


/// Update applies the spread function to each grid cell and modifies the grid accordingly
///
/// @param cN catch fire percentage
/// @param nN neighbor effect percentage
/// @param sN size of the grid
/// @param grid the grid
/// @param burn_rem remaining burn cycles
/// @return number of burning trees
static int update(int cN, int nN, int sN, char grid[sN][sN], int burn_rem[sN][sN]) {
	int burning = 0;

	char grid_copy[sN][sN];
	for (int r = 0; r < sN; r++) {
		for (int c = 0; c < sN; c++) {
			grid_copy[r][c] = grid[r][c];
		}
	}

    	for (int r = 0; r < sN; r++) {
        	for (int c = 0; c < sN; c++) {
            		if (grid[r][c] == BURNING) {
                	burning++;
            		}
        	}
    	}

    	for (int r = 0; r < sN; r++) {
        	for (int c = 0; c < sN; c++) {
            		if (grid[r][c] == BURNING) {
                		burn_rem[r][c]++;
                		if (burn_rem[r][c] > BURNCYC) {
                    			grid[r][c] = BURNED;
					cur_changes++;
                		}
            		}
        	}
    	}

	for (int r = 0; r < sN; r++) {
        	for (int c = 0; c < sN; c++) {
            		spread(cN,nN, sN, grid, grid_copy, burn_rem, r, c);
        	}
    	}

    	return burning;
}

/// Sim executes a loop that continually calls the update operation, then tracks the changes, then checks whether all fires are out or # pN cycles
///
/// @param bN burning percentage
/// @param cN catch fire percentage
/// @param dN density percentage
/// @param nN neighbor effect percentage
/// @param pN print mode cycles
/// @param sN size of the grid
/// @param burning initial number of burning trees
/// @param grid the grid
/// @return EXIT_SUCCESS
static int sim(int bN, int cN, int dN, int nN, int pN, int sN, int burning, char grid[sN][sN]) {
	int mode; // 0 = overlay mode (default), 1 = print mode
	int fires = burning; // The amount of trees currently on fire
	int cycles = pN; // Amount of cycles remaining
	int cyc_since = 0;

	int burn_rem[sN][sN]; // Amount of burning remaining (in cycles)
	for (int r = 0; r < sN; r++) {
		for (int c = 0; c < sN; c++) {
			burn_rem[r][c] = 0; // 0 as no trees are burning quite yet
		}
	}
	for (int r = 0; r < sN; r++) {
		for (int c = 0; c < sN; c++) {
			if (grid[r][c] == BURNING) {
				burn_rem[r][c] = 1;
			}
		}
	}

	if (cycles == -1) {
		//printf("Overlay mode\n");
		mode = 0;
	}
	else if (cycles >= 0) {
		//printf("Print mode\n");
		mode = 1;
		printf( "===========================\n"
		 	"======== Wildfire =========\n"
			"===========================\n"
			"=== Print %d Time Steps ===\n"
			"===========================\n", pN);
		p_grid(sN, grid, cN, dN, bN, nN, cyc_since);
	}
	if (fires == 0) {
		//printf("Fires are out.\n\n");
		return EXIT_SUCCESS;
	}
	if (cycles == 0) {
		//printf("Cycles completed.\n\n");
		return EXIT_SUCCESS;
	}

	while (fires > 0) {
		if (mode == 0) {
			clear();
			set_cur_pos(0, 0);
			p_grid(sN, grid, cN, dN, bN, nN, cyc_since);
			usleep(750000);
			fires = update(cN, nN, sN, grid, burn_rem);
			cycles--;
			cyc_since++;
		}
		else if (mode == 1 && cycles > 0) {
			fires = update(cN, nN, sN, grid, burn_rem);
			cycles--;
			cyc_since++;
			p_grid(sN, grid, cN, dN, bN, nN, cyc_since); // Print the grid after each update
		}

		if (fires == 0) {
			printf("Fires are out.\n\n");
			break;
		}
		if (cycles == 0) {
			//printf("Cycles completed.\n\n");
			break;
		}
	}

	return EXIT_SUCCESS;
}

/// Init creates the grid, counts # trees, counts # burning trees, fills the grid, and then randomizes the cells of the grid
///
/// @param bN burning percentage
/// @param cN catch fire percentage
/// @param dN density percentage
/// @param nN neighbor effect percentage
/// @param pN print mode cycles
/// @param sN size of the grid
/// @return EXIT_SUCCESS
static int init(int bN, int cN, int dN, int nN, int pN, int sN) {
	char grid[sN][sN];
	int fsize = (sN * sN); // Size of whole forest or grid
	char forest[fsize];

	int density = (fsize * ((float)dN / 100)); // Number of trees in the grid
	int burning = (density * ((float)bN / 100)); // Number of trees initially burning
	int trees = (density - burning); // Number of trees initially not on fire
	int bc = burning; // Copy of inital burn number

	// Fill the grid with empty space
	for (int r = 0; r < sN; r++) {
		for (int c = 0; c < sN; c++) {
			grid[r][c] = EMPTY;
		}
	}

	// Place the first x trees, then next y burning trees
	for (int i = 0; i < fsize; i++) {
		forest[i] = EMPTY;
	}
	for (int r = 0; r < sN; r++) {
		for (int c = 0; c < sN; c++) {
			if (trees > 0) {
				forest[density - trees] = TREE;
				trees--;
			}
			else if ((trees == 0) && (burning > 0)) {
				forest[burning - 1] = BURNING;
				burning--;
			}
		}
	}

	// Fisher Yates Shuffle
	srandom(41); // 41 must be seed per assignment

	for (int i = fsize - 1; i > 0; i--) {
		int j = random() % (1 + i); // Modulo ensures j is between 0 and i
		char temp = forest[i];
		forest[i] = forest[j];
		forest[j] = temp;
	}

	int fcount = fsize;
	for (int r = 0; r < sN; r++) {
                for (int c = 0; c < sN; c++) {
			grid[r][c] = forest[fcount - 1];
			fcount--;
                }
        }

	sim(bN, cN, dN, nN, pN, sN, bc, grid);

	return EXIT_SUCCESS;
}

/// Main processes and validates the command line arguments
///
/// @param argc argument count
/// @param argv argument vector
/// @return EXIT_SUCCESS
int main(int argc, char *argv[]) {
	int bN = DEFB; // Initial tree burning percentage
	int cN = DEFC; // Tree catch fire percent chance
	int dN = DEFD; // Density: percentage of cells that contain trees
	int nN = DEFN; // Neighbor effect: how many neighboring trees does it take for one to start burning
	int pN = DEFP; // Print mode cycles. -1 = overlay mode. Any # >= 0 enables print mode and is the amount of cycles 
	int sN = DEFS; // Grid size. One side's length

	int arg;
	while ((arg = getopt(argc, argv, "Hb:c:d:n:p:s:")) != -1) {
		for (int i = 1; i < argc; i++) {
			char ca = argv[i][1]; // Current argument
			// Good args: H, b, c, d, n, p, s
			if (ca != 'H' && ca != 'b' && ca != 'c' && ca != 'd' && ca != 'n' && ca != 'p' && ca != 's') {
				printf(
				"Unknown flag \"%c\"\n"
				"\n"
				"usage: wildfire [options]\n"
				"By default, the simulation runs in overlay display mode.\n"
				"The -pN option makes the simulation run in print mode for up to N states.\n"
				"\n"
				"Simulation Configuration Options:\n"
				"-H  # View simulation options and quit.\n"
				"-bN # proportion of trees that are already burning. 0 < N < 101.\n"
				"-cN # probability that a tree will catch fire. 0 < N < 101.\n"
				"-dN # density: the proportion of trees in the grid. 0 < N < 101.\n"
				"-nN # proportion of neighbors that influence a tree catching fire. -1 < N < 101.\n"
				"-pN # number of states to print before quitting. -1 < N < ...\n"
				"-sN # simulation grid size. 4 < N < 41.\n", ca
				);
				return EXIT_FAILURE;
			}
		}

		if (arg == 'H') { // Program will display help information
			printf(
			"usage: wildfire [options]\n"
			"By default, the simulation runs in overlay display mode.\n"
			"The -pN option makes the simulation run in print mode for up to N states.\n"
			"\n"
			"Simulation Configuration Options:\n"
			"-H  # View simulation options and quit.\n"
			"-bN # proportion of trees that are already burning. 0 < N < 101.\n"
			"-cN # probability that a tree will catch fire. 0 < N < 101.\n"
			"-dN # density: the proportion of trees in the grid. 0 < N < 101.\n"
			"-nN # proportion of neighbors that influence a tree catching fire. -1 < N < 101.\n"
			"-pN # number of states to print before quitting. -1 < N < ...\n"
			"-sN # simulation grid size. 4 < N < 41.\n"
			);
			return EXIT_SUCCESS; // Help is an exclusive option
		}

		else if (arg == 'b') { // Min 1 Max 100
			int tbN = OTI(optarg);
			if (tbN >= 1 && tbN <= 100) {
				bN = tbN;
			}
			else {
				printf("(-bN) proportion already burning must be an integer in [1...100].\n");
				return EXIT_FAILURE;
			}
            	}

		else if (arg == 'c') { // Min 1 Max 100
			int tcN = OTI(optarg);
			if (tcN >= 1 && tcN <= 100) {
				cN = tcN;
			}
			else {
				printf("(-cN) probability a tree will catch fire must be an integer in [1...100].\n");
				return EXIT_FAILURE;
			}
                }

		else if (arg == 'd') { // Min 1 Max 100
			int tdN = OTI(optarg);
			if (tdN >= 1 && tdN <= 100) {
				dN = tdN;			}
			else {
				printf("(-dN) density of trees in the grid must be an integer in [1...100].\n");
				return EXIT_FAILURE;
			}
                }

		else if (arg == 'n') { // Min 0 Max 100
			int tnN = OTI(optarg);
			if (tnN >= 0 && tnN <= 100) {
				nN = tnN;
			}
			else {
				printf("(-nN) %%neighbors influence catching fire must be an integer in [0...100].\n");
				return EXIT_FAILURE;
			}
                }

		else if (arg == 'p') { // Min 0 Max 10000
			int tpN = OTI(optarg);
			if (tpN >= 0 && tpN <= 10000) {
				pN = tpN;
			}
			else {
				printf("(-pN) number of states to print must be an integer in [0...10000].\n");
				return EXIT_FAILURE;
			}
                }

		else if (arg == 's') { // Min 5 Max 40
			int tsN = OTI(optarg);
			if (tsN >= 5 && tsN <= 40) {
				sN = tsN;
			}
			else {
				printf("(-sN) simulation grid size must be an integer in [5...40].\n");
				return EXIT_FAILURE;
			}
                }
	}

	init(bN, cN, dN, nN, pN, sN);

	return EXIT_SUCCESS;
}
