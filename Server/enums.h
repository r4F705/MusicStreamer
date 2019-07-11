#pragma once

namespace musicStreamer {

	enum class StreamerState {
		IDLE,
		STARTING,
		PLAYING,
		PAUSED,
		STOPPED,
		CLOSED,
	};


	enum class Command
	{
		STREAM = 460,
		PAUSE = 382,
		STOP = 326,
	};

}

