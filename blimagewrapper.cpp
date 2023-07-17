#include "blimagewrapper.h"

BLImageWrapper::BLImageWrapper(int w, int h)
{
	qImage = std::make_unique<QImage>(w, h, QImage::Format_ARGB32_Premultiplied);
	blImage = std::make_unique<BLImage>();
	blImage->createFromData(w, h, BL_FORMAT_PRGB32, qImage->bits(), qImage->bytesPerLine());
}

QImage *BLImageWrapper::getQImage() const
{
	return qImage.get();
}

BLImage *BLImageWrapper::getBlImage() const
{
	return blImage.get();
}
