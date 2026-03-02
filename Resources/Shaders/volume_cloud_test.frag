#version 450 core

#define STRATUS_GRADIENT       vec4(0.0, 0.1, 0.2, 0.3)      // 层云
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)  // 层积云
#define CUMULUS_GRADIENT       vec4(0.0, 0.1625, 0.7, 0.98)  // 积云

layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 depth;
struct Light
{
    vec4 position;
    vec4 color;
    vec4 direction;
    vec4 info;
};

layout(set = 0, binding = 0) uniform UniformBufferVolumeCloud
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 center;
    vec4 size;
    vec4 parameter;
    float cameranear;
    float camerafar;
} ubo;

layout(set = 0, binding = 1) uniform UniformBufferObjectView
{
    Light directional_lights[4];
    Light point_lights[4];
    Light spot_lights[4];
    ivec4 lights_count;
    vec4 camera_position;
} view;

layout(set = 0, binding = 2) uniform sampler3D volumeTexture;
layout(set = 0, binding = 3) uniform sampler2D weathermap;
layout(set = 0, binding = 4) uniform sampler2D bluenoise;

// 统一参数
float densityFactor = ubo.parameter[0];
float absorption = ubo.parameter[1];
float cloudLumen = ubo.parameter[2];
float time = ubo.parameter[3];

// 新增参数
const float DENSITY_THRESHOLD = 0.3;
const float CLOUD_EDGE_SOFTNESS = 0.1;
const float DETAIL_STRENGTH = 0.5;
const float HEIGHT_GRADIENT_STRENGTH = 1.0;
const float CLOUD_BOTTOM_FADE = 0.2;
const float CLOUD_TOP_FADE = 0.3;

const float POWDER_STRENGTH = 0.5;
const float AMBIENT_STRENGTH = 0.2;
const float FORWARD_SCATTERING = 0.2;
const float BACK_SCATTERING = -0.2;
const float LIGHT_INTENSITY = 4.5;
const float NOISE_SPEED = 1.0;
vec3 NOISE_SCALE = vec3(1.0, 1.0, 1.0);
vec3 NOISE_OFFSET = vec3(0.0, 0.0, 0.0);

// 光照颜色
vec4 CLOUD_COLOR = vec4(1.0, 1.0, 1.0, 1.0);
vec4 AMBIENT_COLOR = vec4(0.5, 0.6, 0.7, 1.0);
vec4 LIGHT_COLOR = vec4(1.0, 1.0, 1.0, 1.0);

#define BAYER_FACTOR 1.0/16.0
float bayerFilter[16u] = float[]
(
	0.0*BAYER_FACTOR, 8.0*BAYER_FACTOR, 2.0*BAYER_FACTOR, 10.0*BAYER_FACTOR,
	12.0*BAYER_FACTOR, 4.0*BAYER_FACTOR, 14.0*BAYER_FACTOR, 6.0*BAYER_FACTOR,
	3.0*BAYER_FACTOR, 11.0*BAYER_FACTOR, 1.0*BAYER_FACTOR, 9.0*BAYER_FACTOR,
	15.0*BAYER_FACTOR, 7.0*BAYER_FACTOR, 13.0*BAYER_FACTOR, 5.0*BAYER_FACTOR
);

vec3 noiseKernel[6] = vec3[] 
(
	vec3( 0.38051305,  0.92453449, -0.02111345),
	vec3(-0.50625799, -0.03590792, -0.86163418),
	vec3(-0.32509218, -0.94557439,  0.01428793),
	vec3( 0.09026238, -0.27376545,  0.95755165),
	vec3( 0.28128598,  0.42443639, -0.86065785),
	vec3(-0.16852403,  0.14748697,  0.97460106)
);

float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
    return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

vec3 getBoxMin() {
    return ubo.center.xyz - ubo.size.xyz * 0.5;
}

vec3 getBoxMax() {
    return ubo.center.xyz + ubo.size.xyz * 0.5;
}

vec3 worldToTexCoord(vec3 worldPos) {
    vec3 boxMin = getBoxMin();
    vec3 boxMax = getBoxMax();
    return (worldPos - boxMin) / (boxMax - boxMin);
}

vec3 objToTexCoord(vec3 objPos) {
    return objPos + 0.5;
}

float hgPhase(float g, float cosTheta)
{
    float numer = 1.0 - g * g;
    float denom = 1.0 + g * g + 2.0 * g * cosTheta;
    return numer / (4.0 * 3.141592653589793 * denom * sqrt(denom));
}

