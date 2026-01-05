#version 330

uniform sampler2D textureMap; //tekstura koloru podstawowego

out vec4 pixelColor; //kolor wyjsciowy piksela

in vec4 l1;
in vec4 l2;
in vec4 n;
in vec4 v;
in vec2 iTexCoord;

void main(void) {
    vec4 nL1 = normalize(l1);
    vec4 nL2 = normalize(l2);

    vec4 nN = normalize(n);
    vec4 nV = normalize(v);

    vec4 r1 = reflect(-nL1, nN); //wektor odbity dla swiatla 1
    vec4 r2 = reflect(-nL2, nN); //wektor odbity dla swiatla 2

    float ml1 = clamp(dot(nN, nL1), 0.0, 1.0); //iloczyn skalarny n i l1
    float ml2 = clamp(dot(nN, nL2), 0.0, 1.0); //iloczyn skalarny n i l2

    float rv1 = pow(clamp(dot(r1, nV), 0.0, 1.0), 100.0); //sila odbicia dla swiatla 1
    float rv2 = pow(clamp(dot(r2, nV), 0.0, 1.0), 100.0); //sila odbicia dla swiatla 2

    vec4 kd = texture(textureMap, iTexCoord);
	vec4 ks = vec4(0.7f, 0.7f, 0.1f, 0.1f);

    pixelColor = vec4(0.2, 0.2, 0.6, 0.1) + vec4(ml1 * kd.rgb, ks.a) + vec4(ml2 * kd.rgb, ks.a) + vec4(ks.rgb*rv1, 0) + vec4(ks.rgb*rv2, 0);
}