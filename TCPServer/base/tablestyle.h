#ifndef TABLESTYLE_H
#define TABLESTYLE_H
#include <QTabBar>
#include <QProxyStyle>
#include <QPainter>
#include <QStyleOption>  //利用 QProxyStyle设置QtabWidget的样式
#include <QStyle>

class TableStyle : public QProxyStyle
{
public:
    TableStyle();
public:
    QSize sizeFromContents(QStyle::ContentsType type,
    const QStyleOption *option,
    const QSize &size,
    const QWidget *widget) const;

    void drawControl(QStyle::ControlElement element,
    const QStyleOption *option,
    QPainter *painter,
    const QWidget *widget) const;

};

#endif // TABLESTYLE_H
