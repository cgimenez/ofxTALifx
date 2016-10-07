import qbs

CppApplication {
    consoleApplication: true
    files: [
        "src/ofxTALifxClient.cpp",
        "src/ofxTALifxClient.h",
        "src/protocol.h",
    ]
}
