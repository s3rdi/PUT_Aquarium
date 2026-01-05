#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 li1;
uniform vec4 li2;

//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 normal; //wektor normalny wierzcholka w przestrzeni modelu
in vec2 texCoord;

out vec4 l1;
out vec4 l2;
out vec4 n;
out vec4 v;
out vec2 iTexCoord;

void main(void) {
    l1 = normalize(V * (li1 - M * vertex)); //znormalizowany wektor do swiatla w przestrzeni oka
    l2 = normalize(V * (li2 - M * vertex)); //znormalizowany wektor do swiatla w przestrzeni oka
    n = normalize(V * M * normal); //znormalizowany wektor normalny w przestrzeni oka
    v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //Wektor do obserwatora w przestrzeni oka

    iTexCoord = texCoord;

    gl_Position=P*V*M*vertex;
}
