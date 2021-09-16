#include "vk/vk_resources.h"

namespace neko::vk
{
void LogicalDevice::Init()
{
	const VkResources* vkObj                     = VkResources::Inst;
	const QueueFamilyIndices& queueFamilyIndices = vkObj->gpu.GetQueueFamilyIndices();

	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::uint32_t uniqueQueueFamilies[] = {
		queueFamilyIndices.graphicsFamily,
		queueFamilyIndices.presentFamily,
		queueFamilyIndices.computeFamily,
	};

	for (std::uint32_t queueFamily : uniqueQueueFamilies)
	{
		const auto it = std::find_if(queueCreateInfos.cbegin(),
			queueCreateInfos.cend(),
			[queueFamily](const VkDeviceQueueCreateInfo createInfo)
			{ return createInfo.queueFamilyIndex == queueFamily; });
		if (it != queueCreateInfos.cend()) continue;

		VkDeviceQueueCreateInfo queueCreateInfo {};
		queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount       = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Disabled for now
	VkPhysicalDeviceFeatures deviceFeatures {};
	//deviceFeatures.samplerAnisotropy = VK_TRUE;

#ifdef NEKO_RAYTRACING
	// Check if raytracing is available along with needed features
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT bufferFeatures {};
    bufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
    bufferFeatures.pNext = nullptr;
    bufferFeatures.bufferDeviceAddress              = VK_TRUE;
    bufferFeatures.bufferDeviceAddressCaptureReplay = VK_FALSE;
    bufferFeatures.bufferDeviceAddressMultiDevice   = VK_FALSE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures {};
    rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingFeatures.pNext = &bufferFeatures;
    rayTracingFeatures.rayTracingPipeline = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationFeatures {};
    accelerationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationFeatures.pNext                              = &rayTracingFeatures;
    accelerationFeatures.accelerationStructure              = VK_TRUE;
    accelerationFeatures.accelerationStructureCaptureReplay = VK_TRUE;
    accelerationFeatures.accelerationStructureIndirectBuild = VK_FALSE;
    accelerationFeatures.accelerationStructureHostCommands  = VK_FALSE;
    accelerationFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE;
#endif

	// Device creation information
	VkDeviceCreateInfo createInfo {};
	createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
#ifdef NEKO_RAYTRACING
	createInfo.pNext                = &accelerationFeatures;
#endif
	createInfo.pQueueCreateInfos    = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures     = nullptr;
	createInfo.pEnabledFeatures     = &deviceFeatures;

#ifdef VALIDATION_LAYERS
	createInfo.enabledLayerCount   = static_cast<std::uint32_t>(kValidationLayers.size());
	createInfo.ppEnabledLayerNames = kValidationLayers.data();
#else
	createInfo.enabledLayerCount = 0;
#endif

	createInfo.enabledExtensionCount   = static_cast<std::uint32_t>(kDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

#ifdef NEKO_RAYTRACING
	// If a pNext(Chain) has been passed, we need to add it to the device creation info
	void* pNextEnabledFeatures = vkObj->gpu.GetEnabledFeatures();
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 {};
	if (pNextEnabledFeatures)
	{
		physicalDeviceFeatures2.sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		physicalDeviceFeatures2.features  = deviceFeatures;
		physicalDeviceFeatures2.pNext     = pNextEnabledFeatures;
        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext            = &physicalDeviceFeatures2;
	}
#endif

	// Finally we're ready to create a new device
	const VkResult res = vkCreateDevice(vkObj->gpu, &createInfo, nullptr, &device_);
	vkCheckError(res, "Failed to create logical device!");

	vkGetDeviceQueue(device_, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue_);
	vkGetDeviceQueue(device_, queueFamilyIndices.presentFamily, 0, &presentQueue_);
	vkGetDeviceQueue(device_, queueFamilyIndices.computeFamily, 0, &computeQueue_);
}

void LogicalDevice::Destroy() const
{
	vkDeviceWaitIdle(device_);
	vkDestroyDevice(device_, nullptr);
}
}    // namespace neko::vk