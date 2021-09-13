
namespace Mizu
{
	struct EventArgs {};

	struct ReizeEventArgs : public EventArgs
	{
		typedef EventArgs base;
		ReizeEventArgs(int width, int height) :
			Width(width),
			Height(height)
		{}


		int Width;
		int Height;
	};
}