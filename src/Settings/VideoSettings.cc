/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VideoSettings.h"
#include "VideoManager.h"

#include <QtCore/QVariantList>

#ifdef QGC_GST_STREAMING
#include "GStreamer.h"
#endif
#ifndef QGC_DISABLE_UVC
#include "UVCReceiver.h"
#endif

DECLARE_SETTINGGROUP(Video, "Video")
{
    // Setup enum values for videoSource settings into meta data
    QVariantList videoSourceList;
#if defined(QGC_GST_STREAMING) || defined(QGC_QT_STREAMING)
    videoSourceList.append(videoSourceRTSP);
    videoSourceList.append(videoSourceUDPH264);
    videoSourceList.append(videoSourceUDPH265);
    videoSourceList.append(videoSourceTCP);
    videoSourceList.append(videoSourceMPEGTS);
    videoSourceList.append(videoSource3DRSolo);
    videoSourceList.append(videoSourceParrotDiscovery);
    videoSourceList.append(videoSourceYuneecMantisG);

    #ifdef QGC_HERELINK_AIRUNIT_VIDEO
        videoSourceList.append(videoSourceHerelinkAirUnit);
    #else
        videoSourceList.append(videoSourceHerelinkHotspot);
    #endif
#endif
#ifndef QGC_DISABLE_UVC
    videoSourceList.append(UVCReceiver::getDeviceNameList());
#endif
    if (videoSourceList.count() == 0) {
        _noVideo = true;
        videoSourceList.append(videoSourceNoVideo);
        setVisible(false);
    } else {
        videoSourceList.insert(0, videoDisabled);
    }

    // make translated strings
    QStringList videoSourceCookedList;
    for (const QVariant& videoSource: videoSourceList) {
        videoSourceCookedList.append( VideoSettings::tr(videoSource.toString().toStdString().c_str()) );
    }

    _nameToMetaDataMap[videoSourceName]->setEnumInfo(videoSourceCookedList, videoSourceList);

    _setForceVideoDecodeList();

    // Set default value for videoSource
    _setDefaults();
}

void VideoSettings::_setDefaults()
{
    if (_noVideo) {
        _nameToMetaDataMap[videoSourceName]->setRawDefaultValue(videoSourceNoVideo);
    } else {
        _nameToMetaDataMap[videoSourceName]->setRawDefaultValue(videoDisabled);
    }
}

