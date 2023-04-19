from conan import ConanFile


class ConanPackage(ConanFile):
    name = 'network-monitor'
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    


