#version 400

layout ( location = 0) in vec4 vPosition;
layout ( location = 1) in vec4 vColors;

out vec4 v2fcolor;

uniform vec4 vColor;

uniform float scaling;
uniform vec4 translation;

void main ()
{
    gl_Position = (vPosition * vec4(scaling, scaling, scaling, 1)) + translation;
//    gl_Position = vPosition;
    v2fcolor = vColor;
}
