#pragma once
#include "Base/Handle.h"
#include "Device/Devicer.h"
#include "Surface.h"
#include <vector>

namespace Vulkan {

class RenderPass : public Handle<VkRenderPass>, public Devicer {

public:
    RenderPass(const Device* device);
    ~RenderPass();
    void create() override;
    void destroy() override;
    void setColorFormat(VkFormat format);
    void setDepthFormat(VkFormat format);
    void setDepthEnable(bool depthEnable);
    void setOverlayEnable(bool overlayEnable);
    void addClearValue(VkClearValue clearValue);
    void setClearValue(VkClearValue clearValue);

    VkRenderPassCreateInfo createInfo = {};
    VkRenderPassBeginInfo beginInfo = {};

private:
    VkAttachmentDescription colorAttachmentDescription = {};
    VkAttachmentDescription depthAttachmentDescription = {};
    VkAttachmentReference colorAttachmentReference = {};
    VkAttachmentReference depthAttachmentRef = {};
    VkSubpassDescription subpassDescription = {};
    std::vector<VkClearValue> clearValues;
    std::vector<VkSubpassDependency> subpassDependencies;
    std::vector<VkSubpassDescription> subpassDescriptions;
    bool depthEnable = false;
    bool overlayEnable = false;
};

} // Vulkan
