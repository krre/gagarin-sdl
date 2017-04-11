#pragma once
#include "../Base/Handle.h"
#include "../Device/Devicer.h"
#include "../Surface.h"

namespace Vulkan {

class ImageView : public Handle<VkImageView>, public Devicer {

public:
    ImageView(VkImage image, Device* device = nullptr);
    ~ImageView();
    void create() override;
    void destroy() override;

    VkImageViewCreateInfo createInfo = {};
};

} // Vulkan
