#version 450
uniform mat4 MVP;//recuperation de la matrice mvp
uniform mat4 MODEL;
layout(location = 0) in vec3 position; // le location permet de dire de quel flux/canal on récupère les données (doit être en accord avec le location du code opengl)
layout (location =3) in vec3 normale;//recuperation des normale
layout (location =2) in vec2 texture;//recuperation des textures
layout (location =1) in vec3 color;//recuperation des textures

out vec3 fragPosition;
out vec3 fragNormale;
out vec2 TexCoord;
out vec3 fragColor;
void main(){
    
    gl_Position= MVP* vec4(position,1.0);
    fragPosition =vec3(MODEL* vec4(position,1.0));
	fragNormale =normale;
    fragColor = color;
    TexCoord=texture;
}