// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <windows.h>
#include <stdexcept>

namespace DxPlus {

    class HighResolutionTimer {
    public:
        HighResolutionTimer(double frameRate = 60.0)
            : deltaTime(0.0), pausedTime(0), stopped(false) {
            LARGE_INTEGER freq;
            if (!QueryPerformanceFrequency(&freq)) {
                throw std::runtime_error("High-resolution timer not supported.");
            }
            countsPerSecond = freq.QuadPart;
            secondsPerCount = 1.0 / static_cast<double>(countsPerSecond);
            SetFrameRate(frameRate);

            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            currentTime = now.QuadPart;
            baseTime = currentTime;
            lastTime = currentTime;
        }

        void Reset() {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            currentTime = now.QuadPart;
            baseTime = currentTime;
            lastTime = currentTime;
            stopTime = 0;
            pausedTime = 0;
            stopped = false;
        }

        void Start() {
            if (stopped) {
                LARGE_INTEGER resumeTime;
                QueryPerformanceCounter(&resumeTime);
                pausedTime += (resumeTime.QuadPart - stopTime);
                lastTime = resumeTime.QuadPart;
                stopTime = 0;
                stopped = false;
            }
        }

        void Stop() {
            if (!stopped) {
                LARGE_INTEGER now;
                QueryPerformanceCounter(&now);
                stopTime = now.QuadPart;
                stopped = true;
            }
        }

        // Call this once per frame. Returns true if a frame should be processed.
        bool UpdateFrame() {
            if (stopped) {
                deltaTime = 0.0;
                return false;
            }

            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            currentTime = now.QuadPart;

            if (currentTime < lastTime + countsPerFrame) return false;

            deltaTime = (currentTime - lastTime) * secondsPerCount;
            LONGLONG elapsed = currentTime - lastTime;
            lastTime += countsPerFrame;

            if (elapsed > countsPerSecond / 15) {
                lastTime = currentTime;
            }

            if (deltaTime < 0.0) {
                deltaTime = 0.0;
            }

            return true;
        }

        double GetTimeSinceStart() const {
            if (stopped) {
                return ((stopTime - pausedTime) - baseTime) * secondsPerCount;
            }
            return ((currentTime - pausedTime) - baseTime) * secondsPerCount;
        }

        double GetDeltaTime() const {
            return deltaTime;
        }

        void SetFrameRate(double frameRate) {
            if (frameRate <= 0.0) {
                throw std::invalid_argument("Frame rate must be positive.");
            }
            countsPerFrame = static_cast<LONGLONG>(countsPerSecond / frameRate);
        }

        LONGLONG GetLastRawTime() const { return lastTime; }

        // 次のフレームまでの残り時間（秒）を返す
        double GetTimeUntilNextFrame() const
        {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            LONGLONG current = now.QuadPart;

            LONGLONG nextFrameTime = lastTime + countsPerFrame;
            LONGLONG remainingCounts = nextFrameTime - current;

            if (remainingCounts <= 0) return 0.0;

            return static_cast<double>(remainingCounts) * secondsPerCount;
        }

    private:
        double secondsPerCount;
        double deltaTime;

        LONGLONG countsPerSecond;
        LONGLONG countsPerFrame;

        LONGLONG currentTime;
        LONGLONG baseTime;
        LONGLONG pausedTime;
        LONGLONG stopTime;
        LONGLONG lastTime;

        bool stopped;
    };

} // namespace DxPlus