float getDensityForCloud(float heightFraction, float cloudType)
{
	float stratusFactor = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
	float stratoCumulusFactor = 1.0 - abs(cloudType - 0.5) * 2.0;
	float cumulusFactor = clamp(cloudType - 0.5, 0.0, 1.0) * 2.0;

	vec4 baseGradient = stratusFactor * STRATUS_GRADIENT + stratoCumulusFactor * STRATOCUMULUS_GRADIENT + cumulusFactor * CUMULUS_GRADIENT;
    float bottom = clamp(remap(heightFraction, baseGradient.x, baseGradient.y, 0.0, 1.0), 0.0, 1.0);
    float top = clamp(remap(heightFraction, baseGradient.z, baseGradient.w, 1.0, 0.0), 0.0, 1.0);
    float temp = top * bottom;
    return clamp(temp, 0.0, 1.0);
}

float dualLobPhase(float g0, float g1, float w, float cosTheta)
{
    return mix(hgPhase(g0, cosTheta), hgPhase(g1, cosTheta), w);
}

// 新的密度采样函数，类似第二个着色器
float sampleCloudDensity(vec3 currentPos)
{
    vec3 boxMin = getBoxMin();
    vec3 boxMax = getBoxMax();

    vec3 uvw = worldToTexCoord(currentPos);

    // 积云形状
    float heightGradient = 1.0;
    
    float base_weathermapsize=1500.0;
    vec2 uv=(currentPos.xz - boxMin.xz);
    uv=mod(uv, vec2(base_weathermapsize)) / base_weathermapsize;

    vec4 weather=texture(weathermap,uv);
    float heightPercent = uvw.y;
    float gMin = remap(weather.x, 0, 1, 0.1, 0.5);
    //heightGradient = remap(heightPercent, 0.0, weather.r, 1, 0) * remap(heightPercent, 0.0, gMin, 0, 1);

    heightGradient = getDensityForCloud(uvw.y,weather.g);
    heightGradient=clamp(heightGradient,0.0,1.0) * weather.g;
    //heightGradient =  1.0;

    if(heightGradient==0.0)
    {
        return 0.0;
    }

    float base_texturesize=128.0;

    vec3 localPos = currentPos - boxMin;
    vec3 repeatedPos = mod(localPos + vec3(time * 5.0), vec3(base_texturesize)) / base_texturesize;

    float detail_texturesize=16.0;
    vec3 detailPos = mod(localPos + vec3(time * 2.0) * NOISE_SPEED, vec3(detail_texturesize)) / detail_texturesize;

    // 采样噪声
    vec4 noise = texture(volumeTexture, fract(repeatedPos));
    vec4 detail_noise = texture(volumeTexture, fract(detailPos));

    float lowFreqFBM = max(dot(noise.gba, vec3(0.625, 0.25, 0.125)),0.0);
    float highFreqFBM = max(dot(detail_noise.gba, vec3(0.625, 0.25, 0.125)),0.0);

    float density = noise.r;
    
    density *= heightGradient;

    lowFreqFBM = max(0.0,lowFreqFBM);
    density = remap(density, clamp(lowFreqFBM * 1.5 * heightPercent, 0.0, 1.0), 1.0, 0.0, 1.0);
    //density -= (1.0 - lowFreqFBM) * (heightGradient) * 0.1 ;
    density=clamp(density,0.0,1.0);

    density = remap(density, highFreqFBM  * 0.7 * (1.0 - density) * heightPercent, 1.0, 0.0, 1.0);
    density=clamp(density,0.0,1.0);

    return density;
}

bool rayBoxIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax, out float tEnter, out float tExit) {
    vec3 invDir = 1.0 / rayDir;
    vec3 t0 = (boxMin - rayOrigin) * invDir;
    vec3 t1 = (boxMax - rayOrigin) * invDir;
    
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    tEnter = max(max(tmin.x, tmin.y), tmin.z);
    tExit = min(min(tmax.x, tmax.y), tmax.z);
    
    return tEnter <= tExit && tExit > 0.0;
}

// 修改光照步进函数，类似第二个着色器
float lightMarch(vec3 startPos, vec3 lightDir, int steps, float dither)
{
    float stepSize = 5.0;
    float opticalDepth = 0.0;
    vec3 boxMin = getBoxMin();
    vec3 boxMax = getBoxMax();
    
    for (int i = 0; i < steps; i++)
    {
        // 使用dither打破分界
        vec3 samplePos = startPos + (lightDir * stepSize * (float(i) + dither ) + noiseKernel[i%steps] * 1.0) / float(steps);
        
        // 转换到UVW空间
        vec3 uvw = worldToTexCoord(samplePos);
        
        // 边界检查
        if (any(lessThan(uvw, vec3(0.0))) || any(greaterThan(uvw, vec3(1.0))))
            break;
        
        float density = sampleCloudDensity(samplePos);
        opticalDepth += density * stepSize * (float(i) + dither) / float(steps);
        
        // 提前退出
        //if (opticalDepth > 5.0)
        //    break;
    }
    
    // Beer-Lambert + Powder效果
    float transmittance = exp(-opticalDepth * absorption);
    float powder = 1.0 - exp(-opticalDepth * 2.0 * POWDER_STRENGTH);
    
    return transmittance * (1.0 + powder * 0.5);
}

