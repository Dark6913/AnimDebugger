#include "resource.h"
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "Animation.hpp"
#include <vector>

#define CM_INCREASE			(HMENU)0x70
#define CM_DECREASE			(HMENU)0x71
#define CM_LOADANIM			(HMENU)0x73

#define CM_MOVEPTOP			(HMENU)0x74
#define CM_MOVEPBOT			(HMENU)0x75
#define CM_MOVEPLEFT		(HMENU)0x76
#define CM_MOVEPRIGHT		(HMENU)0x77

#define CM_PLAY				(HMENU)0x78
#define CM_STOP 			(HMENU)0x79


sf::Vector2f operator*(sf::Vector2f a, sf::Vector2f b)
{
	return sf::Vector2f(a.x * b.x, a.y * b.y);
}

struct RowPreset
{
	std::wstring title;
	uint8_t rows_count;
};

const RowPreset ROWS_PRESETS[] = {
	RowPreset{L"Рух", 6},
	RowPreset{L"Атака", 4},
	RowPreset{L"Смерть", 2},
	RowPreset{L"Магія", 1}
};

sf::Texture texture;
sf::Sprite* tileset = NULL;
Animation animation;
sf::RectangleShape pointer;

const wchar_t CLASS_NAME[] = L"Anim Debugger Class";
HBRUSH hBackgorundBrush;
HWND hFileEdit;
HWND hFramesEdit;
HWND hRowsCombo;
HWND hPSpeedEdit;
HWND hLoadButton;

HWND hFrameStatic;
HWND hRowStatic;
HWND hModeStatic;

