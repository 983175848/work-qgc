/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl

import QGroundControl.FactControls
import QGroundControl.Controls


SettingsPage {
    property var    _settingsManager:            QGroundControl.settingsManager
    property var    _videoManager:              QGroundControl.videoManager
    property var    _videoSettings:             _settingsManager.videoSettings
    property string _videoSource:               _videoSettings.videoSource.rawValue
    property bool   _isGST:                     _videoManager.gstreamerEnabled
    property bool   _isStreamSource:            _videoManager.isStreamSource
    property bool   _isUDP264:                  _isStreamSource && (_videoSource === _videoSettings.udp264VideoSource)
    property bool   _isUDP265:                  _isStreamSource && (_videoSource === _videoSettings.udp265VideoSource)
    property bool   _isRTSP:                    _isStreamSource && (_videoSource === _videoSettings.rtspVideoSource)
    property bool   _isTCP:                     _isStreamSource && (_videoSource === _videoSettings.tcpVideoSource)
    property bool   _isMPEGTS:                  _isStreamSource && (_videoSource === _videoSettings.mpegtsVideoSource)
    property bool   _videoAutoStreamConfig:     _videoManager.autoStreamConfigured
    property bool   _videoSourceDisabled:       _videoSource === _videoSettings.disabledVideoSource
    property real   _urlFieldWidth:             ScreenTools.defaultFontPixelWidth * 40
    property bool   _requiresUDPUrl:            _isUDP264 || _isUDP265 || _isMPEGTS

    SettingsGroupLayout {
        Layout.fillWidth:   true
        heading:            qsTr("Video Source")
        headingDescription: _videoAutoStreamConfig ? qsTr("Mavlink camera stream is automatically configured") : ""
        enabled:            !_videoAutoStreamConfig

        LabelledFactComboBox {
            Layout.fillWidth:   true
            label:              qsTr("Source")
            indexModel:         false
            fact:               _videoSettings.videoSource
            visible:            fact.visible
        }
    }

    SettingsGroupLayout {
        Layout.fillWidth:   true
        heading:            qsTr("Connection")
        visible:            !_videoSourceDisabled && !_videoAutoStreamConfig && (_isTCP || _isRTSP | _requiresUDPUrl)

        // LabelledFactTextField {
        //     Layout.fillWidth:           true
        //     textFieldPreferredWidth:    _urlFieldWidth
        //     label:                      qsTr("RTSP URL")
        //     fact:                       _videoSettings.rtspUrl
        //     visible:                    _isRTSP && _videoSettings.rtspUrl.visible
        // }
        // 唯一的rtsp视频流，需要切换其他的视频，填入并更新此处fact
        LabelledFactTextField {
            Layout.fillWidth:           true
            //textFieldPreferredWidth:    _urlFieldWidth
            label:                      qsTr("RTSP URL")
            fact:                       _videoSettings.rtspUrl
            visible:                    false//_isRTSP && _videoSettings.rtspUrl.visible
        }

        // RTSP流URL输入（分两步，统一风格）
        // 用户只需输入IP和码流编号，URL自动生成并同步到rtspUrl

        // RTSP1
        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("IP1 Address")
            fact: _videoSettings.rtsp1Ip
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("Stream No.1")
            fact: _videoSettings.rtsp1StreamNo
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("RTSP1 URL")
            fact: _videoSettings.rtsp1Url
            visible: false

            Binding {
                target: _videoSettings.rtsp1Url
                property: "rawValue"
                value: (_videoSettings.rtsp1Ip.valueString.length > 0 &&
                        _videoSettings.rtsp1StreamNo.valueString.length > 0)
                       ? "rtsp://admin:Hik12345@" + _videoSettings.rtsp1Ip.valueString +
                         ":554/Streaming/Channels/" + _videoSettings.rtsp1StreamNo.valueString
                       : ""
            }
        }

        // RTSP2
        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("IP2 Address")
            fact: _videoSettings.rtsp2Ip
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("Stream No.2")
            fact: _videoSettings.rtsp2StreamNo
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("RTSP2 URL")
            fact: _videoSettings.rtsp2Url
            visible: false

            Binding {
                target: _videoSettings.rtsp2Url
                property: "rawValue"
                value: (_videoSettings.rtsp2Ip.valueString.length > 0 &&
                        _videoSettings.rtsp2StreamNo.valueString.length > 0)
                       ? "rtsp://admin:Hik12345@" + _videoSettings.rtsp2Ip.valueString +
                         ":554/Streaming/Channels/" + _videoSettings.rtsp2StreamNo.valueString
                       : ""
            }
        }

        // RTSP3
        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("IP3 Address")
            fact: _videoSettings.rtsp3Ip
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("Stream No.3")
            fact: _videoSettings.rtsp3StreamNo
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("RTSP3 URL")
            fact: _videoSettings.rtsp3Url
            visible: false

            Binding {
                target: _videoSettings.rtsp3Url
                property: "rawValue"
                value: (_videoSettings.rtsp3Ip.valueString.length > 0 &&
                        _videoSettings.rtsp3StreamNo.valueString.length > 0)
                       ? "rtsp://admin:Hik12345@" + _videoSettings.rtsp3Ip.valueString +
                         ":554/Streaming/Channels/" + _videoSettings.rtsp3StreamNo.valueString
                       : ""
            }
        }

        // RTSP4
        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("IP4 Address")
            fact: _videoSettings.rtsp4Ip
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("Stream No.4")
            fact: _videoSettings.rtsp4StreamNo
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("RTSP4 URL")
            fact: _videoSettings.rtsp4Url
            visible: false

            Binding {
                target: _videoSettings.rtsp4Url
                property: "rawValue"
                value: (_videoSettings.rtsp4Ip.valueString.length > 0 &&
                        _videoSettings.rtsp4StreamNo.valueString.length > 0)
                       ? "rtsp://admin:Hik12345@" + _videoSettings.rtsp4Ip.valueString +
                         ":554/Streaming/Channels/" + _videoSettings.rtsp4StreamNo.valueString
                       : ""
            }
        }

        // RTSP5
        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("IP5 Address")
            fact: _videoSettings.rtsp5Ip
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("Stream No.5")
            fact: _videoSettings.rtsp5StreamNo
            visible: _isRTSP && _videoSettings.rtspUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth: true
            label: qsTr("RTSP5 URL")
            fact: _videoSettings.rtsp5Url
            visible: false

            Binding {
                target: _videoSettings.rtsp5Url
                property: "rawValue"
                value: (_videoSettings.rtsp5Ip.valueString.length > 0 &&
                        _videoSettings.rtsp5StreamNo.valueString.length > 0)
                       ? "rtsp://admin:Hik12345@" + _videoSettings.rtsp5Ip.valueString +
                         ":554/Streaming/Channels/" + _videoSettings.rtsp5StreamNo.valueString
                       : ""
            }
        }


        LabelledFactTextField {
            Layout.fillWidth:           true
            label:                      qsTr("TCP URL")
            textFieldPreferredWidth:    _urlFieldWidth
            fact:                       _videoSettings.tcpUrl
            visible:                    _isTCP && _videoSettings.tcpUrl.visible
        }

        LabelledFactTextField {
            Layout.fillWidth:           true
            textFieldPreferredWidth:    _urlFieldWidth
            label:                      qsTr("UDP URL")
            fact:                       _videoSettings.udpUrl
            visible:                    _requiresUDPUrl && _videoSettings.udpUrl.visible
        }
    }

    SettingsGroupLayout {
        Layout.fillWidth:   true
        heading:            qsTr("Settings")
        visible:            !_videoSourceDisabled

        LabelledFactTextField {
            Layout.fillWidth:   true
            label:              qsTr("Aspect Ratio")
            fact:               _videoSettings.aspectRatio
            visible:            !_videoAutoStreamConfig && _isStreamSource && _videoSettings.aspectRatio.visible
        }

        FactCheckBoxSlider {
            Layout.fillWidth:   true
            text:               qsTr("Stop recording when disarmed")
            fact:               _videoSettings.disableWhenDisarmed
            visible:            !_videoAutoStreamConfig && _isStreamSource && fact.visible
        }

        FactCheckBoxSlider {
            Layout.fillWidth:   true
            text:               qsTr("Low Latency Mode")
            fact:               _videoSettings.lowLatencyMode
            visible:            !_videoAutoStreamConfig && _isStreamSource && fact.visible && _isGST
        }

        LabelledFactComboBox {
            Layout.fillWidth:   true
            label:              qsTr("Video decode priority")
            fact:               _videoSettings.forceVideoDecoder
            visible:            fact.visible
            indexModel:         false
        }
    }

    SettingsGroupLayout {
        Layout.fillWidth: true
        heading:            qsTr("Local Video Storage")

        LabelledFactComboBox {
            Layout.fillWidth:   true
            label:              qsTr("Record File Format")
            fact:               _videoSettings.recordingFormat
            visible:            _videoSettings.recordingFormat.visible
        }

        FactCheckBoxSlider {
            Layout.fillWidth:   true
            text:               qsTr("Auto-Delete Saved Recordings")
            fact:               _videoSettings.enableStorageLimit
            visible:            fact.visible
        }

        LabelledFactTextField {
            Layout.fillWidth:   true
            label:              qsTr("Max Storage Usage")
            fact:               _videoSettings.maxVideoSize
            visible:            fact.visible
            enabled:            _videoSettings.enableStorageLimit.rawValue
        }
    }
}
