// Flat Color Shader

#type vertex
#version 330 core

layout (location =0) in vec3 a_Position;
layout (location =1) in vec4 a_Color;

uniform mat4 u_ViewProjection= mat4(1);

out vec4 v_Color;

void main(){
    v_Color = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.f);
}

//------------------------

#type fragment
#version 330 core

in vec4 v_Color;

out vec4 out_color;

void main(){
    out_color =  v_Color;
}
