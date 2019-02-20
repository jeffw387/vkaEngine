from conans import ConanFile, CMake

class vkaEngineConan(ConanFile):
    name = "vkaEngine"
    versionfile = open(".version")
    version = versionfile.read()
    versionfile.close()
    license = "MIT"
    author = "Jeff Wright jeffw387@gmail.com"
    url = "https://github.com/jeffw387/vkaEngine.git"
    description = "A vulkan rendering framework"
    settings = "os", "compiler", "build_type", "arch", "cppstd"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports = "CMakeLists.txt", ".version", "!build"
    exports_sources = "!build", "*.hpp", "*.cpp"
    build_policy = "missing"
    requires = (
        "vulkan-sdk/1.X.X@jeffw387/testing",
        "libcpp/latest@jeffw387/testing",
        "libstdcpp/latest@jeffw387/testing",
        "pthread/latest@jeffw387/testing",
        "cppfs/experimental@jeffw387/testing",
        "glfw/3.2.1@jeffw387/testing", 
        "glm/0.9.9.1@g-truc/stable", 
        "VulkanMemoryAllocator/2.2.0@jeffw387/testing", 
        "spdlog/1.3.0@bincrafters/stable", 
        "stb/20180214@conan/stable", 
        "jsonformoderncpp/3.5.0@vthiery/stable", 
        "Catch2/2.5.0@catchorg/stable", 
        "tl_expected/0.2@jeffw387/testing", 
        "tl_optional/0.5@jeffw387/testing",
        "json-shader/latest@jeffw387/testing")
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        
        self.copy("*.hpp", dst="src", src="src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["vkaEngine"]
        self.cpp_info.includedirs = ["src"]
