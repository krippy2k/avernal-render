# avernal-render

High-level rendering library for the Avernal game engine.

## Overview

avernal-render provides higher-level rendering abstractions built on top of the avernal-rhi (Rendering Hardware Interface). It simplifies common rendering tasks by providing convenient APIs for meshes, materials, textures, cameras, and rendering.

## Core Concepts

### Renderer
The main rendering coordinator that manages the rendering pipeline.

```cpp
auto renderer = avernal::Renderer(device);
renderer.begin_frame(swapchain, commands);
renderer.clear(0.0f, 0.0f, 0.0f, 1.0f);  // Clear to black
renderer.render(mesh, material, camera);
renderer.end_frame();
```

### Mesh
Represents 3D geometry with vertices and indices.

```cpp
std::vector<avernal::Vertex> vertices = { /* ... */ };
std::vector<std::uint16_t> indices = { /* ... */ };
auto mesh = avernal::Mesh::create(device, vertices, indices);
```

The `Vertex` struct includes:
- `position[3]` - XYZ position
- `normal[3]` - XYZ normal vector
- `texcoord[2]` - UV texture coordinates
- `color[4]` - RGBA color

### Material
Defines the visual properties and pipeline for rendering.

```cpp
auto material = avernal::Material::create(device, {
    .color_format = swapchain.format(),
    .use_3d = true,
    .use_depth = true,
    .use_texture = true,
});
material->set_texture(texture.get());
```

### Texture
Wraps texture resources for easy loading and binding.

```cpp
auto texture = avernal::Texture::create(
    device,
    width, height,
    avernal::Format::rgba8_unorm,
    pixel_data
);
```

### Camera
Manages view and projection matrices for 3D rendering.

```cpp
avernal::Camera camera;
camera.set_perspective(
    3.14159f / 4.0f,  // 45-degree FOV
    aspect_ratio,
    0.1f,              // near plane
    100.0f             // far plane
);
camera.set_look_at(
    0.0f, 2.0f, 5.0f,  // eye position
    0.0f, 0.0f, 0.0f,  // look at
    0.0f, 1.0f, 0.0f   // up vector
);
```

## Dependencies

- **avernal-core** - Core utilities and types
- **avernal-rhi** - Rendering Hardware Interface

## Building

avernal-render is built automatically when included as a FetchContent dependency:

```cmake
FetchContent_Declare(
    avernal_render
    SOURCE_DIR "${AVERNAL_RENDER_SOURCE_DIR}"
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(avernal_render)

target_link_libraries(your_target PRIVATE avernal::render)
```

## License

Part of the Avernal game engine project.