bool is_play_mode;
std::vector<HWND> control_list;
HWND hPlayButton;
HWND hStopButton;

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateWidgets(HWND hWnd);
void UpdatePointer();

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	SetProcessDPIAware();
	hBackgorundBrush = CreateSolidBrush(RGB(0, 127, 0));
	is_play_mode = false;

	WNDCLASS wc = {};
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WinProc;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = hBackgorundBrush;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	RegisterClass(&wc);

	HWND hWnd = CreateWindow(
		CLASS_NAME, L"Anim Debugger", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1600, 800,
		NULL, NULL, hInstance, NULL
	);

	RECT rc;
	GetClientRect(hWnd, &rc);
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	HWND hRenderView = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 300 , 0, cx - 300, cy, hWnd, 0, hInstance, 0);
	sf::RenderWindow render_window(hRenderView);

	sf::RectangleShape splitter(sf::Vector2f(2, cy));
	splitter.setFillColor(sf::Color(0, 0, 0));
	splitter.setPosition(sf::Vector2f(0, 0));

	animation.setScale(sf::Vector2f(4, 4));

	pointer.setFillColor(sf::Color(0, 0, 0, 0));
	pointer.setOutlineColor(sf::Color(255, 0, 0));
	pointer.setOutlineThickness(-2);

	ShowWindow(hWnd, nCmdShow);

	sf::Clock clock;
	MSG msg;
	msg.message = ~WM_QUIT;
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			float tick = clock.restart().asSeconds();
			render_window.clear(sf::Color(0, 127, 0));

			if (is_play_mode)
			{
				animation.playLoopInStraightOrder(tick);
				UpdatePointer();
			}

			render_window.draw(splitter);
			if (tileset) render_window.draw(*tileset);
			render_window.draw(pointer);
			render_window.draw(animation);
			
			render_window.display();
		}
	}

	DestroyWindow(hWnd);
	UnregisterClass(CLASS_NAME, hInstance);
	DeleteObject(hBackgorundBrush);
	if (tileset) delete tileset;
	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	if (uMsg == WM_COMMAND)
	{
		HMENU hMenu = (HMENU)wParam;
		if (hMenu == CM_INCREASE)
		{
			std::wstring text;
			GetWindowText(hFramesEdit, (LPWSTR)text.c_str(), 1024);

			if (!wcscmp(text.c_str(), L"")) text = L"0";

			int frames_cnt = std::stoi(text, nullptr, 10);
			text = std::to_wstring(++frames_cnt);
			SetWindowText(hFramesEdit, (LPWSTR)text.c_str());
		}
		else if (hMenu == CM_DECREASE)
		{
			std::wstring text;
			GetWindowText(hFramesEdit, (LPWSTR)text.c_str(), 1024);
			int frames_cnt = std::stoi(text, nullptr, 10);
			if (frames_cnt > 1)
			{
				text = std::to_wstring(--frames_cnt);
				SetWindowText(hFramesEdit, (LPWSTR)text.c_str());
			}
		}
		else if (hMenu == CM_LOADANIM)
		{
			LPWSTR buffer = new WCHAR[2048];
			ZeroMemory(buffer, 2048 * sizeof(WCHAR));
			GetWindowText(hFileEdit, buffer, 2048);
			std::wstring file_name = buffer;
			delete[] buffer;

			if (!wcscmp(file_name.c_str(), L""))
			{
				MessageBox(hWnd, L"Поле для вводу шляху до файлу не може бути пустим!", L"Помилка", MB_ICONERROR);
				return 0;
			}

			HANDLE hFile = CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				MessageBox(hWnd, L"Помилка відкриття файлу!", L"Помилка", MB_ICONERROR);
				return 0;
			}

			DWORD dwFileSize = GetFileSize(hFile, NULL);
			LPBYTE lpFileData = new BYTE[dwFileSize];
			if (!ReadFile(hFile, lpFileData, dwFileSize, NULL, NULL))
			{
				MessageBox(hWnd, L"Помилка читання з файлу!", L"Помилка", MB_ICONERROR);
				delete[] lpFileData;
				return 0;
			}
			sf::MemoryInputStream mem_stream;
			mem_stream.open(lpFileData, dwFileSize);

			std::wstring text;
			GetWindowText(hFramesEdit, (LPWSTR)text.c_str(), 10);

			if (!wcscmp(text.c_str(), L""))
			{
				MessageBox(hWnd, L"Поле для вводу кількості кадрів не може бути пустим!", L"Помилка", MB_ICONERROR);
				return 0;
			}

			int frames_cnt;
			try
			{
				frames_cnt = std::stoi(text, nullptr, 10);
			}
			catch (std::exception& e)
			{
				MessageBox(hWnd, L"Кількість кадрів вказано в неправильному форматі!", L"Помилка", MB_ICONERROR);
				return 0;
			}

			if (frames_cnt < 1)
			{
				MessageBox(hWnd, L"Кількість кадрів не може бути меньше ніж 1!", L"Помилка", MB_ICONERROR);
				delete[] lpFileData;
				return 0;
			}

			text.clear();
			GetWindowText(hRowsCombo, (LPWSTR)text.c_str(), 1024);
			uint8_t rows_cnt = 0;
			for (int i = 0; i < sizeof(ROWS_PRESETS) / sizeof(RowPreset); i++)
			{
				if (!wcscmp(text.c_str(), ROWS_PRESETS[i].title.c_str()))
				{
					rows_cnt = ROWS_PRESETS[i].rows_count;
					break;
				}
			}

			if (rows_cnt == 0)
			{
				MessageBox(hWnd, L"Кількість рядків не може дорівнювати 0!", L"Помилка", MB_ICONERROR);
				delete[] lpFileData;
				return 0;
			}

			text.clear();
			GetWindowText(hPSpeedEdit, (LPWSTR)text.c_str(), 1024);
			if (!wcscmp(text.c_str(), L""))
			{
				MessageBox(hWnd, L"Поле для вводу швидкості відтворення не може бути пустим!", L"Помилка", MB_ICONERROR);
				return 0;
			}

			float frame_dur;
			try
			{
				frame_dur = std::stof(text, nullptr);
			}
			catch (std::exception& e)
			{
				MessageBox(hWnd, L"Швидкість відтворення вказано в неправильному форматі!", L"Помилка", MB_ICONERROR);
				return 0;
			}

			texture.loadFromStream(mem_stream);

			if (tileset) delete tileset;
			tileset = new sf::Sprite();
			tileset->setTexture(texture);
			tileset->setScale(sf::Vector2f(3, 3));
			tileset->setPosition(sf::Vector2f(10, 10));

			animation.setPosition(sf::Vector2f(tileset->getGlobalBounds().left + tileset->getGlobalBounds().width + 50, 10));
			animation.loadFromMemory(texture, (uint8_t)frames_cnt, rows_cnt, frame_dur);
			animation.setCurrentFrame(0);
			animation.setCurrentRow(0);
			UpdatePointer();

			for (auto it = control_list.begin(); it != control_list.end(); it++)
				EnableWindow(*it, true);
			EnableWindow(hPlayButton, true);

			delete[] lpFileData;
		}
		else if (hMenu == CM_MOVEPTOP)
		{
			int row = animation.getCurrentRow() - 1;
			if (row < 0) row = animation.getRowsCount() - 1;
			animation.setCurrentRow(row);
			SetWindowText(hRowStatic, (L"Потчний рядок: " + std::to_wstring(row + 1)).c_str());
			UpdatePointer();
		}
		else if (hMenu == CM_MOVEPBOT)
		{
			int row = animation.getCurrentRow() + 1;
			if (row > animation.getRowsCount() - 1) row = 0;
			animation.setCurrentRow(row);
			SetWindowText(hRowStatic, (L"Потчний рядок: " + std::to_wstring(row + 1)).c_str());
			UpdatePointer();
		}
		else if (hMenu == CM_MOVEPLEFT)
		{
			int frame = animation.getCurrentFrame() - 1;
			if (frame < 0) frame = animation.getFramesCount() - 1;
			animation.setCurrentFrame(frame);
			SetWindowText(hFrameStatic, (L"Потчний кадр: " + std::to_wstring(frame + 1)).c_str());
			UpdatePointer();
		}
		else if (hMenu == CM_MOVEPRIGHT)
		{
			int frame = animation.getCurrentFrame() + 1;
			if (frame > animation.getFramesCount() - 1) frame = 0;
			animation.setCurrentFrame(frame);
			SetWindowText(hFrameStatic, (L"Потчний кадр: " + std::to_wstring(frame + 1)).c_str());
			UpdatePointer();
		}
		else if (hMenu == CM_PLAY)
		{
			for (auto it = control_list.begin(); it != control_list.end(); it++)
				EnableWindow(*it, false);
			EnableWindow(hPlayButton, false);
			EnableWindow(hStopButton, true);
			EnableWindow(hLoadButton, false);
			SetWindowText(hModeStatic, L"Потчний режим: Відтворення");
			is_play_mode = true;
		}
		else if (hMenu == CM_STOP)
		{
			for (auto it = control_list.begin(); it != control_list.end(); it++)
				EnableWindow(*it, true);
			EnableWindow(hPlayButton, true);
			EnableWindow(hStopButton, false);
			EnableWindow(hLoadButton, true);
			SetWindowText(hModeStatic, L"Потчний режим: Статичний");
			is_play_mode = false;
			animation.setCurrentFrame(0);
			UpdatePointer();
		}
	}
	else if (uMsg == WM_CREATE)
	{
		CreateWidgets(hWnd);
	}
	else if (uMsg == WM_CTLCOLORSTATIC)
	{
		HDC hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		return (INT_PTR)hBackgorundBrush;
	}
	else if (uMsg == WM_CLOSE) PostQuitMessage(0);
	else return DefWindowProc(hWnd, uMsg, wParam, lParam);

	return 0;
}