DECLARE_SETTINGSFACT(VideoSettings, aspectRatio)
DECLARE_SETTINGSFACT(VideoSettings, videoFit)
DECLARE_SETTINGSFACT(VideoSettings, gridLines)
DECLARE_SETTINGSFACT(VideoSettings, showRecControl)
DECLARE_SETTINGSFACT(VideoSettings, recordingFormat)
DECLARE_SETTINGSFACT(VideoSettings, maxVideoSize)
DECLARE_SETTINGSFACT(VideoSettings, enableStorageLimit)
DECLARE_SETTINGSFACT(VideoSettings, streamEnabled)
DECLARE_SETTINGSFACT(VideoSettings, disableWhenDisarmed)

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, videoSource)
{
    if (!_videoSourceFact) {
        _videoSourceFact = _createSettingsFact(videoSourceName);
        //-- Check for sources no longer available
        if(!_videoSourceFact->enumValues().contains(_videoSourceFact->rawValue().toString())) {
            if (_noVideo) {
                _videoSourceFact->setRawValue(videoSourceNoVideo);
            } else {
                _videoSourceFact->setRawValue(videoDisabled);
            }
        }
        connect(_videoSourceFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _videoSourceFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, forceVideoDecoder)
{
    if (!_forceVideoDecoderFact) {
        _forceVideoDecoderFact = _createSettingsFact(forceVideoDecoderName);

        _forceVideoDecoderFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
        );

        connect(_forceVideoDecoderFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _forceVideoDecoderFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, lowLatencyMode)
{
    if (!_lowLatencyModeFact) {
        _lowLatencyModeFact = _createSettingsFact(lowLatencyModeName);

        _lowLatencyModeFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
        );

        connect(_lowLatencyModeFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _lowLatencyModeFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtspTimeout)
{
    if (!_rtspTimeoutFact) {
        _rtspTimeoutFact = _createSettingsFact(rtspTimeoutName);

        _rtspTimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
        );

        connect(_rtspTimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtspTimeoutFact;
}
DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp1Timeout)
{
    if (!_rtsp1TimeoutFact) {
        _rtsp1TimeoutFact = _createSettingsFact(rtsp1TimeoutName);

        _rtsp1TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp1TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp1TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp2Timeout)
{
    if (!_rtsp2TimeoutFact) {
        _rtsp2TimeoutFact = _createSettingsFact(rtsp2TimeoutName);

        _rtsp2TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp2TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp2TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp3Timeout)
{
    if (!_rtsp3TimeoutFact) {
        _rtsp3TimeoutFact = _createSettingsFact(rtsp3TimeoutName);

        _rtsp3TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp3TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp3TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp4Timeout)
{
    if (!_rtsp4TimeoutFact) {
        _rtsp4TimeoutFact = _createSettingsFact(rtsp4TimeoutName);

        _rtsp4TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp4TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp4TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp5Timeout)
{
    if (!_rtsp5TimeoutFact) {
        _rtsp5TimeoutFact = _createSettingsFact(rtsp5TimeoutName);

        _rtsp5TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp5TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp5TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp6Timeout)
{
    if (!_rtsp6TimeoutFact) {
        _rtsp6TimeoutFact = _createSettingsFact(rtsp6TimeoutName);

        _rtsp6TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp6TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp6TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp7Timeout)
{
    if (!_rtsp7TimeoutFact) {
        _rtsp7TimeoutFact = _createSettingsFact(rtsp7TimeoutName);

        _rtsp7TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp7TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp7TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp8Timeout)
{
    if (!_rtsp8TimeoutFact) {
        _rtsp8TimeoutFact = _createSettingsFact(rtsp8TimeoutName);

        _rtsp8TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp8TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp8TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp9Timeout)
{
    if (!_rtsp9TimeoutFact) {
        _rtsp9TimeoutFact = _createSettingsFact(rtsp9TimeoutName);

        _rtsp9TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp9TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp9TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp10Timeout)
{
    if (!_rtsp10TimeoutFact) {
        _rtsp10TimeoutFact = _createSettingsFact(rtsp10TimeoutName);

        _rtsp10TimeoutFact->setVisible(
#ifdef QGC_GST_STREAMING
            true
#else
            false
#endif
            );

        connect(_rtsp10TimeoutFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp10TimeoutFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, udpUrl)
{
    if (!_udpUrlFact) {
        _udpUrlFact = _createSettingsFact(udpUrlName);
        connect(_udpUrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _udpUrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtspUrl)
{
    if (!_rtspUrlFact) {
        _rtspUrlFact = _createSettingsFact(rtspUrlName);
        connect(_rtspUrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtspUrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp1Url)
{
    if (!_rtsp1UrlFact) {
        _rtsp1UrlFact = _createSettingsFact(rtsp1UrlName);
        connect(_rtsp1UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp1UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp1Ip)
{
    if (!_rtsp1IpFact) {
        _rtsp1IpFact = _createSettingsFact(rtsp1IpName);
        connect(_rtsp1IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp1IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp1StreamNo)
{
    if (!_rtsp1StreamNoFact) {
        _rtsp1StreamNoFact = _createSettingsFact(rtsp1StreamNoName);
        connect(_rtsp1StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp1StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp2Url)
{
    if (!_rtsp2UrlFact) {
        _rtsp2UrlFact = _createSettingsFact(rtsp2UrlName);
        connect(_rtsp2UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp2UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp2Ip)
{
    if (!_rtsp2IpFact) {
        _rtsp2IpFact = _createSettingsFact(rtsp2IpName);
        connect(_rtsp2IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp2IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp2StreamNo)
{
    if (!_rtsp2StreamNoFact) {
        _rtsp2StreamNoFact = _createSettingsFact(rtsp2StreamNoName);
        connect(_rtsp2StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp2StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp3Url)
{
    if (!_rtsp3UrlFact) {
        _rtsp3UrlFact = _createSettingsFact(rtsp3UrlName);
        connect(_rtsp3UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp3UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp3Ip)
{
    if (!_rtsp3IpFact) {
        _rtsp3IpFact = _createSettingsFact(rtsp3IpName);
        connect(_rtsp3IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp3IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp3StreamNo)
{
    if (!_rtsp3StreamNoFact) {
        _rtsp3StreamNoFact = _createSettingsFact(rtsp3StreamNoName);
        connect(_rtsp3StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp3StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp4Url)
{
    if (!_rtsp4UrlFact) {
        _rtsp4UrlFact = _createSettingsFact(rtsp4UrlName);
        connect(_rtsp4UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp4UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp4Ip)
{
    if (!_rtsp4IpFact) {
        _rtsp4IpFact = _createSettingsFact(rtsp4IpName);
        connect(_rtsp4IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp4IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp4StreamNo)
{
    if (!_rtsp4StreamNoFact) {
        _rtsp4StreamNoFact = _createSettingsFact(rtsp4StreamNoName);
        connect(_rtsp4StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp4StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp5Url)
{
    if (!_rtsp5UrlFact) {
        _rtsp5UrlFact = _createSettingsFact(rtsp5UrlName);
        connect(_rtsp5UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp5UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp5Ip)
{
    if (!_rtsp5IpFact) {
        _rtsp5IpFact = _createSettingsFact(rtsp5IpName);
        connect(_rtsp5IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp5IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp5StreamNo)
{
    if (!_rtsp5StreamNoFact) {
        _rtsp5StreamNoFact = _createSettingsFact(rtsp5StreamNoName);
        connect(_rtsp5StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp5StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp6Url)
{
    if (!_rtsp6UrlFact) {
        _rtsp6UrlFact = _createSettingsFact(rtsp6UrlName);
        connect(_rtsp6UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp6UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp6Ip)
{
    if (!_rtsp6IpFact) {
        _rtsp6IpFact = _createSettingsFact(rtsp6IpName);
        connect(_rtsp6IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp6IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp6StreamNo)
{
    if (!_rtsp6StreamNoFact) {
        _rtsp6StreamNoFact = _createSettingsFact(rtsp6StreamNoName);
        connect(_rtsp6StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp6StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp7Url)
{
    if (!_rtsp7UrlFact) {
        _rtsp7UrlFact = _createSettingsFact(rtsp7UrlName);
        connect(_rtsp7UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp7UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp7Ip)
{
    if (!_rtsp7IpFact) {
        _rtsp7IpFact = _createSettingsFact(rtsp7IpName);
        connect(_rtsp7IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp7IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp7StreamNo)
{
    if (!_rtsp7StreamNoFact) {
        _rtsp7StreamNoFact = _createSettingsFact(rtsp7StreamNoName);
        connect(_rtsp7StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp7StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp8Url)
{
    if (!_rtsp8UrlFact) {
        _rtsp8UrlFact = _createSettingsFact(rtsp8UrlName);
        connect(_rtsp8UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp8UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp8Ip)
{
    if (!_rtsp8IpFact) {
        _rtsp8IpFact = _createSettingsFact(rtsp8IpName);
        connect(_rtsp8IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp8IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp8StreamNo)
{
    if (!_rtsp8StreamNoFact) {
        _rtsp8StreamNoFact = _createSettingsFact(rtsp8StreamNoName);
        connect(_rtsp8StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp8StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp9Url)
{
    if (!_rtsp9UrlFact) {
        _rtsp9UrlFact = _createSettingsFact(rtsp9UrlName);
        connect(_rtsp9UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp9UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp9Ip)
{
    if (!_rtsp9IpFact) {
        _rtsp9IpFact = _createSettingsFact(rtsp9IpName);
        connect(_rtsp9IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp9IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp9StreamNo)
{
    if (!_rtsp9StreamNoFact) {
        _rtsp9StreamNoFact = _createSettingsFact(rtsp9StreamNoName);
        connect(_rtsp9StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp9StreamNoFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp10Url)
{
    if (!_rtsp10UrlFact) {
        _rtsp10UrlFact = _createSettingsFact(rtsp10UrlName);
        connect(_rtsp10UrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp10UrlFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp10Ip)
{
    if (!_rtsp10IpFact) {
        _rtsp10IpFact = _createSettingsFact(rtsp10IpName);
        connect(_rtsp10IpFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp10IpFact;
}

DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, rtsp10StreamNo)
{
    if (!_rtsp10StreamNoFact) {
        _rtsp10StreamNoFact = _createSettingsFact(rtsp10StreamNoName);
        connect(_rtsp10StreamNoFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _rtsp10StreamNoFact;
}


DECLARE_SETTINGSFACT_NO_FUNC(VideoSettings, tcpUrl)
{
    if (!_tcpUrlFact) {
        _tcpUrlFact = _createSettingsFact(tcpUrlName);
        connect(_tcpUrlFact, &Fact::valueChanged, this, &VideoSettings::_configChanged);
    }
    return _tcpUrlFact;
}

bool VideoSettings::streamConfigured(void)
{
    //-- First, check if it's autoconfigured
    if(VideoManager::instance()->autoStreamConfigured()) {
        qCDebug(VideoManagerLog) << "Stream auto configured";
        return true;
    }
    //-- Check if it's disabled
    QString vSource = videoSource()->rawValue().toString();
    if(vSource == videoSourceNoVideo || vSource == videoDisabled) {
        return false;
    }
    //-- If UDP, check for URL
    if(vSource == videoSourceUDPH264 || vSource == videoSourceUDPH265) {
        qCDebug(VideoManagerLog) << "Testing configuration for UDP Stream:" << udpUrl()->rawValue().toString();
        return !udpUrl()->rawValue().toString().isEmpty();
    }
    //-- If RTSP, check for URL
    if(vSource == videoSourceRTSP) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtspUrl()->rawValue().toString();
        return !rtspUrl()->rawValue().toString().isEmpty();
    }
    //-- If RTSP, check for URL
    if(vSource == videoSourceRTSP1) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp1Url()->rawValue().toString();
        return !rtsp1Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP2) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp2Url()->rawValue().toString();
        return !rtsp2Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP3) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp3Url()->rawValue().toString();
        return !rtsp3Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP4) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp4Url()->rawValue().toString();
        return !rtsp4Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP5) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp5Url()->rawValue().toString();
        return !rtsp5Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP6) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp6Url()->rawValue().toString();
        return !rtsp6Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP7) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp7Url()->rawValue().toString();
        return !rtsp7Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP8) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp8Url()->rawValue().toString();
        return !rtsp8Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP9) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp9Url()->rawValue().toString();
        return !rtsp9Url()->rawValue().toString().isEmpty();
    }
    if(vSource == videoSourceRTSP10) {
        qCDebug(VideoManagerLog) << "Testing configuration for RTSP Stream:" << rtsp10Url()->rawValue().toString();
        return !rtsp10Url()->rawValue().toString().isEmpty();
    }

    //-- If TCP, check for URL
    if(vSource == videoSourceTCP) {
        qCDebug(VideoManagerLog) << "Testing configuration for TCP Stream:" << tcpUrl()->rawValue().toString();
        return !tcpUrl()->rawValue().toString().isEmpty();
    }
    //-- If MPEG-TS, check for URL
    if(vSource == videoSourceMPEGTS) {
        qCDebug(VideoManagerLog) << "Testing configuration for MPEG-TS Stream:" << udpUrl()->rawValue().toString();
        return !udpUrl()->rawValue().toString().isEmpty();
    }
    //-- If Herelink Air unit, good to go
    if(vSource == videoSourceHerelinkAirUnit) {
        qCDebug(VideoManagerLog) << "Stream configured for Herelink Air Unit";
        return true;
    }
    //-- If Herelink Hotspot, good to go
    if(vSource == videoSourceHerelinkHotspot) {
        qCDebug(VideoManagerLog) << "Stream configured for Herelink Hotspot";
        return true;
    }
#ifndef QGC_DISABLE_UVC
    if (UVCReceiver::enabled() && UVCReceiver::deviceExists(vSource)) {
        qCDebug(VideoManagerLog) << "Stream configured for UVC";
        return true;
    }
#endif
    return false;
}

void VideoSettings::_configChanged(QVariant)
{
    emit streamConfiguredChanged(streamConfigured());
}

void VideoSettings::_setForceVideoDecodeList()
{
#ifdef QGC_GST_STREAMING
    static const QList<GStreamer::VideoDecoderOptions> removeForceVideoDecodeList{
#if defined(Q_OS_ANDROID)
    GStreamer::VideoDecoderOptions::ForceVideoDecoderDirectX3D,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderVideoToolbox,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderVAAPI,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderNVIDIA,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderIntel,
#elif defined(Q_OS_LINUX)
    GStreamer::VideoDecoderOptions::ForceVideoDecoderDirectX3D,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderVideoToolbox,
#elif defined(Q_OS_WIN)
    GStreamer::VideoDecoderOptions::ForceVideoDecoderVideoToolbox,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderVulkan,
#elif defined(Q_OS_MACOS)
    GStreamer::VideoDecoderOptions::ForceVideoDecoderDirectX3D,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderVAAPI,
#elif defined(Q_OS_IOS)
    GStreamer::VideoDecoderOptions::ForceVideoDecoderDirectX3D,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderVAAPI,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderNVIDIA,
    GStreamer::VideoDecoderOptions::ForceVideoDecoderIntel,
#endif
    };

    for (const auto &value : removeForceVideoDecodeList) {
        _nameToMetaDataMap[forceVideoDecoderName]->removeEnumInfo(value);
    }
#endif
}
