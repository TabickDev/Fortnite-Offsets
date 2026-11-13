#include <any>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <unordered_map>

struct Vector3
{
    double x{}, y{}, z{};
};

struct Camera
{
    Vector3 location, rotation;
    double fov{};
};

struct FNRot
{
    double a{}, b{}, c{};
};

namespace cache
{
    constexpr uintptr_t uworld = 0x1000;
    constexpr uintptr_t player_controller = 0x2000;
    constexpr uintptr_t local_player = 0x3000;
}

std::unordered_map<uintptr_t, std::any> memory;

template <typename T>
T read(uintptr_t address)
{
    return std::any_cast<T>(memory.at(address));
}

#include "Camera.h"

void expect_near(double actual, double expected, const char* description)
{
    if (!std::isfinite(actual) || std::abs(actual - expected) > 0.0001)
    {
        std::cerr << description << ": expected " << expected << ", got " << actual << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main()
{
    constexpr uintptr_t location = 0x4000;
    constexpr uintptr_t rotation = 0x5000;
    memory[cache::uworld + 0x130] = location;
    memory[cache::uworld + 0x140] = rotation;
    memory[location] = Vector3{10.0, 20.0, 30.0};
    memory[rotation] = -0.5;
    memory[rotation + 0x20] = std::sqrt(3.0) / 2.0;
    memory[rotation + 0x1D0] = 0.5;
    memory[cache::local_player + 0x4AC] = 75.0f;

    for (double degrees : {60.0, 90.0, 120.0})
    {
        // The controller stores the half-angle in radians, not degrees.
        memory[cache::player_controller + 0x3AC] =
            static_cast<float>(degrees * std::numbers::pi / 360.0);
        const Camera camera = get_view_point_from_player_controller();
        expect_near(camera.fov, degrees, "controller FOV in degrees");
        expect_near(camera.location.x, 10.0, "location x");
        expect_near(camera.location.y, 20.0, "location y");
        expect_near(camera.location.z, 30.0, "location z");
        expect_near(camera.rotation.x, 30.0, "pitch");
        expect_near(camera.rotation.y, 30.0, "yaw");
    }

    const Camera local_camera = get_view_point();
    expect_near(local_camera.fov, 75.0, "local-player FOV remains in degrees");
    std::cout << "Camera regression tests passed\n";
}
