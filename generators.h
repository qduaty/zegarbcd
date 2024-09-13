#ifndef GENERATORS_H
#define GENERATORS_H
#include <QIcon>

QIcon generateIcon(int* digits, QSize dataSize);
QIcon generate24hourIconFromTime(struct tm * tm);
QIcon generate12hourIconFromTime(struct tm * tm);
/// weekday (1-7); monthday (vert + top to right, 1-31); month (bottom right square, 1-12)
QIcon generate3x4IconFromDate(struct tm * tm);
/// weekday (top row: 1-7); monthday (vert + top to right, 1-31); month (bottom right square, 1-12)
QIcon generate4x3IconFromDate(struct tm * tm);
/// weekday (top row: 1-7); monthday (vert + top to right, 1-31); month (bottom right square, 1-12)
QIcon generate4x3IconFromDate2(struct tm * tm);
QIcon generate3x3IconFromDate(struct tm * tm);
QIcon generate5minIconFromTime(struct tm * tm);
QIcon generate5min3x3IconFromTime(struct tm * tm);

#endif // GENERATORS_H
