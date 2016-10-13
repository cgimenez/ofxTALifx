import qbs
import "../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

CppApplication {
    files: [
        "src/defines.h",
        "src/ofxTALifxBulb.cpp",
        "src/ofxTALifxBulb.h",
        "src/ofxTALifxClient.cpp",
        "src/ofxTALifxClient.h",
        "src/ofxTALifxUdpManager.cpp",
        "src/ofxTALifxUdpManager.h",
        "src/protocol.h",
    ]

}

