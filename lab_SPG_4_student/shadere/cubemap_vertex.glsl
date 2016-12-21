#version 330

layout(location = 0) in vec3 in_position;		
layout(location = 1) in vec2 vertexUV;
uniform mat4 model_matrix, view_matrix, projection_matrix;
uniform vec2 tex;

out vec2 texcoord;

void main(){

    texcoord = vertexUV;
	//texcoord = vec3(tex, 1);
	gl_Position = projection_matrix * view_matrix * model_matrix*vec4(in_position,1); 
}
