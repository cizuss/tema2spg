#version 330
layout(location = 0) out vec3 out_color;

uniform sampler2D texture_cubemap;
in vec2 texcoord;

void main(){

    out_color = texture(texture_cubemap, texcoord).rgb;
	//out_color = vec3(0.4f, 0.5f, 0.6f);
}