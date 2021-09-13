
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

	struct UpdateEventArgs : public EventArgs
	{
		UpdateEventArgs(double deltaTime, double totalTime) :
			elapsedTime(deltaTime),
			totalTime(totalTime) {}

		double elapsedTime;
		double totalTime;
	};
}