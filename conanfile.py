from conans import ConanFile, CMake


class vkaEngineConan(ConanFile):
    name = "vkaEngine"
    version = "0.0.1"
    license = "MIT"
    author = "Jeff Wright jeffw387@gmail.com"
    url = "https://github.com/jeffw387/vkaEngine.git"
    description = "A vulkan rendering framework"
    settings = "os", "compiler", "build_type", "arch", "cppstd"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports_sources = "src/*"
    build_policy = "missing"
    requires = "glfw/3.2.1@sesame/stable", "glm/0.9.9.1@g-truc/stable", "VulkanMemoryAllocator/2.0.0@sesame/stable", "spdlog/1.3.0@bincrafters/stable", "stb/20180214@conan/stable", "jsonformoderncpp/3.5.0@vthiery/stable", "Catch2/2.5.0@catchorg/stable", "tl_expected/0.2@jeffw387/testing", "tl_optional/0.5@jeffw387/testing"
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.hpp", src="src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["vkaEngine"]