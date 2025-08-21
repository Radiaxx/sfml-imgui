#version 130

uniform sampler2D u_scalarTexture; // The grayscale data texture
uniform int u_colormapID;

out vec4 fragColor;

vec4 colormap_BlueToRed(float v) {
    return vec4(v, 0.0, 1.0 - v, 1.0);
}

vec4 colormap_Grayscale(float v) {
    return vec4(v, v, v, 1.0);
}

void main()
{
    // Get the normalized scalar value (0.0 to 1.0) from the red channel (any channel could be used) of our texture
    // gl_TexCoord[0].xy contains the texture coordinates for the current pixel (we are in a fragment shader)
    float normalized_value = texture(u_scalarTexture, gl_TexCoord[0].xy).r;

    if (u_colormapID == 0) {
        fragColor = colormap_BlueToRed(normalized_value);
    } else if (u_colormapID == 1) {
        fragColor = colormap_Grayscale(normalized_value);
    } else {
        // Default to green for any unknown id to make errors more noticeable
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
}