attribute highp vec3 vertexPosition;
uniform highp mat4 MVP;
varying highp vec3 colorPosition;

void main(void)
{
    colorPosition = vertexPosition;
    gl_Position = MVP * vec4(vertexPosition,1);
}
