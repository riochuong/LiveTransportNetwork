from conan import ConanFile


class ConanPackage(ConanFile):
    name = 'network-monitor'
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {"boost/*:shared": False}
    
    def layout(self):
        self.folders.build = f"build/{str(self.settings.build_type)}"
        self.folders.generators = "build"
    
    def requirements(self):
        self.requires("boost/1.81.0")
        self.requires("spdlog/1.10.0")
        self.requires("fmt/8.1.1")
        self.requires("gtest/1.13.0")
        self.requires("openssl/3.1.0")

