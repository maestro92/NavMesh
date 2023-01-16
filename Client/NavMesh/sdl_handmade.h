#pragma once
#include "../PlatformShared/platform_shared.h"
#include "SDL.h"

// this is a fifo work queue
struct PlatformWorkQueue
{
	struct Job
	{
		// User can put any data it wants
		void* data;
	};

	// nextEntryToExecute tries to catch up to numEntries. 
	volatile uint32 targetCompletionGoal;
	volatile uint32 numCompletedTask;
	volatile int32 nextEntryToRead;
	volatile int32 nextEntryToWrite;
	Job entries[256];

	// A semaphore controls the maximum num of people allowed inside the critical section
	// And people outside of the critical section are put to sleep.
	// in our case, when there are jobs, we want threads inside the critical section
	// if there are no jobs, we dont want anyone in the critical section
	// if you are working, you are inside critical section. otherwise you are sleeping

	//	SDL_SemWait(workqueue.semaphoreHandle);. This is registering a thread to a sempahore
	//	its saying im waiting for your Post operation

	//	SDL_SemPost();.	This is saying im waking up one of the threads to go through
	SDL_sem* semaphoreHandle;

	void init()
	{
		semaphoreHandle = SDL_CreateSemaphore(0);
		nextEntryToWrite = 0;
		nextEntryToRead = 0;
	}

	bool HasOutstandingWork()
	{
		return nextEntryToRead != nextEntryToWrite;
	}

	void AddJob(void* userData)
	{
		// If its not full
		uint32 newNextEntryToWrite = (nextEntryToWrite + 1) % ArrayCount(entries);
		assert(newNextEntryToWrite != nextEntryToRead);

		Job* entry = &entries[nextEntryToWrite];
		entry->data = userData;
		targetCompletionGoal++;
		SDL_CompilerBarrier();
		nextEntryToWrite = newNextEntryToWrite;
		SDL_SemPost(semaphoreHandle);
	}

	Job* PopNextJob()
	{
		uint32 orignalNextEntryToRead = nextEntryToRead;


		// if its emtpy 
		if (nextEntryToRead != nextEntryToWrite)
		{
			uint32 newNextEntryToRead = (orignalNextEntryToRead + 1) % ArrayCount(entries);
			SDL_bool wasSet = SDL_AtomicCAS((SDL_atomic_t*)&nextEntryToRead, orignalNextEntryToRead, newNextEntryToRead);

			if (wasSet)
			{
				Job* entry = &entries[orignalNextEntryToRead];
				return entry;
			}
		}
		return nullptr;
	}

	void MarkJobCompleted()
	{
		int entryIndex = SDL_AtomicAdd((SDL_atomic_t*)&numCompletedTask, 1);
	}

	bool AreAllJobsCompleted()
	{
		return numCompletedTask == targetCompletionGoal;
	}
};
