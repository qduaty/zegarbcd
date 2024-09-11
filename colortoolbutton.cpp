#include "colortoolbutton.h"
#include <QColorDialog>
#include <qpainter.h>

QIcon generateIcon(const QColor& color) {
    QPixmap pix(16, 16);
    QPainter paint(&pix);
    paint.fillRect(0, 0, 16, 16, color);
    return pix;
}

ColorToolButton::ColorToolButton(QWidget *parent): QToolButton(parent) {
    connect(this, &QAbstractButton::clicked, this, &ColorToolButton::onClick);
}

void ColorToolButton::setValue(int value)
{
    setIcon(generateIcon(color = colorFromInt(value)));
}

int ColorToolButton::value() const
{
    int result = (color.red() << 16) | (color.green() << 8) | color.blue();
    return result;
}

QColor ColorToolButton::colorFromInt(int value)
{
    QColor color;
    color.setRed((value >> 16) & 0xff);
    color.setGreen((value >> 8) & 0xff);
    color.setBlue(value & 0xff);
    return color;
}

void ColorToolButton::onClick()
{
    color = QColorDialog::getColor(color, this);
    setIcon(generateIcon(color));
    emit colorChanged(color);
}
