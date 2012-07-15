#include "eventfilterizer.h"

EventFilterizer::EventFilterizer(QObject *parent) :
	QObject(parent)
{
}

bool EventFilterizer::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Wheel)
	{
		QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
		emit scrollIntercept(wheelEvent->delta(), wheelEvent->pos());
		return true;
	}
	else if(event->type() == QEvent::MouseMove)
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
		emit mouseMoveIntercept(mouseEvent->pos());
		return true;
	}
	else if(event->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
		if(mouseEvent->button() == Qt::MiddleButton)
		{
			emit mouseMiddleClickIntercept(mouseEvent->pos());
			return true;
		}
//		  else if(mouseEvent->button() == Qt::LeftButton)
//		  {
//			  emit mouseLeftClickIntercept();
//			  return true;
//		  }
	}
	else if(event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
		if(mouseEvent->button() == Qt::MiddleButton)
		{
			emit mouseMiddleReleaseIntercept();
			return true;
		}
//		  else if(mouseEvent->button() == Qt::LeftButton)
//		  {
//			  emit mouseLeftReleaseIntercept();
//			  return true;
//		  }
	}
	else if(event->type() == QEvent::Resize)
	{
//		  QResizeEvent* resizeEvent = static_cast<QResizeEvent *>(event);
		emit resizeIntercept();
		// allow the event to be passed on
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}
