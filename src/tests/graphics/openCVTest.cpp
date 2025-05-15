#include "al_ext/opencv/al_OpenCV.hpp"
using namespace al;

struct MyApp : public App
{
    AlloOpenCV video;

    void onCreate() override
    {
        // Start camera capture (index 0)
        video.initializeVideoCaptureDevice(0, true);

        // Setup camera projection
        navControl().useMouse(false); // Disable fly camera
    }

    void onAnimate(double dt) override
    {
        if (video.videoCapture && video.videoCapture->isOpened())
        {
            video.captureFrame(); // Just capture the frame
            if (!video.videoImage.empty())
            {
                // Only submit if we got a valid frame
                video.videoTexture.submit(video.videoImage.ptr());
            }
        }
    }

    void onDraw(Graphics &g) override
    {
        g.clear(0.0);

        if (video.videoTexture.created())
        {
            g.camera(Viewpoint::IDENTITY); // 2D projection

            float aspect = video.frameAspect();
            float w = 1.0f;
            float h = w / aspect;

            g.quad(video.videoTexture, -w, h, 2 * w, -2 * h); // Show full frame
        }
    }
};

int main()
{
    MyApp app;
    app.configureAudio(0, 0, 0, 0); // Disable audio
    app.start();
}
