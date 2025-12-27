from conan import ConanFile
from conan.tools.cmake import cmake_layout


class GrpcSpikeConan(ConanFile):
    """Converted from conanfile.txt

    Keeps the same requirements, generators and package options as the original.
    """

    settings = "os", "arch", "compiler", "build_type"
    requires = ["grpc/1.72.0", "protobuf/5.27.0", "cpputest/4.0"]
    generators = "CMakeDeps", "CMakeToolchain"

    default_options = {
        "grpc/*:shared": False,
        "protobuf/*:shared": False,
        "abseil/*:shared": False,
    }

    def layout(self):
        cmake_layout(self)
