{
  "targets": [
    { "target_name": "" }
  ],
  "conditions": [
    ['OS=="mac"', {
      "targets": [{
        "target_name": "<(module_name)",
        "sources": [ "fsevents.cc" ],
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "xcode_settings": {
          "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
          "CLANG_CXX_LIBRARY": "libc++",
          "MACOSX_DEPLOYMENT_TARGET": "10.7",
          "OTHER_LDFLAGS": [
            "-framework CoreFoundation -framework CoreServices"
          ]
        },
        "include_dirs": [
          "<!@(node -p \"require('node-addon-api').include\")"
        ],
        "dependencies": [
          "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        "msvs_settings": {
          "VCCLCompilerTool": { "ExceptionHandling": 1 },
        },
      }, {
        "target_name": "action_after_build",
        "type": "none",
        "dependencies": ["<(module_name)"],
        "copies": [{
          "files": ["<(PRODUCT_DIR)/<(module_name).node"],
          "destination": "<(module_path)"
        }]
      }]
    }]
  ]
}
