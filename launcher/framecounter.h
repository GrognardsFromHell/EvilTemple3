
#if !defined (__FRAMECOUNTER_H__)
#define __FRAMECOUNTER_H__

#include <QElapsedTimer>

class FrameCounter {
public:
    FrameCounter() : mFrames(0), mFps(0) {
        mLastUpdateTimer.start();
    }

    bool frameDrawn() {
        qint64 elapsed = mLastUpdateTimer.elapsed();

        mFrames++;

        if (elapsed >= 1000) {
            elapsed = mLastUpdateTimer.restart();
            mFps = (mFrames * 1000) / elapsed;
            mFrames = 0;
            return true;
        }

        return false;
    }

    int fps() const {
        return mFps;
    }

    void reset() {
        mFrames = 0;
        mLastUpdateTimer.restart();
    }

private:
    int mFps;
    int mFrames;
    QElapsedTimer mLastUpdateTimer;
};

#endif // __FRAMECOUNTER_H__
