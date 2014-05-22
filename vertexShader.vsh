attribute highp vec3 vertexPosition;
uniform highp mat4 MVP;
uniform highp mat4 cieXYZ2RGB;
varying highp vec4 colorPosition;

void main(void)
{
    colorPosition = cieXYZ2RGB * vec4(vertexPosition,1);
    gl_Position = MVP * colorPosition ;
//    gl_PointSize = 2.0;
}
