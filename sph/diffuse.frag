#version 330

uniform vec3 u_lightColor;
uniform vec3 u_camPosition;
uniform sampler2D u_shadowMap;

in vec3 fs_position;
in vec3 fs_normal;
in vec3 fs_color;
in vec3 fs_light_vector;
in vec4 fs_shadowUV;

out vec4 outColor;

void main(void)
{
    //base colors for materials
    vec4 diffuseColor = vec4(fs_color, 1.0);
	vec4 lightColor = vec4(u_lightColor, 1.0);
    
	vec3 view = normalize( u_camPosition - fs_position );
	vec3 light = normalize(fs_light_vector);
	vec3 normal = normalize(fs_normal);

    float ambientTerm = 0.1;
    float diffuseTerm = clamp( 0.9 * dot(normal, light), 0.0, 1.0);
	float specularTerm = clamp( 0.1 * pow( max( 0.0, dot( reflect(-light, normal), view)), 50.0), 0.0, 1.0);

	float visibility = 1.0;
	if ( texture( u_shadowMap, fs_shadowUV.xy ).z  <  ( fs_shadowUV.z / fs_shadowUV.w ) ){
		visibility = 0.5;
	}

    outColor = lightColor * diffuseColor * ( ambientTerm + diffuseTerm ) + lightColor * specularTerm;
	outColor *= visibility;
}