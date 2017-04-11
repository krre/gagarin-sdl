#pragma once
#include "../Base/Handle.h"
#include "../Device/Devicer.h"
#include <vector>

namespace Vulkan {

class DescriptorSetLayout : public Handle<VkDescriptorSetLayout>, public Devicer {

public:
    DescriptorSetLayout(Device* device = nullptr);
    ~DescriptorSetLayout();
    void create() override;
    void destroy() override;
    void addLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding);

    VkDescriptorSetLayoutCreateInfo createInfo = {};

private:
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

} // Vulkan
