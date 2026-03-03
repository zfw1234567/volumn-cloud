基于Vulkan的体积云渲染器

本项目是为了学习Vulkan而启动的学习项目，实现了基于光线步进的体积云效果。

项目代码基于知乎用户徐兮子美的代码对Vulkan进行了简单的封装，参考地平线：零之曙光的思路实现了体积云效果。

噪声：使用计算管线，在渲染管线开始前预计算Perlin-Worley噪声和三个频率逐渐增加的低频Worley噪声作为128*128*128的3D纹理，实现程序化云体建模。地平线：零之曙光中同时使用了第二张更小的3D纹理用于渲染高频噪声，这里选择复用第一张3D纹理，更高频次地采样3个Worly噪声，在减少带宽的情况下达到了同样的效果。

光照模型：基于RayMarching，使用Beer-Lambert定律计算透光率，使用锥形采样模拟多重散射，使用Powder模拟糖粉效果。

性能优化：利用weather-map减少对3D纹理的采样次数，对空旷天空更友好，1×1分辨率下耗时从12ms降至8ms。实现降分辨率和上采样，1/2×1/2分辨率下单帧耗时降至3ms。

其他优化：基于蓝噪声的dither打破云体分层和阴影分界，使用高斯模糊和TAA实现降噪。

效果展示：
<img width="1075" height="718" alt="image" src="https://github.com/user-attachments/assets/8fa5bf60-1597-45c6-9fd7-9bf23d7726c3" />
<img width="1080" height="717" alt="屏幕截图 2026-03-03 180212" src="https://github.com/user-attachments/assets/502f3c2f-2467-4a32-95e7-c192b8f839b8" />
1/2×1/2分辨率：
<img width="1069" height="705" alt="image" src="https://github.com/user-attachments/assets/2673ca06-276d-469f-8d1c-57671e443917" />

