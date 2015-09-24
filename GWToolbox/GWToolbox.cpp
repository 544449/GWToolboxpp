#include "GWToolbox.h"

#include "../include/OSHGui/OSHGui.hpp"
#include "../include/OSHGui/Drawing/Direct3D9/Direct3D9Renderer.hpp"
#include "../include/OSHGui/Drawing/Theme.hpp"
#include "../include/OSHGui/Input/WindowsMessage.hpp"

#include <string>

#include "Timer.h"
#include "MainWindow.h"
#include "TimerWindow.h"

using namespace OSHGui::Drawing;
using namespace OSHGui::Input;

GWToolbox* GWToolbox::instance_ = NULL;
bool GWToolbox::capture_input = false;

namespace{
	GWAPI::GWAPIMgr * mgr;
	GWAPI::DirectXMgr * dx;

	Direct3D9Renderer* renderer;

	HHOOK oshinputhook;
	long OldWndProc = 0;
	WindowsMessage input;
}

static LRESULT CALLBACK NewWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	
	if (Message == WM_QUIT || Message == WM_CLOSE) {
		GWToolbox::instance()->config()->save();
		return CallWindowProc((WNDPROC)OldWndProc, hWnd, Message, wParam, lParam);
	}

	if (Application::InstancePtr()->HasBeenInitialized()) {
		MSG msg;
		msg.hwnd = hWnd;
		msg.message = Message;
		msg.wParam = wParam;
		msg.lParam = lParam;

		switch (Message) {
		// Send right mouse button events to gw (move view around) and don't mess with them
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			break;

		// Send button up mouse events to both gw and osh, to avoid gw being stuck on mouse-down
		case WM_LBUTTONUP:
			input.ProcessMessage(&msg);
			break;
		
		// Send other mouse events to osh first and consume them if used
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
			if (input.ProcessMessage(&msg)) {
				return true;
			} else {
				Application::InstancePtr()->clearFocus();
			}
			break;

		// send keyboard messages to gw, osh and toolbox
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_IME_CHAR:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			GWToolbox::instance()->main_window()->hotkey_panel()->ProcessMessage(&msg);
			if (GWToolbox::capture_input) {
				input.ProcessMessage(&msg);
				return true;
			}
			break;
		}
	}

	return CallWindowProc((WNDPROC)OldWndProc, hWnd, Message, wParam, lParam);
}


void create_gui(IDirect3DDevice9* pDevice) {

	LOG("Creating GUI...");
	renderer = new Direct3D9Renderer(pDevice);
	Application::Initialize(std::unique_ptr<Direct3D9Renderer>(renderer));

	Application * app = Application::InstancePtr();

	string path = GuiUtils::getPathA("Theme.txt");
	try {
		Theme theme;
		theme.Load(path);
		app->SetTheme(theme);
	} catch (Misc::InvalidThemeException e) {
		ERR("WARNING Could not load theme file %s\n", path.c_str());
	}
	
	app->SetDefaultFont(GuiUtils::getTBFont(10.0f, true));

	app->SetCursorEnabled(false);
	try {
		MainWindow* main_window = new MainWindow();
		main_window->SetFont(app->GetDefaultFont());
		std::shared_ptr<MainWindow> shared_ptr = std::shared_ptr<MainWindow>(main_window);
		app->Run(shared_ptr);

		GWToolbox::instance()->set_main_window(main_window);
		GWToolbox::instance()->set_timer_window(new TimerWindow());
		GWToolbox::instance()->set_bonds_window(new BondsWindow());
		GWToolbox::instance()->set_health_window(new HealthWindow());
		GWToolbox::instance()->set_distance_window(new DistanceWindow());

		app->Enable();
		GWToolbox::instance()->SetInitialized();

		LOG("ok\n");
	} catch (Misc::FileNotFoundException e) {
		LOG("Error: file not found %s\n", e.what());
		GWToolbox::instance()->StartSelfDestruct();
	}
}

// All rendering done here.
static HRESULT WINAPI endScene(IDirect3DDevice9* pDevice) {
	static GWAPI::DirectXMgr::EndScene_t origfunc = dx->GetEndsceneReturn();
	static bool init = false;
	if (!init) {
		init = true;
		create_gui(pDevice);
	}

	GWToolbox::instance()->UpdateUI();

	renderer->BeginRendering();

	Application::InstancePtr()->Render();

	renderer->EndRendering();

	return origfunc(pDevice);
}

static HRESULT WINAPI resetScene(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	static GWAPI::DirectXMgr::Reset_t origfunc = dx->GetResetReturn();


	// pre-reset here.

	renderer->PreD3DReset();

	HRESULT result = origfunc(pDevice, pPresentationParameters);
	if (result == D3D_OK){
		// post-reset here.
		renderer->PostD3DReset();
	}

	return result;
}

void GWToolbox::Exec() {
	mgr = GWAPI::GWAPIMgr::GetInstance();
	dx = mgr->DirectX;

	LOG("Installing dx hooks... ");
	dx->CreateRenderHooks(endScene, resetScene);
	LOG("ok\n");

	LOG("Installing input event handler... ");
	HWND hWnd = GWAPI::MemoryMgr::GetGWWindowHandle();
	OldWndProc = SetWindowLongPtr(hWnd, GWL_WNDPROC, (long)NewWndProc);
	LOG("ok\n");
	
	input.SetKeyboardInputEnabled(true);
	input.SetMouseInputEnabled(true);

	MainLoop();
}

void GWToolbox::MainLoop() {

	Application * app = Application::InstancePtr();

	while (true) { // main loop
		if (app->HasBeenInitialized() && initialized_) {
			__try {
				main_window_->MainRoutine();
				timer_window_->MainRoutine();
				bonds_window_->MainRoutine();
				health_window_->MainRoutine();
				distance_window_->MainRoutine();
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				LOG("Badness happened! (in main thread)\n");
			}
		}

		Sleep(10);

		if (DEBUG_BUILD && GetAsyncKeyState(VK_END) & 1)
			Destroy();
		if (must_self_destruct_)
			Destroy();
	}
}

void GWToolbox::UpdateUI() {
	if (initialized_) {
		__try {
			main_window_->UpdateUI();
			timer_window_->UpdateUI();
			bonds_window_->UpdateUI();
			health_window_->UpdateUI();
			distance_window_->UpdateUI();
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			LOG("Badness happened! (in render thread)\n");
		}
	}
}

void GWToolbox::Destroy()
{
	LOG("Destroying GWToolbox++\n");

	Sleep(100);

	config_->save();
	Sleep(100);
	delete config_;
	Sleep(100);
	HWND hWnd = GWAPI::MemoryMgr::GetGWWindowHandle();
	SetWindowLongPtr(hWnd, GWL_WNDPROC, (long)OldWndProc);
	Sleep(100);
	GWAPI::GWAPIMgr::Destruct();
#if DEBUG_BUILD
	FreeConsole();
#endif
	Sleep(100);
	FreeLibraryAndExitThread(m_dllmodule, EXIT_SUCCESS);
}


void GWToolbox::threadEntry(HMODULE mod) {
	if (GWToolbox::instance()) return;

	LOG("Initializing API... ");
	GWAPI::GWAPIMgr::Initialize();
	LOG("ok\n");

	LOG("Creating GWToolbox++... ");
	instance_ = new GWToolbox(mod);
	LOG("ok\n");

	instance_->Exec();
}
