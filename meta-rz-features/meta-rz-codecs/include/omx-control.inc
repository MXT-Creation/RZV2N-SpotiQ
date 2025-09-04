## Additional configuration in OMX module, include
# USE_H264D_OMX for OMX Media Component H264 OMX Library
USE_H264D_OMX = "${@bb.utils.contains('COMBINED_FEATURES', 'hwh264dec', '1', '', d)}"

# USE_H264E_OMX for OMX Media Component H264 Encoder Library
USE_H264E_OMX = "${@bb.utils.contains('COMBINED_FEATURES', 'hwh264enc', '1', '', d)}"

# USE_H265D_OMX for OMX Media Component H265 Decoder Library
USE_H265D_OMX = "${@bb.utils.contains('COMBINED_FEATURES', 'hwh265dec', '1', '', d)}"

# USE_H265E_OMX for OMX Media Component H265 Encoder Library
USE_H265E_OMX = "${@bb.utils.contains('COMBINED_FEATURES', 'hwh265enc', '1', '', d)}"

# USE_OMX_VIDEO_DEC for Video Decoder Common Libs
USE_VIDEO_DEC = "${@'1' if '1' in '${USE_H264D_OMX} ${USE_H265D_OMX}' else '0'}"

# USE_OMX_VIDEO_DEC for Video Encoder Common Libs
USE_VIDEO_ENC = "${@'1' if '1' in '${USE_H264E_OMX} ${USE_H265E_OMX}' else '0'}"

# USE_VIDEO_OMX for Enable/Disable OMX Video
USE_VIDEO_OMX = "${@'1' if '1' in '${USE_VIDEO_DEC} ${USE_VIDEO_ENC}' else '0'}"
