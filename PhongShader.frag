#version 450
uniform vec3 cameraPosition;
uniform mat4 MVP;//recuperation de la matrice mvp
uniform mat4 MODEL;
uniform float materialShininess;
uniform vec3 materialSpecularColor;
uniform vec3 materialAmbiantColor;
uniform vec3 materialDiffuseColor;
in vec3 fragPosition;
in vec3 fragNormale;
in vec3 fragColor;
out vec4 finalColor;


uniform samplerCube ourTexture0;
uniform sampler2D ourTexture1;
in vec2 TexCoord;



uniform struct Light {vec3 position ;
                      vec3 intensities;
                      float ambientCoefficient;
                      float attenuation ;} 
                      
                      light;

void main() 
{
    vec3 normale = normalize(transpose(inverse(mat3(MODEL))) * fragNormale);
    vec3 LIGHT_DIRECTION = normalize(light.position - fragPosition);

   // ka:0.9
    vec4 ambient = vec4(materialAmbiantColor, 1.0)*0.9 ;


    vec3 DV = normalize(cameraPosition - fragPosition);
    float diffCoefficient = max(dot(normale, LIGHT_DIRECTION), 0.0);
    float VDCoeff = max(dot(normale, DV), 0.0);
    // kd:0.9
     vec4 diffuse = vec4(materialDiffuseColor, 1.0) * diffCoefficient*0.9;

     

    vec3 reflectDir = reflect(-LIGHT_DIRECTION, normale);
    // ks:1
    vec4 specular = vec4(pow(max(dot(reflectDir, DV), 0.0), materialShininess) * materialSpecularColor*1, 1.0);

    

   
    
    finalColor = vec4(fragColor, 1.0)*0.8 + ambient + diffuse + specular;

 
    
}
