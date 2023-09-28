load("//tools/install:install.bzl", "install", "install_src_files")
load("//third_party/gpus:common.bzl", "if_gpu")

package(
    default_visibility = ["//visibility:public"],
)

exports_files([
    "CPPLINT.cfg",
    "tox.ini",
])

install(
    name = "install",
    deps = if_gpu([
        "//modules/perception:install",
        "//modules/planning:install",
        "//modules/v2x:install",
        "//third_party/centerpoint_infer_op:install",
        "//third_party/paddleinference:install",
        "//third_party/caddn_infer_op:install",
    ], [
        "//cyber:install",
        "//tools:install",
        "//modules/data:install",
        #"//cyber/examples:install",
        #"//docker/scripts:install",
        #"//docs:install",
        "//modules/audio:install",
        "//modules/bridge:install",
        "//modules/calibration:install",
        "//modules/canbus:install",
        "//modules/canbus_vehicle:install",
        "//modules/common:install",
        "//modules/common_msgs:install",
        "//modules/contrib/cyber_bridge:install",
        "//modules/control:install",
        "//modules/planning:install",
        "//modules/dreamview:install",
        "//modules/drivers:install",
        "//modules/guardian:install",
        "//modules/localization:install",
        "//modules/map:install",
        "//modules/monitor:install",
        "//modules/prediction:install",
        "//modules/routing:install",
        "//modules/storytelling:install",
        "//modules/task_manager:install",
        "//modules/third_party_perception:install",
        "//modules/tools:install",
        "//modules/transform:install",
        "//scripts:install",
        "//third_party/absl:install",
        "//third_party/ad_rss_lib:install",
        "//third_party/boost:install",
        "//third_party/civetweb:install",
        "//third_party/eigen3:install",
        "//third_party/gtest:install",
        "//third_party/ipopt:install",
        "//third_party/libtorch:install",
        "//third_party/fastrtps:install",
        "//third_party/gflags:install",
        "//third_party/glog:install",
        "//third_party/nlohmann_json:install",
        "//third_party/opencv:install",
        "//third_party/osqp:install",
        "//third_party/pcl:install",
        "//third_party/vtk:install",
        "//third_party/proj:install",
        "//third_party/protobuf:install",
        "//third_party/py:install",
        "//third_party/opengl:install",
        "//third_party/openh264:install",
        "//third_party/cpplint:install",
        "//third_party/centerpoint_infer_op:install",
        "//third_party/portaudio:install",
        "//third_party/fftw3:install",
        "//third_party/glew:install",
        "//third_party/adolc:install",
        "//third_party/atlas:install",
        "//third_party/benchmark:install",
        "//third_party/ncurses5:install",
        "//third_party/sqlite3:install",
        "//third_party/tensorrt:install",
        "//third_party/tinyxml2:install",
        "//third_party/uuid:install",
        "//third_party/yaml_cpp:install",
        "//third_party/qt5:install",
        "//third_party/npp:install",
        "//third_party/gpus:install",
        "//third_party/tf2:install",
        "//third_party/localization_msf:install",
        "//third_party/can_card_library:install",
        "//third_party/camera_library:install",
        "//third_party/rtklib:install",
        "//third_party/ffmpeg:install",
        "//third_party/adv_plat:install",
    ]),
)

install_src_files(
    name = "install_src",
    deps = if_gpu([
        "//modules/perception:install_src",
        "//modules/planning:install_src",
        "//modules/v2x:install_src",
        "//third_party/centerpoint_infer_op:install_src",
        "//third_party/paddleinference:install_src",
        "//third_party/caddn_infer_op:install_src",
    ], [
        "//cyber:install_src",
        "//tools:install_src",
        "//modules/common:install_src",
        "//modules/data:install_src",
        "//modules/common_msgs:install_src",
        "//modules/control:install_src",
        "//modules/dreamview:install_src",
        "//modules/map:install_src",
        "//modules/monitor:install_src",
        "//modules/planning:install_src",
        "//modules/routing:install_src",
        "//modules/task_manager:install_src",
        "//modules/transform:install_src",
        "//modules/audio:install_src",
        "//modules/bridge:install_src",
        "//modules/calibration:install_src",
        "//modules/canbus:install_src",
        "//modules/canbus_vehicle:install_src",
        "//modules/contrib/cyber_bridge:install_src",
        "//modules/drivers:install_src",
        "//modules/guardian:install_src",
        "//modules/localization:install_src",
        "//modules/prediction:install_src",
        "//modules/storytelling:install_src",
        "//modules/third_party_perception:install_src",
        "//modules/tools:install_src",
        "//third_party/absl:install_src",
        "//third_party/boost:install_src",
        "//third_party/civetweb:install_src",
        "//third_party/eigen3:install_src",
        "//third_party/gtest:install_src",
        "//third_party/centerpoint_infer_op:install_src",
        "//third_party/ipopt:install_src",
        "//third_party/libtorch:install_src",
        "//third_party/fastrtps:install_src",
        "//third_party/gflags:install_src",
        "//third_party/glog:install_src",
        "//third_party/nlohmann_json:install_src",
        "//third_party/opencv:install_src",
        "//third_party/osqp:install_src",
        "//third_party/pcl:install_src",
        "//third_party/vtk:install_src",
        "//third_party/proj:install_src",
        "//third_party/protobuf:install_src",
        "//third_party/py:install_src",
        "//third_party/opengl:install_src",
        "//third_party/openh264:install_src",
        "//third_party/cpplint:install_src",
        "//third_party/portaudio:install_src",
        "//third_party/fftw3:install_src",
        "//third_party/glew:install_src",
        "//third_party/adolc:install_src",
        "//third_party/atlas:install_src",
        "//third_party/benchmark:install_src",
        "//third_party/ncurses5:install_src",
        "//third_party/sqlite3:install_src",
        "//third_party/tensorrt:install_src",
        "//third_party/tinyxml2:install_src",
        "//third_party/uuid:install_src",
        "//third_party/yaml_cpp:install_src",
        "//third_party/qt5:install_src",
        "//third_party/npp:install_src",
        "//third_party/gpus:install_src",
        "//third_party/tf2:install_src",
        "//third_party/localization_msf:install_src",
        "//third_party/can_card_library:install_src",
        "//third_party/camera_library:install_src",
        "//third_party/rtklib:install_src",
        "//third_party/ffmpeg:install_src",
        "//third_party/adv_plat:install_src",
    ]),
)