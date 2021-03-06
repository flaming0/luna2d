//-----------------------------------------------------------------------------
// luna2d engine
// Copyright 2014-2016 Stepan Prokofjev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "lunaimage.h"
#include "lunaengine.h"
#include "lunafiles.h"
#include "lunalog.h"
#include "lunatexture.h"

using namespace luna2d;

int BytesPerPixel(LUNAColorType colorType)
{
	switch(colorType)
	{
	case LUNAColorType::RGBA:
		return 4;
	case LUNAColorType::RGB:
		return 4;
	case LUNAColorType::ALPHA:
		return 1;
	}
}

// Construct empty image
LUNAImage::LUNAImage() : LUNAImage(0, 0, LUNAColorType::RGBA)
{
}

// Constuct empty image with given sizes and color type
LUNAImage::LUNAImage(int width, int height, LUNAColorType colorType) :
	data(width * height * BytesPerPixel(colorType)),
	width(width),
	height(height),
	colorType(colorType)
{
}

// Constuct image from given data
LUNAImage::LUNAImage(int width, int height, LUNAColorType colorType, const std::vector<unsigned char>& data) :
	data(data),
	width(width),
	height(height),
	colorType(colorType)
{
}

// Constructor with loading
LUNAImage::LUNAImage(const std::string& filename, const LUNAImageFormat& format, LUNAFileLocation location) : LUNAImage()
{
	Load(filename, format, location);
}

// Convert given coordinates to position in data buffer
int LUNAImage::CoordsToPos(int x, int y) const
{
	return (x + y * width) * BytesPerPixel(colorType);
}

bool LUNAImage::IsEmpty() const
{
	return data.empty();
}

const std::vector<unsigned char>& LUNAImage::GetData() const
{
	return data;
}

int LUNAImage::GetWidth() const
{
	return width;
}

int LUNAImage::GetHeight() const
{
	return height;
}

LUNAColorType LUNAImage::GetColorType() const
{
	return colorType;
}

bool LUNAImage::Load(const std::string& filename, const LUNAImageFormat& format, LUNAFileLocation location)
{
	// Read data form file
	std::vector<unsigned char> fileData = LUNAEngine::SharedFiles()->ReadFile(filename, location);
	if(fileData.empty()) return false;

	return format.Decode(fileData, data, width, height, colorType);
}

void LUNAImage::SetPixel(int x, int y, const LUNAColor& color)
{
	if(IsEmpty() || x < 0 || y < 0 || x > width || y > height) return;

	int pos = CoordsToPos(x, y);

	switch(colorType)
	{
	case LUNAColorType::RGBA:
		data[pos] = color.GetR();
		data[pos + 1] = color.GetG();
		data[pos + 2] = color.GetB();
		data[pos + 3] = color.GetA();
		break;
	case LUNAColorType::RGB:
		data[pos] = color.GetR();
		data[pos + 1] = color.GetG();
		data[pos + 2] = color.GetB();
		break;
	case LUNAColorType::ALPHA:
		data[pos] = color.GetA();
		break;
	}
}

LUNAColor LUNAImage::GetPixel(int x, int y) const
{
	if(IsEmpty() || x < 0 || y < 0 || x > width || y > height) return LUNAColor();

	int pos = CoordsToPos(x, y);

	switch(colorType)
	{
	case LUNAColorType::RGBA:
		return LUNAColor::Rgb(data[pos], data[pos + 1], data[pos + 2], data[pos + 3]);
	case LUNAColorType::RGB:
		return LUNAColor::Rgb(data[pos], data[pos + 1], data[pos + 2]);
	case LUNAColorType::ALPHA:
		return LUNAColor::Rgb(255, 255, 255, data[pos]);
	}
}

// Fill image with given color
void LUNAImage::Fill(const LUNAColor& color)
{
	FillRectangle(0, 0, width, height, color);
}

// Draw another image to this image
void LUNAImage::DrawImage(int x, int y, const LUNAImage& image)
{
	if(IsEmpty() || image.IsEmpty()) return;
	if(x < 0 || y < 0 || x + image.GetWidth() > width || y + image.GetHeight() > height) return;

	// Line-by-line copying buffer is faster than SetPixel
	// Use it when possible
	if(image.GetColorType() == colorType)
	{
		DrawBuffer(x, y, image.GetData(), image.GetWidth(), image.GetHeight(), image.GetColorType());
		return;
	}

	for(int j = 0; j < image.GetHeight(); j++)
	{
		for(int i = 0; i < image.GetWidth(); i++)
		{
			SetPixel(x + i, y + j, image.GetPixel(i, j));
		}
	}
}

// Draw image from given buffer to this image
// Width and height of buffer in pixels
// Color type of given buffer should be same as image color type
void LUNAImage::DrawBuffer(int x, int y,
	const std::vector<unsigned char>& buffer, int bufferWidth, int bufferHeight, LUNAColorType bufferColorType)
{
	DrawRawBuffer(x, y, buffer.data(), bufferWidth, bufferHeight, bufferColorType);
}
// Draw image from given raw buffer to this image
// Width and height of buffer in pixels
// Color type of given buffer should be same as image color type
void LUNAImage::DrawRawBuffer(int x, int y,
	const unsigned char* buffer, int bufferWidth, int bufferHeight, LUNAColorType bufferColorType)
{
	if(bufferColorType != colorType || x < 0 || y < 0 ||
		x + bufferWidth > width || y + bufferHeight > height) return;

	int rowLen = bufferWidth * BytesPerPixel(bufferColorType);

	for(int row = 0; row < bufferHeight; row++)
	{
		int pos = CoordsToPos(x, y + row);
		int bufferPos = row * rowLen;

		memcpy(&data[pos], &buffer[bufferPos], rowLen);
	}
}

// Fill rectangle on image with given color
void LUNAImage::FillRectangle(int x, int y, int width, int height, const LUNAColor& color)
{
	for(int j = 0; j < height; j++)
	{
		for(int i = 0; i < width; i++)
		{
			SetPixel(x + i, y + j, color);
		}
	}
}
