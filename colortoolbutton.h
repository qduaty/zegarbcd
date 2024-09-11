#ifndef COLORTOOLBUTTON_H
#define COLORTOOLBUTTON_H

#include <QToolButton>

class ColorToolButton : public QToolButton
{
    Q_OBJECT
public:
    ColorToolButton(QWidget *parent = nullptr);
    void setValue(int value);
    int value() const;
    static QColor colorFromInt(int value);
signals:
    void colorChanged(QColor newValue);
private:
    void onClick();
    QColor color;
};

#endif // COLORTOOLBUTTON_H