void CreateWidgets(HWND hWnd)
{
	HINSTANCE hi = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	POINT start = { 25, 20 };
	CreateWindow(L"static", L"Шлях до файлу:", WS_CHILD | WS_VISIBLE, start.x, start.y, 256, 20, hWnd, 0, hi, 0);
	hFileEdit = CreateWindow(L"edit", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, start.x, start.y + 20, 256, 20, hWnd, 0, hi, 0);

	CreateWindow(L"static", L"Кількість кадрів:", WS_CHILD | WS_VISIBLE, start.x, start.y + 50, 256, 20, hWnd, 0, hi, 0);
	hFramesEdit = CreateWindow(L"edit", L"1", WS_VISIBLE | WS_CHILD, start.x, start.y + 70, 50, 20, hWnd, 0, hi, 0);
	CreateWindow(L"button", L"+", WS_CHILD | WS_VISIBLE, start.x + 55, start.y + 70, 25, 20, hWnd, CM_INCREASE, hi, 0);
	CreateWindow(L"button", L"-", WS_CHILD | WS_VISIBLE, start.x + 80, start.y + 70, 25, 20, hWnd, CM_DECREASE, hi, 0);

	CreateWindow(L"static", L"Тип тайлсету:", WS_CHILD | WS_VISIBLE, start.x, start.y + 100, 120, 20, hWnd, 0, hi, 0);
	hRowsCombo = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS | WS_VSCROLL | CBS_SIMPLE | ES_READONLY, start.x, start.y + 120, 100, 80, hWnd, 0, hi, 0);
	for (int i = 0; i < sizeof(ROWS_PRESETS) / sizeof(RowPreset); i++)
		SendMessage(hRowsCombo, CB_ADDSTRING, 0, (LPARAM)ROWS_PRESETS[i].title.c_str());
	SendMessage(hRowsCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	CreateWindow(L"static", L"Швидкість відтворення:", WS_VISIBLE | WS_CHILD, start.x, start.y + 155, 256, 20, hWnd, 0, hi, 0);
	hPSpeedEdit = CreateWindow(L"edit", L"0.2", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_RIGHT, start.x, start.y + 175, 80, 20, hWnd, 0, hi, 0);
	CreateWindow(L"static", L"секунд.", WS_VISIBLE | WS_CHILD, start.x + 83, start.y + 179, 100, 20, hWnd, 0, hi, 0);

	hLoadButton = CreateWindow(L"button", L"Завантажити", WS_VISIBLE | WS_CHILD, start.x, start.y + 205, 100, 20, hWnd, CM_LOADANIM, hi, 0);

	POINT control_offset = {start.x, start.y + 405};

	hFrameStatic = CreateWindow(L"static", L"Потчний кадр: 1", WS_VISIBLE | WS_CHILD, control_offset.x, control_offset.y, 256, 20, hWnd, 0, hi, 0);
	hRowStatic = CreateWindow(L"static", L"Потчний рядок: 1", WS_VISIBLE | WS_CHILD, control_offset.x, control_offset.y + 25, 256, 20, hWnd, 0, hi, 0);
	hModeStatic = CreateWindow(L"static", L"Потчний режим: Статичний", WS_VISIBLE | WS_CHILD, control_offset.x, control_offset.y + 50, 256, 20, hWnd, 0, hi, 0);

	control_list.push_back(CreateWindow(L"button", L"A", WS_VISIBLE | WS_CHILD | WS_DISABLED, control_offset.x + 30, control_offset.y + 80, 30, 30, hWnd, CM_MOVEPTOP, hi, 0));
	control_list.push_back(CreateWindow(L"button", L">", WS_VISIBLE | WS_CHILD | WS_DISABLED, control_offset.x + 60, control_offset.y + 110, 30, 30, hWnd, CM_MOVEPRIGHT, hi, 0));
	control_list.push_back(CreateWindow(L"button", L"v", WS_VISIBLE | WS_CHILD | WS_DISABLED, control_offset.x + 30, control_offset.y + 110, 30, 30, hWnd, CM_MOVEPBOT, hi, 0));
	control_list.push_back(CreateWindow(L"button", L"<", WS_VISIBLE | WS_CHILD | WS_DISABLED, control_offset.x, control_offset.y + 110, 30, 30, hWnd, CM_MOVEPLEFT, hi, 0));

	hPlayButton = CreateWindow(L"button", L"Відтворити", WS_CHILD | WS_VISIBLE | WS_DISABLED, control_offset.x + 120, control_offset.y + 80, 100, 25, hWnd, CM_PLAY, hi, 0);
	hStopButton = CreateWindow(L"button", L"Стоп", WS_CHILD | WS_VISIBLE | WS_DISABLED, control_offset.x + 120, control_offset.y + 115, 100, 25, hWnd, CM_STOP, hi, 0);
}

void UpdatePointer()
{
	pointer.setSize(sf::Vector2f(animation.getFrameRect().width, animation.getFrameRect().height) * tileset->getScale());
	pointer.setPosition(sf::Vector2f(
		tileset->getPosition().x + (float)animation.getCurrentFrame() * animation.getFrameRect().width * tileset->getScale().x,
		tileset->getPosition().y + (float)animation.getCurrentRow() * animation.getFrameRect().height * tileset->getScale().y
	));
}