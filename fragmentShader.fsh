varying highp vec4 colorPosition;

void main(void)
{
    gl_FragColor = vec4(colorPosition.xyz/colorPosition.w,1);
}
