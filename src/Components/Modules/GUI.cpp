#include "GUI.hpp"

#include "Window.hpp"

#ifdef _DEBUG

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
EndScene oEndScene = NULL;

WNDPROC oWndProc;
static HWND window = NULL;

namespace Components
{
	static bool GUIInitialized = false;
	static bool Toggled = false;

	static IDirect3DDevice9* device = nullptr;

	static Utils::Concurrency::Container<std::vector<GUI::FrameCallback>> on_frame_callbacks{};
	static Utils::Concurrency::Container<std::vector<GUI::Event>> event_queue{};

	static std::unordered_map<std::string, bool> enabled_menus{};
	static std::vector<GUI::menu_t> menus{};

	namespace
	{
		LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

			if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
				return true;

			return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
		}

		BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
		{
			DWORD wndProcId;
			GetWindowThreadProcessId(handle, &wndProcId);

			if (GetCurrentProcessId() != wndProcId)
				return TRUE; // skip to next window

			window = handle;
			return FALSE; // window found abort search
		}

		HWND GetProcessWindow()
		{
			window = NULL;
			EnumWindows(EnumWindowsCallback, NULL);
			return window;
		}
	}

	void GUI::RunFrameCallbacks()
	{
		on_frame_callbacks.access([](std::vector<FrameCallback>& callbacks)
		{
			for (const auto& [callback, always] : callbacks)
			{
				const auto run_callback = always
	#ifdef _DEBUG
				|| Toggled
	#endif
				;
            
				if (run_callback)
				{
					callback();
				}
			}
		});
	}

	void GUI::RegisterCallback(const std::function<void()>& callback, bool always)
	{
		on_frame_callbacks.access([always, callback](std::vector<FrameCallback>& callbacks)
		{
			callbacks.emplace_back(FrameCallback{ .callback = callback, .always = always });
		});
	}

	void GUI::RegisterMenu(const std::string& name, const std::string& title, const std::function<void()>& callback,
		const bool always)
	{
		menus.emplace_back(menu_t{ .name = name, .title = title, .render = callback });
		enabled_menus[name] = false;

		RegisterCallback([=]()
		{
			if (enabled_menus[name])
			{
				callback();
			}
		}, always);
	}

	bool* GUI::GetEnabledMenu(const std::string& name)
	{
		return &enabled_menus[name];
	}

	/*
	void GUI::RunEventQueue()
	{
		event_queue.access([](std::vector<Event>& queue)
		{
			for (const auto& event : queue)
			{
				ImGui_ImplWin32_WndProcHandler(event.hWnd, event.msg, event.wParam, event.lParam);
			}
			queue.clear();
		});
	}
	*/

	void GUI::NewGUIFrame()
	{
		ImGui::GetIO().MouseDrawCursor = Toggled;

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void GUI::EndGUIFrame()
	{
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	void GUI::MenuCheckbox(const std::string& name, const std::string& menu)
	{
		ImGui::Checkbox(name.data(), &enabled_menus[menu]);
	}

	// main menu
	void GUI::DrawMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Windows"))
			{
				for (const auto& menu : menus)
				{
					MenuCheckbox(menu.title, menu.name);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	// callback
	bool GUI::KeyEvent(const int localClientNum, const int key, const int down)
	{
		if (key == Game::K_F11 && down)
		{
			Toggled = !Toggled;
			return false;
		}

		if (key == Game::K_ESCAPE && down && Toggled)
		{
			Toggled = false;
			return false;
		}

		return !Toggled;
	}

	bool GUI::IsOpen()
	{
		return !Toggled;
	}

	void GUI::SetupGlobals()
	{

	}

	void GUI::InitializeGUI()
	{
		if (!*Game::dx_ptr)
			return;

		ImGui::CreateContext();
		SetupGlobals();
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(Window::GetWindow());
		ImGui_ImplDX9_Init(*Game::dx_ptr);

		GUIInitialized = true;
	}

	void GUI::OnFrame()
	{
		if (!Game::Sys_IsDatabaseReady2())
		{
			return;
		}

		if (!GUIInitialized)
		{
			Logger::Print("[ImGui] Initializing\n");
			InitializeGUI();
		}
		else
		{
			NewGUIFrame();
			RunFrameCallbacks();
			EndGUIFrame();
		}
	}

	long __stdcall GUI::hkEndScene(LPDIRECT3DDEVICE9 pDevice)
	{
		OnFrame();

		return oEndScene(pDevice);
	}

	LRESULT GUI::WndProcStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (wParam != VK_ESCAPE && Toggled)
		{
			event_queue.access([hWnd, msg, wParam, lParam](std::vector<Event>& queue)
			{
				queue.emplace_back(hWnd, msg, wParam, lParam);
			});
		}

		return Utils::Hook::Call<LRESULT(HWND, UINT, WPARAM, LPARAM)>(0x4731F0)(hWnd, msg, wParam, lParam);
	}

	__declspec(naked) void GUI::RB_Frame_Stub()
	{
		__asm
		{
			call OnFrame
			retn
		}
	}

	void GUI::Win_RegisterClass_Stub()
	{
		WNDCLASSEXA dst = {};

		dst.lpfnWndProc = reinterpret_cast<WNDPROC>(WndProcStub);
		dst.hInstance = *reinterpret_cast<HINSTANCE*>(0x64A3AD4); // hInstance
		dst.cbSize = 80;
		dst.hIcon = LoadIconA(*reinterpret_cast<HINSTANCE*>(0x64A3AD4), reinterpret_cast<LPCSTR>(0x7F00)); // ID_ICON = 0x7F00
		dst.hCursor = LoadCursorA(nullptr, reinterpret_cast<LPCSTR>(0x7F00));
		dst.hbrBackground = CreateSolidBrush(0);
		dst.lpszClassName = "IW4";

		if (!RegisterClassExA(&dst))
			Game::Com_Error(Game::ERR_FATAL, "EXE_ERR_COULDNT_REGISTER_WINDOW");
	}

	bool GUI::CreateDevice_Stub(void* a1)
	{
		if (GUIInitialized)
		{
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}

		GUIInitialized = false;

		return Utils::Hook::Call<bool(void*)>(0x507460)(a1);
	}

	void GUI::sub_5078C0_Stub()
	{
		Utils::Hook::Call<void()>(0x5078C0)();

		device = *Game::dx_ptr;
	}

	GUI::GUI()
	{
		//Utils::Hook(0x536B59, RB_Frame_Stub, HOOK_JUMP).install()->quick();

		static bool attached = false;
		Scheduler::Loop([&]
		{
			if (!attached)
			{
				auto result = kiero::init(kiero::RenderType::D3D9);

				printf("[GUI] kiero result is %s\n", kiero::status_to_str(result).c_str());

				if (result == kiero::Status::Success)
				{
					attached = true;

					kiero::bind(42, (void**)&oEndScene, hkEndScene);
					do
						window = GetProcessWindow();
					while (window == NULL);
					oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
				}
			}
		}, Scheduler::Pipeline::MAIN);

		Utils::Hook(0x507BD5, sub_5078C0_Stub, HOOK_CALL).install()->quick();

		RegisterCallback([]
		{
			DrawMainMenuBar();
		}, false);

		// change wndproc
		//Utils::Hook(0x64D270, Win_RegisterClass_Stub, HOOK_JUMP).install()->quick();

		//Utils::Hook(0x5074D3, CreateDevice_Stub, HOOK_CALL).install()->quick();
	}
}
#endif
