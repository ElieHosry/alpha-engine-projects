// Game of life assignment solution
#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"

#define GOL_GRID_COLS (1600/20)
#define GOL_GRID_ROWS (900/20)
#define GOL_GRID_BUFFERS 2

#define GOL_ALIVE 1
#define GOL_DEAD 0

#define TRUE 1
#define FALSE 0


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	/* Initialize the grid */
	int gGrids[GOL_GRID_BUFFERS][GOL_GRID_ROWS][GOL_GRID_COLS];

	/* Set every grids' cells as 'dead' */
	for (int row = 0; row < GOL_GRID_ROWS; ++row) {
		for (int col = 0; col < GOL_GRID_COLS; ++col) {
			for (int i = 0; i < GOL_GRID_BUFFERS; ++i) {
				gGrids[i][row][col] = GOL_DEAD;
			}
		}
	}

	/*********************************************************
	*  Let's start with gGrids[0] as the 'reference grid'
	*  and gGrids[1] as the 'displaying grid'.
	*
	*  We initialize gGrids[0] with a simple 'glider'.
	*********************************************************/

	gGrids[0][1][2] = GOL_ALIVE;
	gGrids[0][2][3] = GOL_ALIVE;
	gGrids[0][3][1] = GOL_ALIVE;
	gGrids[0][3][2] = GOL_ALIVE;
	gGrids[0][3][3] = GOL_ALIVE;

	/* We start unpaused */
	int gIsPaused = FALSE;

	/* Initialization of your other variables here */
	int gDisplayingGridIndex = 0;


	int gGameRunning = 1;

	// Initialization of your own variables go here

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, NULL);

	// Changing the window title
	AESysSetWindowTitle("GOL Demo!");



	s8 pFont = AEGfxCreateFont("Assets/liberation-mono.ttf", 72);

	// Text to print
	const char* pText = "PAUSED";

	f32 w, h;
	AEGfxGetPrintSize(pFont, pText, 1.f, &w, &h);


	// Informing the library that we're about to start adding triangles
	AEGfxMeshStart();

	// This shape has 2 triangles that makes up a square
	// Color parameters represent colours as ARGB
	// UV coordinates to read from loaded textures
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF000000, 0.0f, 1.0f,  // bottom-left: red
		0.5f, -0.5f, 0xFF000000, 1.0f, 1.0f,   // bottom-right: green
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);  // top-left: blue

	AEGfxTriAdd(
		0.5f, -0.5f, 0xFF000000, 1.0f, 1.0f,   // bottom-right: green
		0.5f, 0.5f, 0xFF000000, 1.0f, 0.0f,    // top-right: white
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);  // top-left: blue

	// Saving the mesh (list of triangles) in pMesh
	AEGfxVertexList* pMesh = AEGfxMeshEnd();
	AEGfxTexture* pTex = AEGfxTextureLoad("Assets/border.png");

	// Game Loop
	while (gGameRunning)
	{
		/*=== INPUT START ========================================*/
		if (AEInputCheckTriggered(AEVK_SPACE)) {
			gIsPaused = !gIsPaused;
		}

		if (gIsPaused) {
			if (AEInputCheckTriggered(AEVK_LBUTTON)) {
				s32 concreteMouseX, concreteMouseY;
				AEInputGetCursorPosition(&concreteMouseX, &concreteMouseY);
				float mouseX = (float)concreteMouseX;
				float mouseY = (float)concreteMouseY;

				float windowW = (float)AEGfxGetWindowWidth();
				float windowH = (float)AEGfxGetWindowHeight();

				float ratioW = GOL_GRID_COLS / windowW;
				float ratioH = GOL_GRID_ROWS / windowH;

				int cellCol = (int)(ratioH * mouseX);
				int cellRow = (int)(ratioW * mouseY);

				if (cellRow < GOL_GRID_ROWS && cellCol < GOL_GRID_COLS) {
					gGrids[gDisplayingGridIndex][cellRow][cellCol] =
						!gGrids[gDisplayingGridIndex][cellRow][cellCol];
				}
			}

		}

		/*=== INPUT END ==========================================*/



		/*=== UPDATE START =======================================*/


		if (gIsPaused == FALSE) {
			for (int row = 0; row < GOL_GRID_ROWS; ++row) {
				for (int col = 0; col < GOL_GRID_COLS; ++col) {

					/* Check neighbours */
					int currentCellState = gGrids[gDisplayingGridIndex][row][col];
					int survivors = 0;

					for (int neighbourRow = 0; neighbourRow < 3; ++neighbourRow) {
						int rowToCheck = row - 1 + neighbourRow;

						// Reject invalid row
						if (rowToCheck < 0 || rowToCheck >= GOL_GRID_ROWS) {
							continue;
						}

						for (int neighbourCol = 0; neighbourCol < 3; ++neighbourCol)
						{
							int colToCheck = col - 1 + neighbourCol;
							if (colToCheck < 0 || colToCheck >= GOL_GRID_COLS) {
								continue;
							}

							// ignore middle cell
							if (rowToCheck == row && colToCheck == col) {
								continue;
							}

							if (gGrids[gDisplayingGridIndex][rowToCheck][colToCheck] == GOL_ALIVE)
							{
								++survivors;
							}
						}

					}

					/* Update the next step to the other grid*/
					int otherGridId = (gDisplayingGridIndex + 1) % GOL_GRID_BUFFERS;
					gGrids[otherGridId][row][col] = currentCellState;

					/* Case for living cell */
					if (currentCellState == GOL_ALIVE) {
						/* If underpopulation or overpopulation */
						if (survivors < 2 || survivors > 3) {
							/* Update to die */
							gGrids[otherGridId][row][col] = GOL_DEAD;
						}
					}
					/* Case for dead cell */
					else {
						/* If exactly 3, then repopulation occurs */
						if (survivors == 3) {
							gGrids[otherGridId][row][col] = GOL_ALIVE;
						}
					}
				}
			}
			++gDisplayingGridIndex;
			gDisplayingGridIndex %= GOL_GRID_BUFFERS;

		}

		/* === UPDATE END ==================================*/

		/* === RENDER START ================================*/
		AESysFrameStart();


		/* RENDER background */
		AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);


		// Tell the engine to get ready to draw something with color.
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);

		// Set the the color to multiply to white, so that the sprite can 
		// display the full range of colors (default is black).
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);


		// Set blend mode to AE_GFX_BM_BLEND
		// This will allow transparency.
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		/* RENDER grid */

		float screenWidth = (float)AEGfxGetWindowWidth();
		float screenHeight = (float)AEGfxGetWindowHeight();
		float gridWidth = screenWidth / GOL_GRID_COLS;
		float gridHeight = screenHeight / GOL_GRID_ROWS;

		float offsetX = 0;
		float offsetY = screenHeight - gridHeight;
		for (int row = 0; row < GOL_GRID_ROWS; ++row) {
			for (int col = 0; col < GOL_GRID_COLS; ++col) {
				int currentCell = gGrids[gDisplayingGridIndex][row][col];
				if (currentCell == GOL_ALIVE) {
					AEGfxSetColorToAdd(1.0f, 1.0f, 1.0f, 0.0f);
				}
				else {
					AEGfxSetColorToAdd(0.5f, 0.5f, 0.5f, 0.0f);
				}

				AEMtx33 transform;
				AEMtx33Identity(&transform);

				AEMtx33 scale;
				AEMtx33Scale(&scale, gridWidth * 0.9f, gridHeight * 0.9f);

				AEMtx33 translate;
				AEMtx33Trans(&translate, offsetX - screenWidth / 2 + gridWidth / 2, offsetY - screenHeight / 2 + gridHeight / 2);

				AEMtx33Concat(&transform, &translate, &scale);
				AEGfxSetTransform(transform.m);

				AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
				offsetX += gridWidth;
			}
			offsetY -= gridHeight;
			offsetX = 0;

		}

		// Render paused stuff
		if (gIsPaused) {
			AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
			AEGfxSetColorToAdd(0, 0, 0, 1);

			AEMtx33 scale = { 0 };
			AEMtx33Scale(&scale, 1600, 900.f);
			AEGfxSetTransform(scale.m);

			AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

			AEGfxTextureSet(pTex, 0, 0);

			AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
			AEGfxPrint(pFont, pText, -w / 2, -h / 2, 1, 1, 1, 1, 1);
		}




		// Informing the system about the loop's end
		AESysFrameEnd();

		/*=== RENDER END =======================================*/


		// check if forcing the application to quit
		if (0 == AESysDoesWindowExist())
			gGameRunning = 0;


	}
	AEGfxTextureUnload(pTex);
	AEGfxMeshFree(pMesh);
	AESysExit();

}