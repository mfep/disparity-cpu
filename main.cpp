#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include "lodepng.h"


constexpr int WINDOW = 9;
constexpr int MAX_D = 260 / 4;


std::vector<float> preprocessPixels(const std::vector<unsigned char>& pixels, unsigned width, unsigned height)
{
    const float
            R = 0.2126f,
            G = 0.7152f,
            B = 0.0722f;

    unsigned index = 0;
    std::vector<float> resized (pixels.size() / 16 / 3);
    for (unsigned row = 0; row < height; row += 4) {
        for (unsigned col = 0; col < width; col += 4) {
            const unsigned char
                    r = pixels[(row * width + col) * 3],
                    g = pixels[(row * width + col) * 3 + 1],
                    b = pixels[(row * width + col) * 3 + 2];
            resized[index++] = r * R + g * G + b * B;
        }
    }
    return resized;
}


std::vector<float> loadGreyPixels(const char *filename, unsigned& width, unsigned& height)
{
    std::vector<unsigned char> pixels;
    unsigned error = lodepng::decode(pixels, width, height, filename, LCT_RGB);
    if (error) {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    } else {
		std::cout << "loading '" << filename << "' was successful" << std::endl;
	}
    return preprocessPixels(pixels, width, height);
}


template<typename T>
void savePixels(const std::vector<T>& pixels, unsigned width, unsigned height)
{
    std::vector<unsigned char> convertedPixels(pixels.size());
    unsigned i = 0;
    for (auto f : pixels) {
        convertedPixels[i++] = static_cast<unsigned char>(f);
    }
    unsigned error = lodepng::encode("im0_resized.png", convertedPixels, width, height, LCT_GREY, 8);
    if (error) {
        std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
}


class PixelWrapper {
public:
    PixelWrapper(std::vector<float>&& data, unsigned width, unsigned height) noexcept :
            m_data      (data),
            m_width     (width),
            m_height    (height)
    {
    }

    unsigned getWidth   () const noexcept { return m_width; }
    unsigned getHeight  () const noexcept { return m_height; }

    float get(int row, int col) const noexcept
    {
        if (row < 0) {
            row = 0;
        } else if ((unsigned)row >= m_height) {
            row = m_height - 1;
        }
        if (col < 0) {
            col = 0;
        } else if ((unsigned)col >= m_width) {
            col = m_width - 1;
        }
        return m_data[m_width * row + col];
    }

private:
    const std::vector<float> m_data;
    const unsigned m_width;
    const unsigned m_height;
};


float windowMean(const PixelWrapper& pixels, int cx, int cy)
{
    const int D = WINDOW / 2;
    float sum = 0.0f;
    for (int row = cy - D; row <= cy + D; ++row) {
        for (int col = cx - D; col <= cx + D; ++col) {
            sum += pixels.get(row, col);
        }
    }
    return sum / static_cast<float>(WINDOW * WINDOW);
}


float windowVar(const PixelWrapper &pixels, int cx, int cy, int d)
{
    const int D = WINDOW / 2;
    const float mean = windowMean(pixels, cx, cy);
    float sum = 0.0f;
    for (int row = cy - D; row <= cy + D; ++row) {
        for (int col = cx - D; col <= cx + D; ++col) {
            const float val = pixels.get(row - d, col) - mean;
            sum += val * val;
        }
    }
    return sqrtf(sum);
}


float calcZncc(const PixelWrapper& pixL, const PixelWrapper& pixR, int cx, int cy, int d)
{
    const float meanL = windowMean(pixL, cx, cy);
    const float meanR = windowMean(pixR, cx, cy);

    const int D = WINDOW / 2;
    float sum = 0.0f;
    for (int row = cy - D; row <= cy + D; ++row) {
        for (int col = cx - D; col <= cx + D; ++col) {
            sum += (pixL.get(row, col) - meanL) * (pixR.get(row, col - d) - meanR);
        }
    }
    return sum / windowVar(pixL, cx, cy, d) / windowVar(pixR, cx, cy, d);
}


int findBestDisparity(const PixelWrapper& pixL, const PixelWrapper& pixR, int cx, int cy)
{
    float best_zncc = 0.0f;
    int best_disp = 0;
    for (unsigned disp = 0; disp < MAX_D; ++disp) {
        const float zncc = calcZncc(pixL, pixR, cx, cy, disp);
        if (zncc > best_zncc) {
            best_zncc = zncc;
            best_disp = disp;
        }
    }
    return best_disp;
}


int main()
{
	getchar();
	std::cout << "start" << std::endl;
    unsigned width, height;
    auto grey1 = loadGreyPixels("im0.png", width, height);
    auto grey2 = loadGreyPixels("im1.png", width, height);

    width /= 4;
    height /= 4;

    PixelWrapper greyPx1(std::move(grey1), width, height);
    PixelWrapper greyPx2(std::move(grey2), width, height);

    {
        std::vector<int> filtered(width * height);
        unsigned index = 0;
        for (unsigned row = 0; row < height; ++row) {
            for (unsigned col = 0; col < width; ++col) {
                filtered[index++] = findBestDisparity(greyPx1, greyPx2, col, row);
            }
            static int lastProgress = -1;
            const auto currentProgress = static_cast<int>(static_cast<float>(row) / height * 100.f);
            if (lastProgress != currentProgress) {
                std::cout << "progress: " << currentProgress << " %" << std::endl;
                lastProgress = currentProgress;
            }
        }
        savePixels(filtered, width, height);
    }

    return 0;
}