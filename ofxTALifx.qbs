import qbs
import "../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

CppApplication {
    files: [
        "src/ofxTALifxClient.cpp",
        "src/ofxTALifxClient.h",
        "src/protocol.h",
    ]
}
