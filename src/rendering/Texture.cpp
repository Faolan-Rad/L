#include "Texture.h"

#include <string.h>
#include "GPUBuffer.h"
#include "../dev/debug.h"
#include "../math/math.h"

using namespace L;

Texture::Texture(uint32_t width, uint32_t height, VkFormat format, const void* data, size_t size)
  : _width(width), _height(height), _format(format), _layout(VK_IMAGE_LAYOUT_UNDEFINED) {
  const bool depth_texture(Vulkan::is_depth_format(_format));
  const bool compressed_texture(Vulkan::is_block_format(_format));
  const bool cubemap(_width*6==_height);

  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = _width;
  imageInfo.extent.height = _height = cubemap ? _width : _height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = _layer_count = cubemap ? 6 : 1;
  imageInfo.format = _format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = _layout;
  imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if(depth_texture) imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  if(!depth_texture && !compressed_texture) imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  if(cubemap) imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

  L_VK_CHECKED(vkCreateImage(Vulkan::device(), &imageInfo, nullptr, &_image));

  VkMemoryRequirements requirements;
  vkGetImageMemoryRequirements(Vulkan::device(), _image, &requirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex = Vulkan::find_memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  L_VK_CHECKED(vkAllocateMemory(Vulkan::device(), &allocInfo, nullptr, &_memory));

  vkBindImageMemory(Vulkan::device(), _image, _memory, 0);

  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = _image;
  viewInfo.viewType = cubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = _format;
  viewInfo.subresourceRange.aspectMask = depth_texture ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = _layer_count;

  L_VK_CHECKED(vkCreateImageView(Vulkan::device(), &viewInfo, nullptr, &_view));

  if(data) { // Optional loading of texture data
    load(data, size);
  }
}
Texture::~Texture() {
  Vulkan::destroy_image(_image, _memory);
}

void Texture::load(const void* data, size_t size, VkOffset3D offset, VkExtent3D extent) {
  GPUBuffer buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

  void* mapped_mem;
  vkMapMemory(Vulkan::device(), buffer, 0, size, 0, &mapped_mem);
  memcpy(mapped_mem, data, size);
  vkUnmapMemory(Vulkan::device(), buffer);

  VkCommandBuffer cmd(Vulkan::begin_command_buffer());
  transition_layout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = _layer_count;

  region.imageOffset = offset;
  region.imageExtent = extent;

  vkCmdCopyBufferToImage(
    cmd,
    buffer,
    _image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
  );

  transition_layout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  Vulkan::end_command_buffer(cmd);
}
void Texture::transition_layout(VkCommandBuffer cmd_buffer, VkImageLayout new_layout) {
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = _layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = _image;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = _layer_count;

  if(new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if(Vulkan::is_stencil_format(_format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkPipelineStageFlags src_stage;
  VkPipelineStageFlags dst_stage;

  if(_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if(_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if(_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if(_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    return error("Unsupported Texture layout transition");
  }

  vkCmdPipelineBarrier(cmd_buffer, src_stage, dst_stage,
    0, 0, nullptr, 0, nullptr, 1, &barrier);

  _layout = new_layout;
}

const Texture& Texture::black() {
  static const uint32_t black_color(0);
  static Texture texture(1, 1, VK_FORMAT_R8G8B8A8_UNORM, &black_color, sizeof(black_color));
  return texture;
}
