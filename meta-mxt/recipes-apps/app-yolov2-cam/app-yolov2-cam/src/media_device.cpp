#include "media_device.hpp"
#include <map>
#include <iostream>
#include <string>
#include <algorithm>

static const std::map<std::string, std::uint32_t> kMbusFormats = {
	{ "FIXED",      MEDIA_BUS_FMT_FIXED },
	{ "YUYV",       MEDIA_BUS_FMT_YUYV8_1X16 },
	{ "UYVY",       MEDIA_BUS_FMT_UYVY8_1X16 },
	{ "SBGGR8",     MEDIA_BUS_FMT_SBGGR8_1X8 },
	{ "SGBRG8",     MEDIA_BUS_FMT_SGBRG8_1X8 },
	{ "SGRBG8",     MEDIA_BUS_FMT_SGRBG8_1X8 },
	{ "SRGGB8",     MEDIA_BUS_FMT_SRGGB8_1X8 },
	{ "SBGGR10",    MEDIA_BUS_FMT_SBGGR10_1X10 },
	{ "SGBRG10",    MEDIA_BUS_FMT_SGBRG10_1X10 },
	{ "SGRBG10",    MEDIA_BUS_FMT_SGRBG10_1X10 },
	{ "SRGGB10",    MEDIA_BUS_FMT_SRGGB10_1X10 },
	{ "SBGGR12",    MEDIA_BUS_FMT_SBGGR12_1X12 },
	{ "SGBRG12",    MEDIA_BUS_FMT_SGBRG12_1X12 },
	{ "SGRBG12",    MEDIA_BUS_FMT_SGRBG12_1X12 },
	{ "SRGGB12",    MEDIA_BUS_FMT_SRGGB12_1X12 },
	{ "RBG24",      MEDIA_BUS_FMT_RBG888_1X24 },
	{ "RGB32",      MEDIA_BUS_FMT_RGB888_1X32_PADHI },
	{ "ARGB32",     MEDIA_BUS_FMT_ARGB8888_1X32 },
};


static const std::map<EntityType, std::string> kEntityNameLookup = {
    {EntityType::Sensor,            "ar1335"},
    {EntityType::VideoIsp,          "ap1302"},
    {EntityType::CsiMipiReceiver,   "csi2"},
    {EntityType::VideoCapture,      "CRU"},
};


static const std::array<EntityType, 4> kSimplePipeline {
    EntityType::Sensor, 
    EntityType::VideoIsp, 
    EntityType::CsiMipiReceiver, 
    EntityType::VideoCapture
};


MediaDevice::MediaDevice(std::string dev_name) 
{
    (void)open(dev_name);
}

MediaDevice::MediaDevice(int dev_id)
{
    auto media_device_str = std::string{"/dev/media"} + std::to_string(dev_id);
    (void)open(media_device_str);
}

bool MediaDevice::open(std::string dev_name) 
{
    if (open_) return true;

    media_ = UniquePtrMediaDevice{media_device_new(dev_name.c_str())};
    open_ = (media_.get() != nullptr);
    if (!open_) return false;

    if (!Enumerate())
    {
        media_.reset();
        open_ = false;
        return false;
    }
    return true;
}

void MediaDevice::close() 
{
    if (!open_) return;

    media_.reset();
    open_ = false;       
}

bool MediaDevice::isOpen() const 
{
    return open_;
}

bool MediaDevice::Enumerate()
{
    if (media_device_enumerate(media_.get()) < 0)
    {
        std::cerr << "Failed to enumerate media device" << std::endl;
        return false;
    }

    for (const auto& entity_type: kSimplePipeline)
    {
        const std::uint32_t entity_count = media_get_entities_count(media_.get());
        for (std::uint32_t i = 0; i < entity_count; i++) 
        { 
            struct media_entity *entity = media_get_entity(media_.get(), i);
            const struct media_entity_desc *entity_info = media_entity_get_info(entity);
            std::string entity_name{entity_info->name};       
            if (entity_name.find(kEntityNameLookup.at(entity_type)) != std::string::npos)
            {
                MediaEntity media_entity;
                media_entity.type = entity_type;
                media_entity.name = entity_name;

                pipeline_entities_.push_back(media_entity);
                break;
            }
        }
    }

    // Handling Empty pipeline
    // An empty pipeline is not useful for devices which need to configure the pipeline
    // Should we return an error?

    return true;
}

