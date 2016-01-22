#pragma once

#include <QImage>

namespace cv {
    class Mat;
}

QImage Mat2QImage(cv::Mat const&);
