#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // 深度缓存区，OpenGL默认是（-1.0， 1.0）Vulakn为（0.0， 1.0）
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#include <chrono>
#include <unordered_map>

#include <vulkanDataStructure/vulkanDataStructure.h>
#include <vulkanImageResource/vulkanResource.h>
#include <vulkanRender/vulkanUniformBuffer.h>
#include <vulkanCommandPool/vulkanCommandPool.h>
#include <vulkanRender/vulkanGraphicsPipeline.h>
#include <vulkanRenderObject/vulkanRenderObject.h>
#include <vulkanRenderObject/vulkanRenderBackgroundObject.h>
#include <vulkanRender/vulkanRenderPass.h>
#include <vulkanRenderObject/vulkanVolumeCloud.h>
#include <vulkanRenderObject/vulkanSkyBox.h>
#include <camera/camera.h>
class vulkanContext
{
private:
const uint32_t WIDTH = 1080;
const uint32_t HEIGHT = 720;

/** 同时渲染多帧的最大帧数*/
const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;  // Build Configuration: Release
#else
const bool enableValidationLayers = true;   // Build Configuration: Debug
#endif

 
public:
    bool firstMouse = true;
    float lastX = 0.0f;
    float lastY = 0.0f;
    bool keys[1024] = { false };
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float totalTime=0.0f;
    int framenumber=0.0;
    const int MAXFRAMECOUNT=128;
    Camera camera;

    /* data */
    GLFWwindow* window;									// Window 渲染桌面

    VkInstance instance;								// 链接程序的Vulkan实例
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;								// 链接桌面和Vulkan的实例

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	// 物理显卡硬件
    VkDevice device;									// 逻辑硬件，对接物理硬件

    VkQueue graphicsQueue;                              // 显卡的队列
    VkQueue presentQueue;                               // 显示器的队列
    VkQueue computeQueue;                               // 计算队列

    VkSwapchainKHR swapChain;							// 缓存渲染图像队列，同步到显示器
    std::vector<VkImage> swapChainImages;				// 渲染图像队列
    VkFormat swapChainImageFormat;						// 渲染图像格式
    VkExtent2D swapChainExtent;							// 渲染图像范围
    std::vector<VkImageView> swapChainImageViews;		// 渲染图像队列对应的视图队列
    std::vector<VkFramebuffer> swapChainFramebuffers;	// 渲染图像队列对应的帧缓存队列

    vulkanDepthResource depthImage;                     //深度图像

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"}; // VK_LAYER_KHRONOS_validation这个是固定的，不能重命名
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    bool framebufferResized = false;

    std::vector<VkSemaphore> imageAvailableSemaphores;	// 图像是否完成的信号
    std::vector<VkSemaphore> renderFinishedSemaphores;	// 渲染是否结束的信号
    std::vector<VkFence> inFlightFences;				// 围栏，下一帧渲染前等待上一帧全部渲染完成

    uint32_t currentFrame = 0; 

    vulkanCommandPool* commandpool;

    vulkanRenderPass* renderPass;

    //体积云的低分辨率采样通道
    vulkanRenderPass* halfResPass;

    VkImage halfRescloud;
    VkImageView halfRescloudview;
    VkFramebuffer halfRescloudFramebufferA;
    VkFramebuffer halfRescloudFramebufferB;
    uint32_t downscaleFactor;

    //体积云的合并通道
    vulkanRenderPass*  compositeRenderPass;

    vulkanGraphicsPipeline* backgroundPipeline;

    vulkanGraphicsPipeline* objectPipeline;

    vulkanGraphicsPipeline* skyboxPipeline;

    vulkanGraphicsPipeline* volumeCloudPipeline;

    vulkanGraphicsPipeline* compositePipeline;

    std::vector<VkDescriptorSet> backgroundDescriptorSets;		// 描述符集合，描述符使得着色器可以自由的访问缓存和图片

    VkDescriptorPool compositeDescriptorPool;
    std::vector<VkDescriptorSet> compositeDescriptorSets;

    vulkanRenderBackgroundObject* background=nullptr;

    std::vector<vulkanRenderObject*> stageObject;

    vulkanVolumeCloud* volumecloud;

    vulkanUniformBuffer* volumeclouduniform;

    vulkanSkyBox* skybox; 
    vulkanUniformBuffer* skyUniform;
    
    vulkanContext();
    ~vulkanContext();

    /** 初始化GUI渲染窗口*/
    void initWindow();
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    void createInstance();
    void createDebugMessenger();
    void createWindowsSurface();
    void selectPhysicalDevice();     
    void createLogicalDevice();   
    void createSwapChain();   
    void createImageViews();
    void createImageView(VkImageView &outImageView, const VkImage &inImage, const VkFormat &inFormat, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t levelCount = 1);

    void cleanupSwapChain();
    void recreateSwapChain();

    void createFramebuffers();

    void createSyncObjects();

    void mainTick(const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer, const vulkancubemaperesource &cubemap);

    void drawFrame(const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer, const vulkancubemaperesource &cubemap);

    void updateUniformBuffer(uint32_t currentImage,const vulkanUniformBuffer &meshUniformBuffer, const vulkanUniformBuffer &viewUniformBuffer, const vulkancubemaperesource &cubemap);

    void recordCommandBuffer(VkRenderPass renderpass, VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool checkValidationLayerSupport();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
     static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    // 实例回调函数
    void processMouseMovement(float xpos, float ypos);
    void processMouseScroll(float yoffset);

    void processKeyInput(int key, int action);
    void updateCameraPosition();
};


#endif