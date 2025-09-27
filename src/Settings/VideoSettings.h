/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtQmlIntegration/QtQmlIntegration>

#include "SettingsGroup.h"

class VideoSettings : public SettingsGroup
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
public:
    VideoSettings(QObject* parent = nullptr);
    DEFINE_SETTING_NAME_GROUP()

    DEFINE_SETTINGFACT(videoSource)
    DEFINE_SETTINGFACT(udpUrl)
    DEFINE_SETTINGFACT(tcpUrl)
    DEFINE_SETTINGFACT(rtspUrl)

    DEFINE_SETTINGFACT(rtsp1Url)
    DEFINE_SETTINGFACT(rtsp1Ip)
    DEFINE_SETTINGFACT(rtsp1StreamNo)

    DEFINE_SETTINGFACT(rtsp2Url)
    DEFINE_SETTINGFACT(rtsp2Ip)
    DEFINE_SETTINGFACT(rtsp2StreamNo)

    DEFINE_SETTINGFACT(rtsp3Url)
    DEFINE_SETTINGFACT(rtsp3Ip)
    DEFINE_SETTINGFACT(rtsp3StreamNo)

    DEFINE_SETTINGFACT(rtsp4Url)
    DEFINE_SETTINGFACT(rtsp4Ip)
    DEFINE_SETTINGFACT(rtsp4StreamNo)

    DEFINE_SETTINGFACT(rtsp5Url)
    DEFINE_SETTINGFACT(rtsp5Ip)
    DEFINE_SETTINGFACT(rtsp5StreamNo)

    DEFINE_SETTINGFACT(rtsp6Url)
    DEFINE_SETTINGFACT(rtsp6Ip)
    DEFINE_SETTINGFACT(rtsp6StreamNo)

    DEFINE_SETTINGFACT(rtsp7Url)
    DEFINE_SETTINGFACT(rtsp7Ip)
    DEFINE_SETTINGFACT(rtsp7StreamNo)

    DEFINE_SETTINGFACT(rtsp8Url)
    DEFINE_SETTINGFACT(rtsp8Ip)
    DEFINE_SETTINGFACT(rtsp8StreamNo)

    DEFINE_SETTINGFACT(rtsp9Url)
    DEFINE_SETTINGFACT(rtsp9Ip)
    DEFINE_SETTINGFACT(rtsp9StreamNo)

    DEFINE_SETTINGFACT(rtsp10Url)
    DEFINE_SETTINGFACT(rtsp10Ip)
    DEFINE_SETTINGFACT(rtsp10StreamNo)


    DEFINE_SETTINGFACT(aspectRatio)
    DEFINE_SETTINGFACT(videoFit)
    DEFINE_SETTINGFACT(gridLines)
    DEFINE_SETTINGFACT(showRecControl)
    DEFINE_SETTINGFACT(recordingFormat)
    DEFINE_SETTINGFACT(maxVideoSize)
    DEFINE_SETTINGFACT(enableStorageLimit)
    DEFINE_SETTINGFACT(rtspTimeout)

    DEFINE_SETTINGFACT(rtsp1Timeout)
    DEFINE_SETTINGFACT(rtsp2Timeout)
    DEFINE_SETTINGFACT(rtsp3Timeout)
    DEFINE_SETTINGFACT(rtsp4Timeout)
    DEFINE_SETTINGFACT(rtsp5Timeout)
    DEFINE_SETTINGFACT(rtsp6Timeout)
    DEFINE_SETTINGFACT(rtsp7Timeout)
    DEFINE_SETTINGFACT(rtsp8Timeout)
    DEFINE_SETTINGFACT(rtsp9Timeout)
    DEFINE_SETTINGFACT(rtsp10Timeout)

    DEFINE_SETTINGFACT(streamEnabled)
    DEFINE_SETTINGFACT(disableWhenDisarmed)
    DEFINE_SETTINGFACT(lowLatencyMode)
    DEFINE_SETTINGFACT(forceVideoDecoder)

    Q_PROPERTY(bool     streamConfigured        READ streamConfigured       NOTIFY streamConfiguredChanged)
    Q_PROPERTY(QString  rtspVideoSource         READ rtspVideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp1VideoSource         READ rtsp1VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp2VideoSource         READ rtsp2VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp3VideoSource         READ rtsp3VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp4VideoSource         READ rtsp4VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp5VideoSource         READ rtsp5VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp6VideoSource         READ rtsp6VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp7VideoSource         READ rtsp7VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp8VideoSource         READ rtsp8VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp9VideoSource         READ rtsp9VideoSource        CONSTANT)
    Q_PROPERTY(QString  rtsp10VideoSource        READ rtsp10VideoSource       CONSTANT)

    Q_PROPERTY(QString  udp264VideoSource       READ udp264VideoSource      CONSTANT)
    Q_PROPERTY(QString  udp265VideoSource       READ udp265VideoSource      CONSTANT)
    Q_PROPERTY(QString  tcpVideoSource          READ tcpVideoSource         CONSTANT)
    Q_PROPERTY(QString  mpegtsVideoSource       READ mpegtsVideoSource      CONSTANT)
    Q_PROPERTY(QString  disabledVideoSource     READ disabledVideoSource    CONSTANT)

    bool     streamConfigured       ();
    QString  rtspVideoSource        () { return videoSourceRTSP; }
    QString  rtsp1VideoSource        () { return videoSourceRTSP1; }
    QString  rtsp2VideoSource        () { return videoSourceRTSP2; }
    QString  rtsp3VideoSource        () { return videoSourceRTSP3; }
    QString  rtsp4VideoSource        () { return videoSourceRTSP4; }
    QString  rtsp5VideoSource        () { return videoSourceRTSP5; }
    QString  rtsp6VideoSource        () { return videoSourceRTSP6; }
    QString  rtsp7VideoSource        () { return videoSourceRTSP7; }
    QString  rtsp8VideoSource        () { return videoSourceRTSP8; }
    QString  rtsp9VideoSource        () { return videoSourceRTSP9; }
    QString  rtsp10VideoSource       () { return videoSourceRTSP10; }


    QString  udp264VideoSource      () { return videoSourceUDPH264; }
    QString  udp265VideoSource      () { return videoSourceUDPH265; }
    QString  tcpVideoSource         () { return videoSourceTCP; }
    QString  mpegtsVideoSource      () { return videoSourceMPEGTS; }
    QString  disabledVideoSource    () { return videoDisabled; }

    static constexpr const char* videoSourceNoVideo           = QT_TRANSLATE_NOOP("VideoSettings", "No Video Available");
    static constexpr const char* videoDisabled                = QT_TRANSLATE_NOOP("VideoSettings", "Video Stream Disabled");
    static constexpr const char* videoSourceRTSP              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP Video Stream");
    static constexpr const char* videoSourceRTSP1              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP1 Video Stream");
    static constexpr const char* videoSourceRTSP2              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP2 Video Stream");
    static constexpr const char* videoSourceRTSP3              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP3 Video Stream");
    static constexpr const char* videoSourceRTSP4              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP4 Video Stream");
    static constexpr const char* videoSourceRTSP5              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP5 Video Stream");
    static constexpr const char* videoSourceRTSP6              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP6 Video Stream");
    static constexpr const char* videoSourceRTSP7              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP7 Video Stream");
    static constexpr const char* videoSourceRTSP8              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP8 Video Stream");
    static constexpr const char* videoSourceRTSP9              = QT_TRANSLATE_NOOP("VideoSettings", "RTSP9 Video Stream");
    static constexpr const char* videoSourceRTSP10             = QT_TRANSLATE_NOOP("VideoSettings", "RTSP10 Video Stream");

    static constexpr const char* videoSourceUDPH264           = QT_TRANSLATE_NOOP("VideoSettings", "UDP h.264 Video Stream");
    static constexpr const char* videoSourceUDPH265           = QT_TRANSLATE_NOOP("VideoSettings", "UDP h.265 Video Stream");
    static constexpr const char* videoSourceTCP               = QT_TRANSLATE_NOOP("VideoSettings", "TCP-MPEG2 Video Stream");
    static constexpr const char* videoSourceMPEGTS            = QT_TRANSLATE_NOOP("VideoSettings", "MPEG-TS Video Stream");
    static constexpr const char* videoSource3DRSolo           = QT_TRANSLATE_NOOP("VideoSettings", "3DR Solo (requires restart)");
    static constexpr const char* videoSourceParrotDiscovery   = QT_TRANSLATE_NOOP("VideoSettings", "Parrot Discovery");
    static constexpr const char* videoSourceYuneecMantisG     = QT_TRANSLATE_NOOP("VideoSettings", "Yuneec Mantis G");
    static constexpr const char* videoSourceHerelinkAirUnit   = QT_TRANSLATE_NOOP("VideoSettings", "Herelink AirUnit");
    static constexpr const char* videoSourceHerelinkHotspot   = QT_TRANSLATE_NOOP("VideoSettings", "Herelink Hotspot");

signals:
    void streamConfiguredChanged    (bool configured);

private slots:
    void _configChanged             (QVariant value);

private:
    void _setDefaults               ();
    void _setForceVideoDecodeList();

private:
    bool _noVideo = false;

};
