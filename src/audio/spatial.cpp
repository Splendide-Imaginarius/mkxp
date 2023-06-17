#include "spatial.h"

#include "sharedstate.h"
#include "filesystem.h"
#include "exception.h"
#include "config.h"
#include "util.h"
#include "debugwriter.h"

#include <SDL_sound.h>

#define SE_CACHE_MEM (10*1024*1024) // 10 MB

SpatialEmitter::SoundEmitter(const Config &conf)
    : bufferBytes(0),
      srcCount(4),
      alSrcs(srcCount),
      atchBufs(srcCount),
      srcPrio(srcCount)
{
	for (size_t i = 0; i < srcCount; ++i)
	{
		alSrcs[i] = AL::Source::gen();
		atchBufs[i] = 0;
		srcPrio[i] = i;
	}
}

SpatialEmitter::~SoundEmitter()
{
	for (size_t i = 0; i < srcCount; ++i)
	{
		AL::Source::stop(alSrcs[i]);
		AL::Source::del(alSrcs[i]);

		if (atchBufs[i])
			SoundBuffer::deref(atchBufs[i]);
	}

	BufferHash::const_iterator iter;
	for (iter = bufferHash.cbegin(); iter != bufferHash.cend(); ++iter)
		SoundBuffer::deref(iter->second);
}

void SpatialEmitter::play(const std::string &filename,
                        int volume,
                        int pitch,
						double x, double y, double z)
{
	float _volume = clamp<int>(volume, 0, 100) / 100.0f;
	float _pitch  = clamp<int>(pitch, 50, 150) / 100.0f;

	SoundBuffer *buffer = allocateBuffer(filename);

	if (!buffer)
		return;

	/* Try to find first free source */
	size_t i;
	for (i = 0; i < srcCount; ++i)
		if (AL::Source::getState(alSrcs[srcPrio[i]]) != AL_PLAYING)
			break;

	/* If we didn't find any, try to find the lowest priority source
	 * with the same buffer to overtake */
	if (i == srcCount)
		for (size_t j = 0; j < srcCount; ++j)
			if (atchBufs[srcPrio[j]] == buffer)
				i = j;

	/* If we didn't find any, overtake the one with lowest priority */
	if (i == srcCount)
		i = 0;

	size_t srcIndex = srcPrio[i];

	/* Only detach/reattach if it's actually a different buffer */
	bool switchBuffer = (atchBufs[srcIndex] != buffer);

	/* Push the used source to the back of the priority list */
	arrayPushBack(srcPrio, srcCount, i);

	AL::Source::ID src = alSrcs[srcIndex];
	AL::Source::stop(src);

	if (switchBuffer)
		AL::Source::detachBuffer(src);

	SoundBuffer *old = atchBufs[srcIndex];

	if (old)
		SoundBuffer::deref(old);

	atchBufs[srcIndex] = SoundBuffer::ref(buffer);

	if (switchBuffer)
		AL::Source::attachBuffer(src, buffer->alBuffer);

	AL::Source::setVolume(src, _volume * GLOBAL_VOLUME);
	AL::Source::setPitch(src, _pitch);
	AL::Source::setPosition(src, x, y, z);

	AL::Source::play(src);
}

void SpatialEmitter::stop()
{
	for (size_t i = 0; i < srcCount; i++)
		AL::Source::stop(alSrcs[i]);
}

struct SoundOpenHandler : FileSystem::OpenHandler
{
	SoundBuffer *buffer;

	SoundOpenHandler()
	    : buffer(0)
	{}

	bool tryRead(SDL_RWops &ops, const char *ext)
	{
		Sound_Sample *sample = Sound_NewSample(&ops, ext, 0, STREAM_BUF_SIZE);

		if (!sample)
		{
			SDL_RWclose(&ops);
			return false;
		}

		/* Do all of the decoding in the handler so we don't have
		 * to keep the source ops around */
		uint32_t decBytes = Sound_DecodeAll(sample);
		uint8_t sampleSize = formatSampleSize(sample->actual.format);
		uint32_t sampleCount = decBytes / sampleSize;

		buffer = new SoundBuffer;
		buffer->bytes = sampleSize * sampleCount;

		ALenum alFormat = chooseALFormat(sampleSize, sample->actual.channels);

		AL::Buffer::uploadData(buffer->alBuffer, alFormat, sample->buffer,
							   buffer->bytes, sample->actual.rate);

		Sound_FreeSample(sample);

		return true;
	}
};

SoundBuffer *SpatialEmitter::allocateBuffer(const std::string &filename)
{
	SoundBuffer *buffer = bufferHash.value(filename, 0);

	if (buffer)
	{
		/* Buffer still in cashe.
		 * Move to front of priority list */
		buffers.remove(buffer->link);
		buffers.append(buffer->link);

		return buffer;
	}
	else
	{
		/* Buffer not in cache, needs to be loaded */
		SoundOpenHandler handler;
		shState->fileSystem().openRead(handler, filename.c_str());
		buffer = handler.buffer;

		if (!buffer)
		{
			char buf[512];
			snprintf(buf, sizeof(buf), "Unable to decode sound: %s: %s",
			         filename.c_str(), Sound_GetError());
			Debug() << buf;

			return 0;
		}

		buffer->key = filename;
		uint32_t wouldBeBytes = bufferBytes + buffer->bytes;

		/* If memory limit is reached, delete lowest priority buffer
		 * until there is room or no buffers left */
		while (wouldBeBytes > SE_CACHE_MEM && !buffers.isEmpty())
		{
			SoundBuffer *last = buffers.tail();
			bufferHash.remove(last->key);
			buffers.remove(last->link);

			wouldBeBytes -= last->bytes;

			SoundBuffer::deref(last);
		}

		bufferHash.insert(filename, buffer);
		buffers.prepend(buffer->link);

		bufferBytes = wouldBeBytes;

		return buffer;
	}
}
