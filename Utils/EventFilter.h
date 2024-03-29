#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>
#include <QDebug>
#include <QEvent>

class ProgramOutputDialog;

class EventFilter : public QObject
{
    Q_OBJECT
public:
    explicit EventFilter(QWidget *parent = 0);
signals:
protected:
    bool eventFilter(QObject *target, QEvent *event);

    ProgramOutputDialog* dialog = nullptr;
};

#endif // EVENTFILTER_H
