#version 430 core

in vec3 pos;
in vec3 nor;

out vec4 fColor;

uniform vec4 color_ambient = vec4(0.7,0.7,0.9,1.0);
uniform vec4 color_diffuse = vec4(0.7,0.7,0.9,1.0);
uniform vec4 color_specular = vec4(1.0,1.0,1.0,1.0);
uniform vec4 Color = vec4(0.7,0.7,0.9,1.0);
uniform float shininss = 100.0f;
uniform vec3 light_position = vec3(50.0f,32.0f,560.0f);
uniform vec3 EyeDir=vec3(0.0,0.0,1.0);

uniform samplerCube skybox;
in vec3 pos_eye;
in vec3 n_eye;
in mat4 V;

vec3 baseColor = vec3(0.7,0.7,0.9);

uniform float isNormalmap;
uniform sampler2D normalmap1;
uniform sampler2D normalmap2;
in vec2 vUV1;
in vec2 vUV2;

void main()
{
    vec3 r_normal;
//    if(isNormalmap == 1)
//    {
//        //normal map
//        float temp;
//        vec3 normalmapCol1 = vec3(texture2D(normalmap1,vUV1));
//        temp = normalmapCol1.y; normalmapCol1.y = normalmapCol1.z; normalmapCol1.z = temp;
//        vec3 normalmapCol2 = vec3(texture2D(normalmap2,vUV2));
//        temp = normalmapCol2.y; normalmapCol2.y = normalmapCol2.z; normalmapCol2.z = temp;
//        r_normal = normalize(mix(n_eye,normalmapCol1,normalmapCol2));
//    }
//    else
    r_normal = normalize(n_eye);
    vec3 incident_eye = normalize(pos_eye);
//    vec3 reflectVec = reflect(incident_eye , r_normal);
//    reflectVec = vec3(inverse(V) * vec4(reflectVec,0.0));
//
//    vec3 refractVec = refract(incident_eye , r_normal, 1.34);
//    refractVec = vec3(inverse(V) * vec4(refractVec,0.0));

//    vec3 ReflectColor = vec3(textureCube(skybox,reflectVec));
//    vec3 RefractColor = vec3(textureCube(skybox,refractVec));

    float f = pow(1.0-0.66,2.0)/pow(1.0+0.66,2.0);
    float Ratio=f+(1-f)*pow((1-dot(incident_eye,r_normal)),5.0);	//The Ratio may make it error, ignore it.
    //vec3 outputCol = mix(ReflectColor,RefractColor,0.4);	//outputCol is only ReflectColor and RefractColor.


    //Add phong shader.
    vec3 light_direction = normalize(light_position - pos);
    vec3 normal = normalize(nor);
    vec3 half_vector = normalize(normalize(light_direction)+normalize(EyeDir));
    float diffuse = max(0.0,dot(normal,light_direction));
    float specular = pow(max(0.0,dot(nor,half_vector)),shininss);
    vec4 lColor= min(vec4(0.5)*color_ambient,vec4(1.0))+diffuse*color_diffuse*0.5+specular*color_specular;

    

    //fColor =vec4(outputCol,1.0);
    
    fColor = lColor;
    fColor.a = 0.94;

    //fColor = vec4(1.0);

}
