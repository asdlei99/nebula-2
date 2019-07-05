//------------------------------------------------------------------------------
//  vksubcontexthandler.cc
//  (C)2017-2018 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "render/stdneb.h"
#include "vksubcontexthandler.h"
#include "coregraphics/config.h"

namespace Vulkan
{

//------------------------------------------------------------------------------
/**
*/
void
VkSubContextHandler::Setup(VkDevice dev, const Util::FixedArray<uint> indexMap, const Util::FixedArray<uint> families)
{
	// store device
	this->device = dev;

	// get all queues related to their respective family (we can have more draw queues than 1, for example)
	this->drawQueues.Resize(indexMap[families[GraphicsQueueType]]);
	this->drawQueueStages.Resize(indexMap[families[GraphicsQueueType]]);
	this->computeQueues.Resize(indexMap[families[ComputeQueueType]]);
	this->computeQueueStages.Resize(indexMap[families[ComputeQueueType]]);
	this->transferQueues.Resize(indexMap[families[TransferQueueType]]);
	this->transferQueueStages.Resize(indexMap[families[TransferQueueType]]);
	this->sparseQueues.Resize(indexMap[families[SparseQueueType]]);
	this->sparseQueueStages.Resize(indexMap[families[SparseQueueType]]);

	this->queueFamilies[GraphicsQueueType] = families[GraphicsQueueType];
	this->queueFamilies[ComputeQueueType] = families[ComputeQueueType];
	this->queueFamilies[TransferQueueType] = families[TransferQueueType];
	this->queueFamilies[SparseQueueType] = families[SparseQueueType];

	uint i;
	for (i = 0; i < indexMap[families[GraphicsQueueType]]; i++)
	{
		vkGetDeviceQueue(dev, families[GraphicsQueueType], i, &this->drawQueues[i]);
		this->drawQueueStages[i] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	for (i = 0; i < indexMap[families[ComputeQueueType]]; i++)
	{
		vkGetDeviceQueue(dev, families[ComputeQueueType], i, &this->computeQueues[i]);
		this->computeQueueStages[i] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	for (i = 0; i < indexMap[families[TransferQueueType]]; i++)
	{
		vkGetDeviceQueue(dev, families[TransferQueueType], i, &this->transferQueues[i]);
		this->transferQueueStages[i] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	for (i = 0; i < indexMap[families[SparseQueueType]]; i++)
	{
		vkGetDeviceQueue(dev, families[SparseQueueType], i, &this->sparseQueues[i]);
		this->sparseQueueStages[i] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	this->currentDrawQueue = 0;
	this->currentComputeQueue = 0;
	this->currentTransferQueue = 0;
	this->currentSparseQueue = 0;
}

//------------------------------------------------------------------------------
/**
*/
void
VkSubContextHandler::Discard()
{

}

//------------------------------------------------------------------------------
/**
*/
void
VkSubContextHandler::SetToNextContext(const CoreGraphicsQueueType type)
{
	Util::FixedArray<VkQueue>* list = nullptr;
	uint* currentQueue = nullptr;
	switch (type)
	{
	case GraphicsQueueType:
		list = &this->drawQueues;
		currentQueue = &this->currentDrawQueue;
		break;
	case ComputeQueueType:
		list = &this->computeQueues;
		currentQueue = &this->currentComputeQueue;
		break;
	case TransferQueueType:
		list = &this->transferQueues;
		currentQueue = &this->currentTransferQueue;
		break;
	case SparseQueueType:
		list = &this->sparseQueues;
		currentQueue = &this->currentSparseQueue;
		break;
	}

	// progress the queue index
	*currentQueue = (*currentQueue + 1) % list->Size();
}

//------------------------------------------------------------------------------
/**
*/
void 
VkSubContextHandler::AddSubmission(CoreGraphicsQueueType type, VkCommandBuffer cmds, VkSemaphore waitSemaphore, VkPipelineStageFlags waitFlag, VkSemaphore signalSemaphore)
{
	Util::Array<VkCommandBuffer>& buffers = this->buffers[type];
	Util::Array<VkSemaphore>& waitSemaphores = this->waitSemaphores[type];
	Util::Array<VkPipelineStageFlags>& waitFlags = this->waitFlags[type];
	Util::Array<VkSemaphore>& signalSemaphores = this->signalSemaphores[type];
	Util::Array<VkSubmitInfo>& submitInfos = this->submitInfos[type];

	VkSubmitInfo info =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr, nullptr,	// wait
		0, nullptr,				// cmd buf
		0, nullptr				// signal
	};

	// if command buffer is present, add to 
	if (cmds != VK_NULL_HANDLE)
	{
		buffers.Append(cmds);
		info.commandBufferCount = 1;
		info.pCommandBuffers = &buffers.Back();
	}
		
	// if we have wait semaphores, add both flags and the semaphore it self
	if (waitSemaphore != VK_NULL_HANDLE)
	{
		waitSemaphores.Append(waitSemaphore);
		waitFlags.Append(waitFlag);
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &waitSemaphores.Back();
		info.pWaitDstStageMask = &waitFlags.Back();
	}

	// finally add signal semaphore if present
	if (signalSemaphore != VK_NULL_HANDLE)
	{
		signalSemaphores.Append(signalSemaphore);
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &signalSemaphores.Back();
	}

	// finally, add submission
	submitInfos.Append(info);
	
}

//------------------------------------------------------------------------------
/**
*/
void 
VkSubContextHandler::AddWaitSemaphore(CoreGraphicsQueueType type, VkSemaphore waitSemaphore, VkPipelineStageFlags waitFlag)
{
	Util::Array<VkSemaphore>& waitSemaphores = this->waitSemaphores[type];
	Util::Array<VkPipelineStageFlags>& waitFlags = this->waitFlags[type];
	Util::Array<VkSubmitInfo>& submitInfos = this->submitInfos[type];
	VkSubmitInfo& info = submitInfos.Back();

	// if we have wait semaphores, add both flags and the semaphore it self
	if (waitSemaphore != VK_NULL_HANDLE)
	{
		waitSemaphores.Append(waitSemaphore);
		waitFlags.Append(waitFlag);

		// get pointer to previous item using the count to offset the value
		info.pWaitSemaphores = &waitSemaphores.Back() - info.waitSemaphoreCount; 
		info.pWaitDstStageMask = &waitFlags.Back() - info.waitSemaphoreCount;

		info.waitSemaphoreCount++; // add 1 more to the semaphore count
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
VkSubContextHandler::FlushSubmissions(CoreGraphicsQueueType type, VkFence fence, bool waitImmediately, VkSemaphore queueSemaphore, VkPipelineStageFlags queueWaitFlags)
{
	Util::Array<VkSubmitInfo>& submitInfos = this->submitInfos[type];

	if (submitInfos.Size() > 0)
	{
		VkQueue queue = VK_NULL_HANDLE;
		switch (type)
		{
		case GraphicsQueueType:
			queue = this->drawQueues[this->currentDrawQueue];
			break;
		case ComputeQueueType:
			queue = this->computeQueues[this->currentComputeQueue];
			break;
		case TransferQueueType:
			queue = this->transferQueues[this->currentTransferQueue];
			break;
		case SparseQueueType:
			queue = this->sparseQueues[this->currentSparseQueue];
			break;
		}

		// if we have an end-to-end cross-queue semaphore, add it to the beginning of the first submission
		if (queueSemaphore != VK_NULL_HANDLE)
		{
			Util::Array<VkSemaphore>& waitSemaphores = this->waitSemaphores[type];
			Util::Array<VkPipelineStageFlags>& waitFlags = this->waitFlags[type];
			waitSemaphores.Insert(0, queueSemaphore);
			waitFlags.Insert(0, queueWaitFlags);

			// set the initial submit semaphore
			submitInfos[0].waitSemaphoreCount = waitSemaphores.Size();
			submitInfos[0].pWaitSemaphores = waitSemaphores.Begin();
		}		

		VkResult res = vkQueueSubmit(queue, submitInfos.Size(), submitInfos.Begin(), fence);
		n_assert(res == VK_SUCCESS);

		if (waitImmediately && fence != VK_NULL_HANDLE)
		{
			res = vkWaitForFences(this->device, 1, &fence, true, UINT64_MAX);
			n_assert(res == VK_SUCCESS);
		}
	}

	// clear the submit infos
	buffers.Clear();
	waitSemaphores.Clear();
	waitFlags.Clear();
	signalSemaphores.Clear();
	submitInfos.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void VkSubContextHandler::SubmitImmediate(CoreGraphicsQueueType type, VkCommandBuffer cmds, VkSemaphore waitSemaphore, VkPipelineStageFlags waitFlags, VkSemaphore signalSemaphore, VkFence fence, bool waitImmediately)
{
	VkQueue queue = VK_NULL_HANDLE;
	switch (type)
	{
	case GraphicsQueueType:
		queue = this->drawQueues[this->currentDrawQueue];
		break;
	case ComputeQueueType:
		queue = this->computeQueues[this->currentComputeQueue];
		break;
	case TransferQueueType:
		queue = this->transferQueues[this->currentTransferQueue];
		break;
	case SparseQueueType:
		queue = this->sparseQueues[this->currentSparseQueue];
		break;
	}

	VkSubmitInfo info =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0, nullptr, nullptr,	// wait
		0, nullptr,				// cmd buf
		0, nullptr				// signal
	};


	// if command buffer is present, add to 
	if (cmds != VK_NULL_HANDLE)
	{
		info.commandBufferCount = 1;
		info.pCommandBuffers = &cmds;
	}

	// if we have wait semaphores, add both flags and the semaphore it self
	if (waitSemaphore != VK_NULL_HANDLE)
	{
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &waitSemaphore;
		info.pWaitDstStageMask = &waitFlags;
	}

	// finally add signal semaphore if present
	if (signalSemaphore != VK_NULL_HANDLE)
	{
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &signalSemaphore;
	}

	// submit
	VkResult res = vkQueueSubmit(queue, 1, &info, fence);
	n_assert(res == VK_SUCCESS);

	if (waitImmediately && fence != VK_NULL_HANDLE)
	{
		res = vkWaitForFences(this->device, 1, &fence, true, UINT64_MAX);
		n_assert(res == VK_SUCCESS);
	}
}

//------------------------------------------------------------------------------
/**
*/
void
VkSubContextHandler::WaitIdle(const CoreGraphicsQueueType type)
{
	Util::FixedArray<VkQueue>* list = nullptr;
	switch (type)
	{
	case GraphicsQueueType:
		list = &this->drawQueues;
		break;
	case ComputeQueueType:
		list = &this->computeQueues;
		break;
	case TransferQueueType:
		list = &this->transferQueues;
		break;
	case SparseQueueType:
		list = &this->sparseQueues;
		break;
	}
	for (IndexT i = 0; i < list->Size(); i++)
	{
		VkResult res = vkQueueWaitIdle((*list)[i]);
		n_assert(res == VK_SUCCESS);
	}
}

//------------------------------------------------------------------------------
/**
*/
VkQueue
VkSubContextHandler::GetQueue(const CoreGraphicsQueueType type)
{
	switch (type)
	{
	case GraphicsQueueType:
		return this->drawQueues[this->currentDrawQueue];
	case ComputeQueueType:
		return this->computeQueues[this->currentComputeQueue];
	case TransferQueueType:
		return this->transferQueues[this->currentTransferQueue];
	case SparseQueueType:
		return this->sparseQueues[this->currentSparseQueue];
	default:
		n_error("Invalid queue type %d", type);
		return VK_NULL_HANDLE;
	}
}

} // namespace Vulkan
