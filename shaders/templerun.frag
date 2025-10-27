#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D overrideTex;
uniform bool  useOverride;
uniform float tiling;
uniform vec3  uTint;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

void main() {
    vec3 tex = (useOverride)
        ? texture(overrideTex, fs_in.TexCoord * tiling).rgb
        : texture(texture_diffuse1, fs_in.TexCoord * tiling).rgb;

    tex *= uTint;

    // Blinn-Phong
    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uViewPos - fs_in.FragPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 ambient  = 0.2 * tex;
    vec3 diffuse  = diff * tex * uLightColor;
    vec3 specular = 0.4 * spec * uLightColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
