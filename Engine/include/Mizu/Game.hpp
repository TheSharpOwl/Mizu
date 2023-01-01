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

		int width() const { return m_width; }
		int height() const { return m_height; }
		bool vsync() const { return m_vsync; }
		std::wstring name() const { return m_name; }

		virtual bool initialize();

		virtual bool loadContent() = 0;

		virtual void unloadContent() = 0;

		virtual void reset();

		auto getWindow() { return m_window; }

	protected:

		friend class Window;

		virtual void onUpdate(UpdateEventArgs& e) = 0;

		virtual void onRender(RenderEventArgs& e) = 0;

		virtual void onResize(ResizeEventArgs& e);

		virtual void onWindowDestroy();


		std::shared_ptr<Window> m_window;


		// TODO add mouse/keyboard functions

		std::wstring m_name;
		int m_width;
		int m_height;
		bool m_vsync;

		bool m_contentLoaded = false;
	};
}