void main() {
    vec4 CloudColor = vec4(1.0, 1.0, 1.0, 1.0);
    
    // 计算视线方向
    vec3 viewDir = normalize(fragPosition - view.camera_position.xyz);
    vec3 rayOrigin = view.camera_position.xyz;
    
    // 获取UBO中定义的包围盒
    vec3 boxMin = getBoxMin();
    vec3 boxMax = getBoxMax();
    
    // 计算射线与包围盒的交点
    float tEnter, tExit;
    if (!rayBoxIntersect(rayOrigin, viewDir, boxMin, boxMax, tEnter, tExit)) {
        outColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }
    
    // 确保从正方向进入
    tEnter = max(tEnter, 0.0);
    
    // 计算步进参数
    float totalDistance = tExit - tEnter;
    const int steps = 64; // 增加步数以获得更好的质量
    float stepSize = totalDistance / float(steps);
    float bigstepsize = stepSize;
    float smallstepsize = float(min(ubo.size.x, min(ubo.size.y, ubo.size.z))) / float(steps);
    
    // 初始化累加值
    vec3 accumulatedColor = vec3(0.0);
    float transmittance = 1.0;
    
    // 光线步进起点
    float dither = texture(bluenoise, (gl_FragCoord.xy  + vec2(time) * 10.0 ) / vec2(512.0) ).r * 0.7;

    vec3 currentPos = rayOrigin + viewDir * (tEnter + stepSize *(1.0+dither));
    
    // 获取主光方向
    vec3 lightDir = normalize(view.directional_lights[0].position.xyz - currentPos);

    float sampleincloudCount = 0;
    vec3 nearestCloudpos = vec3(0.0, 0.0, 0.0);

    float ambientLumen = 0.0;

    for (int i = 0; i < steps; i++) {
        // 边界检查
        vec3 uvw = worldToTexCoord(currentPos);
        if (any(lessThan(uvw, vec3(0.0))) || any(greaterThan(uvw, vec3(1.0))) || transmittance < 0.001) {
            
            break;
        }
        
        // 采样密度
        float density = sampleCloudDensity(currentPos) ;

        
        
        if (density > 0.01) {

            //深度
            sampleincloudCount+=1.0;
            nearestCloudpos+=(1.0 - step(1.5, sampleincloudCount)) * currentPos;

            // 计算光照能量
            float lightEnergy = lightMarch(currentPos, lightDir, 3, dither * 0.0);
            
            // 相位函数（双叶散射）
            float cosAngle = dot(viewDir, lightDir);
            float phase = mix(
                hgPhase(FORWARD_SCATTERING, cosAngle),
                hgPhase(BACK_SCATTERING, cosAngle),
                0.5
            );
            
            // 添加基础多重散射
            phase = max(0.15,phase);
            //phase = 1.0;
            
            // 环境光（在薄处和边缘更强）
            float ambientStrength = clamp((1.0 - lightEnergy) * transmittance, 0.0, 1.0);
            
            //vec3 ambient = AMBIENT_COLOR.rgb * ambientStrength * (1.0 + AMBIENT_STRENGTH) * transmittance ;
            vec3 ambient = AMBIENT_COLOR.rgb * ambientStrength * (1.0 + AMBIENT_STRENGTH) * transmittance ;
            
            // 直接光
            vec3 direct = LIGHT_COLOR.rgb * lightEnergy * phase * LIGHT_INTENSITY;
            
            // 合并光照
            vec3 litColor = CLOUD_COLOR.rgb * (ambient * 0.3 + direct * 1.0);
            
            // Beer's Law 吸收
            float densityStep = density * stepSize * densityFactor;
            float absorptionFactor = exp(-densityStep);
            
            // 累积颜色
            vec3 sampleColor = litColor * density * transmittance  * cloudLumen * stepSize;
            accumulatedColor += sampleColor;

            ambientLumen+=transmittance * lightEnergy * phase;
            
            // 更新透射率
            transmittance *= absorptionFactor;

        }
        // 前进
        currentPos += viewDir * stepSize * (1.0);
    }

    
    vec3 color=accumulatedColor + (transmittance) * AMBIENT_COLOR.xyz ;

    // 最终Alpha
    float alpha = 1.0 - pow(transmittance, 1.0);

    vec4 depthPos =  ubo.view * vec4(nearestCloudpos, 1.0);
    
    
    outColor = vec4(color, alpha);
    float depthcount = ((-depthPos.z)-ubo.cameranear)/(ubo.camerafar-ubo.cameranear);
    depth = vec4(depthcount,depthcount,depthcount,1.0);
}