#include <memory>
#include <string>

#include "EventArgs.hpp"


namespace Mizu
{
	class Window;

	class Game : public std::enable_shared_from_this<Game>
	{

	public:

		Game(const std::wstring& name, int width, int height, bool vSync);
		
		virtual ~Game();

		int Width() const { return m_width; }
		int Height() const { return m_height; }
		bool Vsync() const { return m_vsync; }
		std::wstring Name() const { return m_name; }

		virtual bool Initialize();

		virtual bool LoadContent() = 0;

		virtual void UnloadContent() = 0;

		virtual void Reset();

		auto getWindow() { return m_window; }

	protected:

		friend class Window;

		virtual void OnUpdate(UpdateEventArgs& e) = 0;

		virtual void OnRender(RenderEventArgs& e) = 0;

		virtual void OnResize(ResizeEventArgs& e);

		virtual void OnWindowDestroy();


		std::shared_ptr<Window> m_window;


		// TODO add mouse/keyboard functions

		std::wstring m_name;
		int m_width;
		int m_height;
		bool m_vsync;

		bool m_contentLoaded = false;
	};
}