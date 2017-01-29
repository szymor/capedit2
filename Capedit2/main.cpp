// edytor plansz do gry Capman

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "resource.h"

#define TILE(x) ((x)-1000)
#define ISWALL(x) ( (((x) >= 1) && ((x) <= 15)) )

char level[21][24];
struct
{
	int play1X;
	int play1Y;
	int play2X;
	int play2Y;
	int play3X;
	int play3Y;
	int play4X;
	int play4Y;
	int bugsX;
	int bugsY;
	int bonus1X;
	int bonus1Y;
	int bonus2X;
	int bonus2Y;
	int bonus3X;
	int bonus3Y;
	int bonus4X;
	int bonus4Y;
	char redAmount;
	char yellowAmount;
	char blueAmount;
	char title[33];
} restOfTheFile;

int currentSelection;

HBITMAP tiles[TILE(SPRITE_BONUS4)+1];
HBITMAP oldtiles[TILE(SPRITE_BONUS4)+1];
HDC tilehdc[TILE(SPRITE_BONUS4)+1];

HWND hMainWindow = NULL;

LRESULT CALLBACK MainWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK OptionsDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void NewLevel(void);
void OpenLevel(const char* filename);
void SaveLevel(const char* filename);
void LoadResources(void);
void UnloadResources(void);
void ValidateTile(int x, int y);
void EraseCapsules(void);
void FillWithCapsules(void);
void GenerateLabyrinth(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	NewLevel();
	LoadResources();

	WNDCLASSEX wce;
	memset(&wce,0,sizeof(wce));
	wce.cbSize = sizeof(wce);
	wce.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wce.hCursor = LoadCursor(NULL,IDC_ARROW);
	wce.hInstance = hInstance;
	wce.lpfnWndProc = MainWindowProc;
	wce.lpszClassName = "Capedit2MainWindowClass";
	wce.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wce.lpszMenuName = MAKEINTRESOURCE(MENU_DEFAULT);
	RegisterClassEx(&wce);

	RECT rect;
	rect.left = rect.top = 100;
	rect.right = 21*24 + 100;
	rect.bottom = 24*24 + 100;
	AdjustWindowRectEx(&rect,WS_CAPTION,TRUE,NULL);

	hMainWindow = CreateWindowEx(0, "Capedit2MainWindowClass", "Capman Level Editor", WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	ShowWindow(hMainWindow,nCmdShow);

	// ustawienie odpowiednim polom w menu flagi MFT_RADIOCHECK
	for( int i = MENUITEM_BONUS4; i >= MENUITEM_EMPTY; --i )
		CheckMenuRadioItem( GetMenu(hMainWindow), MENUITEM_EMPTY, MENUITEM_BONUS4, i, MF_BYCOMMAND );
	currentSelection = MENUITEM_EMPTY;

	MSG msg;
	while( GetMessage(&msg,NULL,NULL,NULL) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnloadResources();

	return msg.wParam;
}

LRESULT CALLBACK MainWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
			case MENUITEM_NEW:
				{
					NewLevel();
					InvalidateRect(hWnd,NULL,FALSE);
					UpdateWindow(hWnd);
				} break;
			case MENUITEM_OPEN:
				{
					OPENFILENAME ofn;
					char openfilename[1024];
					memset(openfilename,0,1024);
					memset(&ofn,0,sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = "Capman Levels\0*.lev\0All files\0*.*";
					ofn.lpstrCustomFilter = NULL;
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = openfilename;
					ofn.nMaxFile = 1024;
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;
					if( GetOpenFileName(&ofn) )
						OpenLevel(ofn.lpstrFile);
					InvalidateRect(hWnd,NULL,FALSE);
					UpdateWindow(hWnd);
				} break;
			case MENUITEM_SAVE:
				{
					OPENFILENAME ofn;
					char savefilename[1024];
					memset(savefilename,0,1024);
					memset(&ofn,0,sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = "Capman Levels\0*.lev\0All files\0*.*";
					ofn.lpstrCustomFilter = NULL;
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = savefilename;
					ofn.nMaxFile = 1024;
					ofn.lpstrDefExt = "lev";
					ofn.Flags = OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
					if( GetSaveFileName(&ofn) )
						SaveLevel(ofn.lpstrFile);
				} break;
			case MENUITEM_ERASE:
				{
					EraseCapsules();
					InvalidateRect(hWnd,NULL,FALSE);
					UpdateWindow(hWnd);
				} break;
			case MENUITEM_FILL:
				{
					FillWithCapsules();
					InvalidateRect(hWnd,NULL,FALSE);
					UpdateWindow(hWnd);
				} break;
			case MENUITEM_OPTIONS:
				{
					DialogBox( GetModuleHandle(NULL), MAKEINTRESOURCE( DIALOG_OPTIONS ), hWnd, OptionsDlgProc );
				} break;
			case MENUITEM_EXIT:
				{
					SendMessage(hWnd,WM_CLOSE,0,0);
				} break;
			case MENUITEM_LABYRINTH:
				{
					GenerateLabyrinth();
					InvalidateRect(hWnd,NULL,FALSE);
					UpdateWindow(hWnd);
				} break;
			case MENUITEM_EMPTY:
			case MENUITEM_CAPSULE:
			case MENUITEM_WALL:
			case MENUITEM_MINE:
			case MENUITEM_PLAYER1:
			case MENUITEM_PLAYER2:
			case MENUITEM_PLAYER3:
			case MENUITEM_PLAYER4:
			case MENUITEM_BUGS:
			case MENUITEM_BONUS1:
			case MENUITEM_BONUS2:
			case MENUITEM_BONUS3:
			case MENUITEM_BONUS4:
				{
					currentSelection = LOWORD(wParam);
					CheckMenuRadioItem( GetMenu(hWnd), MENUITEM_EMPTY, MENUITEM_BONUS4, LOWORD(wParam), MF_BYCOMMAND );
				} break;
			}
			return 0;
		}
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hWnd,&ps);

			for( int x = 0; x < 21; ++x )
				for( int y = 0; y < 24; ++y )
					BitBlt(hdc,x*24,y*24,24,24,tilehdc[ level[x][y] ],0,0,SRCCOPY);
			TransparentBlt(hdc,restOfTheFile.play1X*24,restOfTheFile.play1Y*24,24,24,tilehdc[ TILE( SPRITE_PLAYER1 ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.play2X*24,restOfTheFile.play2Y*24,24,24,tilehdc[ TILE( SPRITE_PLAYER2 ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.play3X*24,restOfTheFile.play3Y*24,24,24,tilehdc[ TILE( SPRITE_PLAYER3 ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.play4X*24,restOfTheFile.play4Y*24,24,24,tilehdc[ TILE( SPRITE_PLAYER4 ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.bugsX*24,restOfTheFile.bugsY*24,24,24,tilehdc[ TILE( SPRITE_BUGS ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.bonus1X*24,restOfTheFile.bonus1Y*24,24,24,tilehdc[ TILE( SPRITE_BONUS1 ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.bonus2X*24,restOfTheFile.bonus2Y*24,24,24,tilehdc[ TILE( SPRITE_BONUS2 ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.bonus3X*24,restOfTheFile.bonus3Y*24,24,24,tilehdc[ TILE( SPRITE_BONUS3 ) ],0,0,24,24,RGB(255,0,255));
			TransparentBlt(hdc,restOfTheFile.bonus4X*24,restOfTheFile.bonus4Y*24,24,24,tilehdc[ TILE( SPRITE_BONUS4 ) ],0,0,24,24,RGB(255,0,255));

			EndPaint(hWnd,&ps);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			SetCapture(hWnd);

			int x, y;
			x = LOWORD(lParam) / 24;
			y = HIWORD(lParam) / 24;

			switch( currentSelection )
			{
			case MENUITEM_EMPTY:
				{
					level[x][y] = TILE( TILE_EMPTY );
					ValidateTile(x-1,y);
					ValidateTile(x+1,y);
					ValidateTile(x,y-1);
					ValidateTile(x,y+1);
				} break;
			case MENUITEM_CAPSULE:
				{
					level[x][y] = TILE( TILE_CAPSULE );
					ValidateTile(x-1,y);
					ValidateTile(x+1,y);
					ValidateTile(x,y-1);
					ValidateTile(x,y+1);
				} break;
			case MENUITEM_WALL:
				{
					level[x][y] = TILE( TILE_P_WALL );
					ValidateTile(x,y);
					ValidateTile(x-1,y);
					ValidateTile(x+1,y);
					ValidateTile(x,y-1);
					ValidateTile(x,y+1);
				} break;
			case MENUITEM_MINE:
				{
					level[x][y] = TILE( TILE_MINE );
					ValidateTile(x-1,y);
					ValidateTile(x+1,y);
					ValidateTile(x,y-1);
					ValidateTile(x,y+1);
				} break;
			case MENUITEM_PLAYER1:
				{
					restOfTheFile.play1X = x;
					restOfTheFile.play1Y = y;
				} break;
			case MENUITEM_PLAYER2:
				{
					restOfTheFile.play2X = x;
					restOfTheFile.play2Y = y;
				} break;
			case MENUITEM_PLAYER3:
				{
					restOfTheFile.play3X = x;
					restOfTheFile.play3Y = y;
				} break;
			case MENUITEM_PLAYER4:
				{
					restOfTheFile.play4X = x;
					restOfTheFile.play4Y = y;
				} break;
			case MENUITEM_BUGS:
				{
					restOfTheFile.bugsX = x;
					restOfTheFile.bugsY = y;
				} break;
			case MENUITEM_BONUS1:
				{
					restOfTheFile.bonus1X = x;
					restOfTheFile.bonus1Y = y;
				} break;
			case MENUITEM_BONUS2:
				{
					restOfTheFile.bonus2X = x;
					restOfTheFile.bonus2Y = y;
				} break;
			case MENUITEM_BONUS3:
				{
					restOfTheFile.bonus3X = x;
					restOfTheFile.bonus3Y = y;
				} break;
			case MENUITEM_BONUS4:
				{
					restOfTheFile.bonus4X = x;
					restOfTheFile.bonus4Y = y;
				} break;
			}
			InvalidateRect(hWnd,NULL,FALSE);
			UpdateWindow(hWnd);

			return 0;
		}
	case WM_LBUTTONUP:
		{
			if( GetCapture() == hWnd)
				ReleaseCapture();
			return 0;
		}
	case WM_MOUSEMOVE:
		{
			if( GetCapture() == hWnd )
			{
				int x, y;
				RECT rect;
				GetClientRect(hWnd,&rect);
				x = LOWORD(lParam);
				y = HIWORD(lParam);

				if( (x < rect.right) && (y < rect.bottom) )		// sprawdzanie dla x i y < 0 nie jest konieczne - x i y po przekroczeniu lewej strony okna beda mialy wartosci ok. 65000
				{
					x /= 24;
					y /= 24;
					switch( currentSelection )
					{
					case MENUITEM_EMPTY:
						{
							level[x][y] = TILE( TILE_EMPTY );
							ValidateTile(x-1,y);
							ValidateTile(x+1,y);
							ValidateTile(x,y-1);
							ValidateTile(x,y+1);
						} break;
					case MENUITEM_CAPSULE:
						{
							level[x][y] = TILE( TILE_CAPSULE );
							ValidateTile(x-1,y);
							ValidateTile(x+1,y);
							ValidateTile(x,y-1);
							ValidateTile(x,y+1);
						} break;
					case MENUITEM_WALL:
						{
							level[x][y] = TILE( TILE_P_WALL );
							ValidateTile(x,y);
							ValidateTile(x-1,y);
							ValidateTile(x+1,y);
							ValidateTile(x,y-1);
							ValidateTile(x,y+1);
						} break;
					case MENUITEM_MINE:
						{
							level[x][y] = TILE( TILE_MINE );
							ValidateTile(x-1,y);
							ValidateTile(x+1,y);
							ValidateTile(x,y-1);
							ValidateTile(x,y+1);
						} break;
					case MENUITEM_PLAYER1:
						{
							restOfTheFile.play1X = x;
							restOfTheFile.play1Y = y;
						} break;
					case MENUITEM_PLAYER2:
						{
							restOfTheFile.play2X = x;
							restOfTheFile.play2Y = y;
						} break;
					case MENUITEM_PLAYER3:
						{
							restOfTheFile.play3X = x;
							restOfTheFile.play3Y = y;
						} break;
					case MENUITEM_PLAYER4:
						{
							restOfTheFile.play4X = x;
							restOfTheFile.play4Y = y;
						} break;
					case MENUITEM_BUGS:
						{
							restOfTheFile.bugsX = x;
							restOfTheFile.bugsY = y;
						} break;
					case MENUITEM_BONUS1:
						{
							restOfTheFile.bonus1X = x;
							restOfTheFile.bonus1Y = y;
						} break;
					case MENUITEM_BONUS2:
						{
							restOfTheFile.bonus2X = x;
							restOfTheFile.bonus2Y = y;
						} break;
					case MENUITEM_BONUS3:
						{
							restOfTheFile.bonus3X = x;
							restOfTheFile.bonus3Y = y;
						} break;
					case MENUITEM_BONUS4:
						{
							restOfTheFile.bonus4X = x;
							restOfTheFile.bonus4Y = y;
						} break;
					}
					InvalidateRect(hWnd,NULL,FALSE);
					UpdateWindow(hWnd);
				}
			}
			return 0;
		}
	case WM_CLOSE:
		{
			if( MessageBox(hWnd,"Are you really sure?","Capman Level Editor",MB_YESNO) == IDYES )
				DestroyWindow(hWnd);
			return 0;
		}

	}
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

BOOL CALLBACK OptionsDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		{
			char aux[3];
			SetDlgItemText(hWnd,DIALOG_ETNAME,restOfTheFile.title);
			sprintf(aux,"%d",restOfTheFile.redAmount);
			SetDlgItemText(hWnd,DIALOG_ETRED,aux);
			sprintf(aux,"%d",restOfTheFile.yellowAmount);
			SetDlgItemText(hWnd,DIALOG_ETYELLOW,aux);
			sprintf(aux,"%d",restOfTheFile.blueAmount);
			SetDlgItemText(hWnd,DIALOG_ETBLUE,aux);
			return TRUE;
		}
	case WM_CLOSE:
		{
			EndDialog(hWnd,0);
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
			case DIALOG_OK:
				{
					int yav;
					char temp[9];
					char maxbugs;
					GetDlgItemText(hWnd,DIALOG_ETNAME,restOfTheFile.title,30);
					GetDlgItemText(hWnd,DIALOG_ETRED,temp,8);
					sscanf(temp,"%d",&yav);
					restOfTheFile.redAmount = yav;
					GetDlgItemText(hWnd,DIALOG_ETYELLOW,temp,8);
					sscanf(temp,"%d",&yav);
					restOfTheFile.yellowAmount = yav;
					GetDlgItemText(hWnd,DIALOG_ETBLUE,temp,8);
					sscanf(temp,"%d",&yav);
					restOfTheFile.blueAmount = yav;
					maxbugs = 24;
					restOfTheFile.redAmount = restOfTheFile.redAmount > maxbugs ? maxbugs : restOfTheFile.redAmount;
					maxbugs -= restOfTheFile.redAmount;
					restOfTheFile.yellowAmount = restOfTheFile.yellowAmount > maxbugs ? maxbugs : restOfTheFile.yellowAmount;
					maxbugs -= restOfTheFile.yellowAmount;
					restOfTheFile.blueAmount = restOfTheFile.blueAmount > maxbugs ? maxbugs : restOfTheFile.blueAmount;
					EndDialog(hWnd,0);
				} break;
			case DIALOG_CANCEL:
				{
					EndDialog(hWnd,0);
				} break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

void NewLevel(void)
{
	memset(&level,TILE(TILE_EMPTY),21*24);
	memset(&restOfTheFile,0,sizeof(restOfTheFile));
}

void OpenLevel(const char* filename)
{
	FILE* file;
	file = fopen(filename,"rb");

	for( int y = 0; y < 24; ++y )
		for( int x = 0; x < 21; ++x )
			fread(&level[x][y],1,1,file);
	fread(&restOfTheFile,sizeof(restOfTheFile),1,file);

	fclose(file);
}

void SaveLevel(const char* filename)
{
	FILE* file;
	file = fopen(filename,"wb");

	for( int y = 0; y < 24; ++y )
		for( int x = 0; x < 21; ++x )
			fwrite(&level[x][y],1,1,file);
	fwrite(&restOfTheFile,sizeof(restOfTheFile),1,file);

	fclose(file);
}

void LoadResources(void)
{
	memset(tiles,0,TILE(SPRITE_BONUS4)+1);
	for( int i = TILE(TILE_EMPTY); i <= TILE(TILE_CAPSULE); ++i )
		tiles[i] = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(i+1000));
	tiles[TILE(TILE_MINE)] = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(TILE_MINE));
	for( int i = TILE(SPRITE_PLAYER1); i <= TILE(SPRITE_BONUS4); ++i )
		tiles[i] = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(i+1000));
	for( int i = TILE(TILE_EMPTY); i < TILE(SPRITE_BONUS4)+1; ++i )
		if( tiles[i] )
		{
			tilehdc[i] = CreateCompatibleDC( GetDC(NULL) );
			oldtiles[i] = (HBITMAP)SelectObject(tilehdc[i],tiles[i]);
		}
		else
		{
			tilehdc[i] = tilehdc[ TILE(TILE_EMPTY) ];
		}
}

void UnloadResources(void)
{
	for( int i = 0; i < TILE(SPRITE_BONUS4)+1; ++i )
		if( tiles[i] )
		{
			SelectObject(tilehdc[i],oldtiles[i]);
			DeleteObject(tiles[i]);
			DeleteDC(tilehdc[i]);
		}
}

void ValidateTile(int x, int y)
{
	char walltype;

	if( (x < 0) || (x > 20) || (y < 0) || (y > 23) || (level[x][y] < TILE( TILE_UL_WALL )) || (level[x][y] > TILE( TILE_P_WALL )) )
		return;

	walltype = TILE( TILE_P_WALL );

	// na lewo
	if( (x > 0) && ISWALL( level[x-1][y] ) )
	{
		walltype = TILE( TILE_R_WALL );
	}

	// na prawo
	if( (x < 20) && ISWALL( level[x+1][y] ) )
	{
		if( walltype == TILE( TILE_R_WALL ) )
			walltype = TILE( TILE_H_WALL );
		else
			walltype = TILE( TILE_L_WALL );
	}

	// na gorze
	if( (y > 0) && ISWALL( level[x][y-1] ) )
	{
		switch( walltype )
		{
		case TILE( TILE_R_WALL ):
			walltype = TILE( TILE_DR_WALL ); break;
		case TILE( TILE_L_WALL ):
			walltype = TILE( TILE_DL_WALL ); break;
		case TILE( TILE_H_WALL ):
			walltype = TILE( TILE_TU_WALL ); break;
		case TILE( TILE_P_WALL ):
			walltype = TILE( TILE_D_WALL ); break;
		}
	}

	// na dole
	if( (y < 23) && ISWALL( level[x][y+1] ) )
	{
		switch( walltype )
		{
		case TILE( TILE_R_WALL ):
			walltype = TILE( TILE_UR_WALL ); break;
		case TILE( TILE_L_WALL ):
			walltype = TILE( TILE_UL_WALL ); break;
		case TILE( TILE_H_WALL ):
			walltype = TILE( TILE_TD_WALL ); break;
		case TILE( TILE_D_WALL ):
			walltype = TILE( TILE_V_WALL ); break;
		case TILE( TILE_DR_WALL ):
			walltype = TILE( TILE_TL_WALL ); break;
		case TILE( TILE_DL_WALL ):
			walltype = TILE( TILE_TR_WALL ); break;
		case TILE( TILE_TU_WALL ):
			walltype = TILE( TILE_P_WALL ); break;
		case TILE( TILE_P_WALL ):
			walltype = TILE( TILE_U_WALL ); break;
		}
	}

	level[x][y] = walltype;
}

void EraseCapsules(void)
{
	for( int x = 0; x < 21; ++x )
		for( int y = 0; y < 24; ++y )
			if( level[x][y] == TILE( TILE_CAPSULE ) )
				level[x][y] = TILE( TILE_EMPTY );
}

void FillWithCapsules(void)
{
	for( int x = 0; x < 21; ++x )
		for( int y = 0; y < 24; ++y )
			if( level[x][y] == TILE( TILE_EMPTY ) )
				level[x][y] = TILE( TILE_CAPSULE );
}

int LabHelper(int x, int y, int mode)
{
	// 0 - call LabHelper for all surrounding tiles
	// 1 - call LabHelper for one random-chosen adjacent tile
	// and there is slight chance to call it for more tiles
	int neighbours;
	int order[4];
	int aux;

	switch( mode )
	{
	case 0:
		{
			level[x][y] = TILE( TILE_EMPTY );
			LabHelper(x-1,y,1);
			LabHelper(x,y-1,1);
			LabHelper(x+1,y,1);
			LabHelper(x,y+1,1);
		} break;
	case 1:
		{
			if( (x == 20) || (y == 23) || (x == 0) || (y == 0) || (level[x][y] == TILE( TILE_EMPTY )) )
				return 1;
			neighbours = 0;
			neighbours += (level[x-1][y] == TILE( TILE_EMPTY )) ? 1 : 0;
			neighbours += (level[x][y-1] == TILE( TILE_EMPTY )) ? 1 : 0;
			neighbours += (level[x+1][y] == TILE( TILE_EMPTY )) ? 1 : 0;
			neighbours += (level[x][y+1] == TILE( TILE_EMPTY )) ? 1 : 0;
			if( neighbours == 1 )
			{
				level[x][y] = TILE( TILE_EMPTY );

				order[0] = rand() % 4;

				aux = rand() % 3;
				order[1] = (aux >= order[0]) ? aux + 1: aux;

				aux = rand() % 2;
				if( aux >= order[0] )
					if( aux >= order[1] )
						order[2] = aux + 2;
					else
						order[2] = aux + 1;
				else
					if( aux >= order[1] )
						order[2] = aux + 1;
					else
						order[2] = aux;

				order[3] = -1;
				do
				{
					++order[3];
					for( int i = 0; i < 4; ++i )
						if( order[3] == order[i] )
							break;
					break;
				} while(1);

				for( int i = 0; i < 4; ++i )
				{
					switch( order[i] )
					{
					case 0: LabHelper(x-1,y,1); break;
					case 1: LabHelper(x+1,y,1); break;
					case 2: LabHelper(x,y-1,1); break;
					case 3: LabHelper(x,y+1,1); break;
					}
				}
			}
		}
	}
	return 0;
}

void GenerateLabyrinth(void)
{
	srand( (unsigned int) time(NULL) );

	// initially fill everything with walls
	for( int x = 0; x < 21; ++x )
		for( int y = 0; y < 24; ++y )
			level[x][y] = TILE( TILE_P_WALL );

	// 'dig up' a level shape
	LabHelper( rand() % 10 + 5, rand() % 12 + 6, 0);

	// correct wall shapes
	for( int x = 0; x < 21; ++x )
		for( int y = 0; y < 24; ++y )
			ValidateTile(x,y);
}