std::string MediaDevice::getVideoCaptureDevice() const 
{
  const std::uint32_t entity_count = media_get_entities_count(media_.get());

  for (std::uint32_t i = 0; i < entity_count; i++) 
  {
    struct media_entity *entity = media_get_entity(media_.get(), i);
    std::string dev_name{media_entity_get_devname(entity)};
    if (dev_name.find("/dev/video") != std::string::npos)
    {
        return dev_name;
    }
  }
  return "";
}

/*****************************************
 * Function Name : format
 * Description   : Format specification.
 * Arguments     : fmt = an object that represents the format string.
 *                 args = arguments to be formatted
 * Return value  : A string object holding the formatted result.
 ******************************************/
template <typename... Args>
static std::string format(const std::string& fmt, Args... args) 
{
  size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args...);
  std::vector<char> buf(len + 1);
  std::snprintf(&buf[0], len + 1, fmt.c_str(), args...);
  return std::string(&buf[0], &buf[0] + len);
}

static std::string createLinkString(const MediaEntity& source, const MediaEntity& sink) 
{
    const EntityPad& src_pad  = source.config.source_pad;
    const EntityPad& sink_pad = sink.config.sink_pad;

    bool invalid_pad_types     = (src_pad.type == EntityPad::PadType::NONE) || (sink_pad.type == EntityPad::PadType::NONE);
    bool invalid_entity_names  = (source.name.empty() || sink.name.empty());   

    if (invalid_entity_names || invalid_pad_types)
    {
        return "";
    }

    std::string link_str = format("\'%s\':%u -> \'%s\':%u [1]", source.name.c_str(),src_pad.index,
                                                                    sink.name.c_str(), sink_pad.index);
    std::cout << link_str << std::endl;
    return link_str;
}

static std::string createPadFormatString(const std::string& entity_name, const EntityPad& pad)
{
    if (entity_name.empty() || pad.type == EntityPad::PadType::NONE) 
    {
        return "";
    }

    // TODO - Remove this hardcode and use a lookup table to find the associated image format
    std::string img_fmt_str{"YUYV8_1X16"};
    std::string pad_fmt_str = format("\'%s\':%u [fmt:%s/%ux%u field:none]", 
                                    entity_name.c_str(), pad.index, 
                                    img_fmt_str.c_str(), pad.resolution.width, pad.resolution.height);
    

    std::cout << pad_fmt_str << std::endl;
    return pad_fmt_str;
 }

 void MediaDevice::configurePipeline(const std::vector<EntityConfig>& pipeline_config)
{
    media_reset_links(media_.get());

    // Associate the config for each entity in the pipeline with the MediaDevice pipeline
    // This is done because the API user has a very generic view of what components are in the pipeline
    for (const auto& entity_config : pipeline_config)
    {
        auto entity = std::find_if(pipeline_entities_.begin(), 
                                         pipeline_entities_.end(), 
                                        [&entity_config](const MediaEntity& entity) 
                                        {
                                            return entity.type == entity_config.type; 
                                        });

        if (entity != pipeline_entities_.end())
        {
            entity->config = entity_config;
        }   
    }

    for (std::uint32_t i = 0; i < pipeline_entities_.size(); i++)
    {
        const auto *first_entity = &pipeline_entities_[i];
        const auto *next_entity  = &pipeline_entities_[i + 1U];

        // There are no links to create if we are at the end of the pipeline
        // TODO - Check what the kernel does when we setup a link. Does ordering matter?
        // IE Can we setup the pad formats before creating the link? Or does the link need to be made BEFORE we set the pad format?
        if (next_entity != &pipeline_entities_[pipeline_entities_.size()])
        {
            auto link_str = createLinkString(*first_entity, *next_entity);
            if (!link_str.empty()) media_parse_setup_links(media_.get(), link_str.c_str());
        }

        auto sink_pad_conf_str = createPadFormatString(first_entity->name, first_entity->config.sink_pad);
        auto src_pad_conf_str  = createPadFormatString(first_entity->name, first_entity->config.source_pad);

        if (!sink_pad_conf_str.empty()) v4l2_subdev_parse_setup_formats(media_.get(), sink_pad_conf_str.c_str());
        if (!src_pad_conf_str.empty())  v4l2_subdev_parse_setup_formats(media_.get(), src_pad_conf_str.c_str());
    }
}
