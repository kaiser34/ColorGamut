varying highp vec3 colorPosition;

float gamma_sRGB(float x){
    float ft;
    float t = (x>0.0)?x:-x;

    if (t>0.0031308)
    {
        ft = 1.055*pow(t,1.0/2.4)-0.055;
    }
    else
    {
        ft = 12.92*t;
    }

    return (x>0.0)?ft:-ft;
}

vec4 corCIEXYZtosRGB(vec3 XYZ)
{
    vec3 color;

    color.r = 3.2404542*XYZ.x - 1.5371385*XYZ.y - 0.4985314*XYZ.z;
    color.g =-0.9692660*XYZ.x + 1.8760108*XYZ.y + 0.0415560*XYZ.z;
    color.b = 0.0556434*XYZ.x - 0.2040259*XYZ.y + 1.0572252*XYZ.z;

    color.r = gamma_sRGB(color.r);
    color.g = gamma_sRGB(color.g);
    color.b = gamma_sRGB(color.b);

    return vec4(color,1);
}

void main(void)
{
    gl_FragColor = corCIEXYZtosRGB(colorPosition);
}
