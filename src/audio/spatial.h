#ifndef SPATIAL_H
#define SPATIAL_H

#include "soundemitter.h"

#include "intrulist.h"
#include "al-util.h"
#include "boost-hash.h"

#include <string>
#include <vector>

struct SoundBuffer;
struct Config;

struct SpatialEmitter
{
	typedef BoostHash<std::string, SoundBuffer*> BufferHash;

	IntruList<SoundBuffer> buffers;
	BufferHash bufferHash;

	/* Byte count sum of all cached / playing buffers */
	uint32_t bufferBytes;

	const size_t srcCount;
	std::vector<AL::Source::ID> alSrcs;
	std::vector<SoundBuffer*> atchBufs;

	/* Indices of sources, sorted by priority (lowest first) */
	std::vector<size_t> srcPrio;

	SpatialEmitter(const Config &conf);
	~SpatialEmitter();

	void play(const std::string &filename,
			  int volume,
			  int pitch,
			  double x, double y, double z);

	void stop();

private:
	SoundBuffer *allocateBuffer(const std::string &filename);
};

#endif // SOUNDEMITTER_H
