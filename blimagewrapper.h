#ifndef BLIMAGEWRAPPER_H
#define BLIMAGEWRAPPER_H

#include <blend2d.h>
#include <QImage>
#include <memory>

// BLImageWrapper : shares pixel data between a BLImage and a QImage

class BLImageWrapper
{
public:
	BLImageWrapper(int w, int h);
	QImage *getQImage() const;
	BLImage *getBlImage() const;

private:
	std::unique_ptr<QImage> qImage;
	std::unique_ptr<BLImage> blImage;
};

#endif // BLIMAGEWRAPPER_H
