#include "EventFilter.h"
#include "Utils/ProgramOutputDialog.h"

#include <QApplication>


EventFilter::EventFilter(QWidget *parent):
    QObject(parent)
{
    dialog = ProgramOutputDialog::getInstance();
}

bool EventFilter::eventFilter(QObject *target, QEvent *event)
{
    //     if (event->type() == QEvent::KeyPress)
    //    {
    QPoint p = QCursor::pos();
    QWidget *w = QApplication::widgetAt(p);
    if(w)
        dialog->appendText(w->objectName());
    //    }

    return true;
}
