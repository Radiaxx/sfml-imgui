# 2D Scalar Field Renderer

Daniele Scrima's Bachelor Thesis project, an interactive visualization of 2D scalar fields with geospatial overlays using SFML 3 and Dear ImGui.

## Features

- **Shader based Heatmap Rendering** - ASC data to float texture to fragment shader
- **Interactive Navigation** - Mouse wheel zoom at cursor and left-drag panning
- **Dynamic Grid Overlay** - Appears when cells size is >= 30x30 px with optional numeric labels
- **Click to Query** - Tooltip with cell value under cursor
- **Geospatial Data Support** - Overlay points, lines, and areas from GeoCSV files
- **Smart Value Clamping** - Auto or manual min / max adjustment
- **Multiple Colormaps** - Customize visualization appearance

## Project Structure

```
sfml-imgui/
├── data/
│   ├── asc/        # ASC files (auto scanned at startup)
│   └── geo/        # Optional GeoCSV files (auto scanned at startup)
├── shaders/        # GLSL shaders
├── src/            # Application source code
├── dependencies/   # Third party dependencies
└── CMakeLists.txt
```

> **Note**: All paths are predefined in CMake as compile definitions:
> - `ASC_DATA_PATH` -> `data/asc/`
> - `GEO_DATA_PATH` -> `data/geo/`
> - `SHADERS_PATH` -> `shaders/`

## Build Instructions

### Prerequisites
- CMake 3.28+
- C++ 17 compatible compiler
- SFML 3
- Dear ImGui

### Build Steps

```bash
# From the repository root
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Project Components
- **Main target**: `sfml-imgui` (C++17)
- **Utility libraries**: `AscParser`, `GeoCsvParser` (static)
- **Dependencies**: `SFML::Graphics`, `ImGui-SFML::ImGui-SFML`

## Running the Application

```bash
# From the root directory
./release/sfml-imgui
```

## User Guide

### 1. Loading Data

1. Place one or more `.asc` files in the `data/` directory
2. Launch the application (files are auto discovered)
3. Select a dataset from **Control Panel -> Dataset**

### 2. Navigation

| Action | Control |
|--------|---------|
| **Zoom** | Mouse wheel (zooms at cursor) |
| **Pan** | Hold left mouse button + drag |

### 3. Visualization Controls

#### Colormap Selection
- Navigate to **Control Panel -> Colormap**
- Choose from available color schemes

#### Value Clamping
- **Auto Mode**: Enable _"Auto clamp min / max to View"_ to adapt colors to visible cells
- **Manual Mode**: Disable auto clamp to manually edit min / max range values

#### Grid and Labels
- **Grid Display**: Appears automatically when cells size is >= 30 pixels
- **Value Labels**: Toggle _"Show Values"_ to display numeric values in each visible cell when cells size is >= 30 pixels

### 4. Querying Values

Click anywhere on the heatmap to display the value of the cell under the cursor (works at any zoom level).

### 5. Geo Data Overlay (Optional)

1. Place GeoCSV files in `data/geo/`
2. Select a file from **Control Panel -> Geo Data**
3. Configure display options:
   - Toggle entity types (points/lines/areas)
   - Apply filters by life
   - Customize colors
   - Adjust size and thickness
