/**
 * @file media_device.hpp
 * @brief
 * @author
 * @copyright Copyright (C) 2024 IMD Technologies Ltd.
 *
 */

#ifndef MEDIA_DEVICE_H__
#define MEDIA_DEVICE_H__

#include <linux/v4l2-controls.h>
#include <cerrno>
#include <memory>
#include <cstdint>
#include <string>
#include <vector>

#include <linux/v4l2-subdev.h>
#include <linux/media.h>

extern "C" {
    #include <mediactl/mediactl.h>
    #include <mediactl/v4l2subdev.h>
}

struct MediaDeviceDeleter 
{
    void operator()(struct media_device *dev) 
    {
        media_device_unref(dev);
    }
};

using UniquePtrMediaDevice = std::unique_ptr<struct media_device, MediaDeviceDeleter>;

enum class EntityType 
{
    Sensor,
    CsiMipiReceiver,
    VideoIsp,
    VideoCapture
};

struct EntityPad 
{
    enum class PadType 
    {
        NONE,
        SOURCE,
        SINK,
    };

    struct Resolution 
    {
        uint32_t width, height;
    };

    PadType         type{PadType::NONE};
    std::uint32_t   index{0u};
    std::string     img_fmt_str{""};
    Resolution      resolution{0, 0};
};

struct EntityConfig 
{
    EntityType type;
    EntityPad  source_pad;
    EntityPad  sink_pad;
};

struct MediaEntity 
{
    EntityType type;
    std::string name{};
    EntityConfig config{};
};

class MediaDevice 
{
public:
    explicit MediaDevice(std::string dev_name);
    explicit MediaDevice(int dev_id);
    MediaDevice() = default;
    ~MediaDevice() = default;

    MediaDevice(const MediaDevice&) = delete;
    MediaDevice& operator=(const MediaDevice&) = delete;

    bool open(std::string dev_name);
    void close();

    bool isOpen() const;
    std::string getVideoCaptureDevice() const;
    std::string getEntityDevName(EntityType entity_type) const;
    void configurePipeline(const std::vector<EntityConfig>& pipeline);

private: 
    UniquePtrMediaDevice media_{nullptr};
    std::vector<MediaEntity> pipeline_entities_{};
    bool open_{false};

    bool Enumerate();
};


#endif /* MEDIA_DEVICE_H__ */