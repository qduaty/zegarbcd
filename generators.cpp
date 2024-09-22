#include "generators.h"

#include "colortoolbutton.h"

#include <QPainter>
#include <QSettings>

QIcon generateIcon(int *digits, QSize dataSize)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\qduaty\\zegarbcd", QSettings::NativeFormat);
    settings.beginGroup("Preferences");
    auto gridColor = ColorToolButton::colorFromInt(settings.value("pixelColorBackground", 0x505050).toInt());
    auto onColor = ColorToolButton::colorFromInt(settings.value("pixelColorOn", 0xffffff).toInt());
    auto offColor = ColorToolButton::colorFromInt(settings.value("pixelColorOff", 0x000000).toInt());
    int iconSize = settings.value("iconSize", 256).toInt();
    QPixmap pix(iconSize, iconSize);
    QPainter paint(&pix);
    paint.fillRect(0, 0, iconSize, iconSize, gridColor);
    for(int x = 0; x < dataSize.width(); x++)
        for(int y = 0; y < dataSize.height(); y++)
        {
            int xsize = iconSize / dataSize.width();
            int ysize = iconSize / dataSize.height();
            int margin = iconSize / 16;
            int xpos = x * xsize + margin / 2;
            int ypos = y * ysize + margin / 2;
            QColor color = (digits[x] >> (dataSize.height() - 1 - y)) & 0x1 ? onColor : offColor;
            paint.fillRect(xpos, ypos, xsize - margin, ysize - margin, color);
        }

    return pix;
}

QIcon generate24hourIconFromTime(tm *tm) {

    int digits[4];
    digits[0] = tm->tm_hour / 10;
    digits[1] = tm->tm_hour % 10;
    digits[2] = tm->tm_min / 10;
    digits[3] = tm->tm_min % 10;
    return generateIcon(digits, {4, 4});
}

QIcon generate12hourIconFromTime(tm *tm) {
    int digits[3];
    digits[0] = (tm->tm_hour % 12);
    if(digits[0] == 0)
        digits[0] = 12;
    digits[1] = tm->tm_min / 10;
    digits[2] = tm->tm_min % 10;
    return generateIcon(digits, {3, 4});
}

QIcon generate3x4IconFromDate(tm *tm) {
    int digits[4];
    digits[0] = (tm->tm_wday);
    digits[1] = tm->tm_mday & 0b111;
    digits[2] = (tm->tm_mon + 1) & 0b11 | (tm->tm_mday >> 1) & 0b100;
    digits[3] = ((tm->tm_mon + 1) >> 2) | (tm->tm_mday >> 2) & 0b100;
    return generateIcon(digits, {4, 3});
}

QIcon generate4x3IconFromDate(tm *tm) {
    int digits[3];
    digits[0] = tm->tm_mday & 0b111 | ((tm->tm_wday & 0b1) << 3);
    digits[1] = (tm->tm_mon + 1) & 0b11 | (tm->tm_mday >> 1) & 0b100 | ((tm->tm_wday & 0b10) << 2);
    digits[2] = ((tm->tm_mon + 1) >> 2) | (tm->tm_mday >> 2) & 0b100 | ((tm->tm_wday & 0b100) << 1);
    return generateIcon(digits, {3, 4});
}

QIcon generate4x3IconFromDate2(tm *tm) {
    int digits[3];
    digits[0] = tm->tm_mon + 1;
    digits[1] = tm->tm_mday & 0b1111;
    int wday = tm->tm_wday;
    if(wday == 0) wday = 7;
    digits[2] = wday | ((tm->tm_mday & 0b10000) >> 1);
    // for(int i=0;i<3;i++)qDebug()<<digits[i];
    return generateIcon(digits, {3, 4});
}

QIcon generate5minIconFromTime(tm *tm) {
    QSettings settings("HKEY_CURRENT_USER\\Software\\qduaty\\zegarbcd", QSettings::NativeFormat);
    settings.beginGroup("Preferences");
    auto gridColor = ColorToolButton::colorFromInt(settings.value("pixelColorBackground", 0x505050).toInt());
    auto onColor = ColorToolButton::colorFromInt(settings.value("pixelColorOn", 0xffffff).toInt());
    auto offColor = ColorToolButton::colorFromInt(settings.value("pixelColorOff", 0x000000).toInt());
    int iconSize = settings.value("iconSize", 64).toInt();
    QPixmap pix(iconSize, iconSize);
    QPainter paint(&pix);
    paint.fillRect(0, 0, iconSize, iconSize, gridColor);

    int digits[4];
    constexpr int sizes[4] = {2, 3, 2, 2};
    digits[0] = (tm->tm_hour / 8);
    digits[1] = (tm->tm_hour % 8);
    digits[2] = tm->tm_min / 15;
    digits[3] = (tm->tm_min % 15) / 5;

    for(int x = 0; x < 4; x++) {
        for(int y = 0; y < sizes[x]; y++)
        {
            int xsize = iconSize / 4;
            int ysize = iconSize / sizes[x];
            int xmargin = iconSize / 16;
            int ymargin = iconSize / 8;
            int xpos = x * xsize + xmargin / 2;
            int ypos = y * ysize + ymargin / sizes[x];
            QColor color = (digits[x] >> (sizes[x] - 1 - y)) & 0x1 ? onColor : offColor;
            paint.fillRect(xpos, ypos, xsize - xmargin, ysize - ymargin * 2 / sizes[x], color);
        }
    }

    return pix;
}

QIcon generate5min3x3IconFromTime(tm *tm) {
    int digits[3];
    auto hour = tm->tm_hour % 8;
    digits[0] = tm->tm_hour / 8 | (hour & 0b1) << 2;
    digits[1] = tm->tm_min / 15 | (hour & 0b10) << 1;
    digits[2] = (tm->tm_min % 15) / 5 | (hour & 0b100);
    return generateIcon(digits, {3, 3});
}
