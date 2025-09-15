import matplotlib.colors as mcolors
import numpy as np
from typing import Final

_gist_earth_data = {
    'red': (
        (0.0, 0.0, 0.0000),
        (0.2824, 0.1882, 0.1882),
        (0.4588, 0.2714, 0.2714),
        (0.5490, 0.4719, 0.4719),
        (0.6980, 0.7176, 0.7176),
        (0.7882, 0.7553, 0.7553),
        (1.0000, 0.9922, 0.9922),
    ),
    'green': (
        (0.0, 0.0, 0.0000),
        (0.0275, 0.0000, 0.0000),
        (0.1098, 0.1893, 0.1893),
        (0.1647, 0.3035, 0.3035),
        (0.2078, 0.3841, 0.3841),
        (0.2824, 0.5020, 0.5020),
        (0.5216, 0.6397, 0.6397),
        (0.6980, 0.7171, 0.7171),
        (0.7882, 0.6392, 0.6392),
        (0.7922, 0.6413, 0.6413),
        (0.8000, 0.6447, 0.6447),
        (0.8078, 0.6481, 0.6481),
        (0.8157, 0.6549, 0.6549),
        (0.8667, 0.6991, 0.6991),
        (0.8745, 0.7103, 0.7103),
        (0.8824, 0.7216, 0.7216),
        (0.8902, 0.7323, 0.7323),
        (0.8980, 0.7430, 0.7430),
        (0.9412, 0.8275, 0.8275),
        (0.9569, 0.8635, 0.8635),
        (0.9647, 0.8816, 0.8816),
        (0.9961, 0.9733, 0.9733),
        (1.0000, 0.9843, 0.9843),
    ),
    'blue': (
        (0.0, 0.0, 0.0000),
        (0.0039, 0.1684, 0.1684),
        (0.0078, 0.2212, 0.2212),
        (0.0275, 0.4329, 0.4329),
        (0.0314, 0.4549, 0.4549),
        (0.2824, 0.5004, 0.5004),
        (0.4667, 0.2748, 0.2748),
        (0.5451, 0.3205, 0.3205),
        (0.7843, 0.3961, 0.3961),
        (0.8941, 0.6651, 0.6651),
        (1.0000, 0.9843, 0.9843),
    )
}

_terrain_data = (
        (0.00, (0.2, 0.2, 0.6)),
        (0.15, (0.0, 0.6, 1.0)),
        (0.25, (0.0, 0.8, 0.4)),
        (0.50, (1.0, 1.0, 0.6)),
        (0.75, (0.5, 0.36, 0.33)),
        (1.00, (1.0, 1.0, 1.0))
)


def convertAndPrintColormap(name, data, num_samples=256):
    """
    Converts a Matplotlib colormap definition into a GLSL vec3 array.
    Handles both dictionary and list of tuples formats.
    """
    print(f"const vec3 {name.upper()}_COLORS[{num_samples}] = vec3[](")

    # Create the colormap object from the data
    if isinstance(data, dict):
        # For the dictionary format (e.g. gist_earth)
        cmap = mcolors.LinearSegmentedColormap(name, data)
    else:
        # For the list of tuples format (e.g. terrain)
        cmap = mcolors.LinearSegmentedColormap.from_list(name, data)

    # Sample the colormap at evenly spaced points
    sample_points = np.linspace(0, 1, num_samples)
    colors = cmap(sample_points)

    glsl_lines = []

    for i, color in enumerate(colors):
        # color is an (r, g, b, a) tuple but we only need RGB.
        r, g, b, a = color
        line = f"    vec3({r:.6f}, {g:.6f}, {b:.6f})"

        if i < num_samples - 1:
            line += ","

        glsl_lines.append(line)

    # Formatted output
    print("\n".join(glsl_lines))
    print(");")
    print("\n" * 2)

NUM_SAMPLES: Final = 256
convertAndPrintColormap('gist_earth', _gist_earth_data, NUM_SAMPLES)
convertAndPrintColormap('terrain', _terrain_data, NUM_SAMPLES)