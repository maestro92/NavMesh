uniform mat4 u_MVPMat;

in vec3 position;
in vec3 normal;
in vec2 UV;
in vec4 color;


smooth out vec3 vf_normal;
smooth out vec2 vf_UV;
smooth out vec4 vf_color;

void main()
{
	gl_Position = u_MVPMat * vec4(position, 1.0);
	vf_normal = normal;
	vf_UV = UV;
	vf_color = color;
}



