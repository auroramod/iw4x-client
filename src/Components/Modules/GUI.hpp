#pragma once

namespace Components
{
	class GUI : public Component
	{
	public:
		struct FrameCallback
		{
			std::function<void()> callback;
			bool always;
		};

		struct Event
		{
			HWND hWnd;
			UINT msg;
			WPARAM wParam;
			LPARAM lParam;
		};

		struct menu_t
		{
			std::string name;
			std::string title;
			std::function<void()> render;
		};

		struct notification_t
		{
			std::string title;
			std::string text;
			std::chrono::milliseconds duration{};
			std::chrono::high_resolution_clock::time_point creation_time{};
		};

		GUI();

		static bool KeyEvent(const int localClientNum, const int key, const int down);

		static bool IsOpen();

		static void RegisterCallback(const std::function<void()>& callback, bool always = false);
		static void RegisterMenu(const std::string& name, const std::string& title, const std::function<void()>& callback,
			const bool always = false);

		static bool* GetEnabledMenu(const std::string& name);

	private:
		static void InitializeGUI();
		static void OnFrame();

		static void RunEventQueue();

		static void RunFrameCallbacks();

		static bool CreateDevice_Stub(void* a1);

		static void NewGUIFrame();
		static void EndGUIFrame();

		static void SetupGlobals();

		static long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice);
		static LRESULT WndProcStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static void sub_5078C0_Stub();

		static void MenuCheckbox(const std::string& name, const std::string& menu);
		static void DrawMainMenuBar();

		// hooks
		static void RB_Frame_Stub();
		static void Win_RegisterClass_Stub();
	};
}
