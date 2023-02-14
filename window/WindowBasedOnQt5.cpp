#define _WINDOW_BASED_ON_QT5_CPP_
#include "WindowBasedOnQt5.h"
#include <QApplication>
#include <QMainWindow>
//#include <QtWidgets/QWidget>
#include <QTimer>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QImage>
#include <QRgb>

namespace window_based_on_qt5
{

#define _MOUSE_BUTTON(e) (Qt::LeftButton == event->button() ? 0 : (Qt::MidButton == event->button() ? 1 : 2))

	class Qt5Window : public QMainWindow
	{
		class MemoryImage
		{
			QSize m_Used;
			QSize m_Real;
			QImage* m_Image;

			MemoryImage(const MemoryImage&) = delete;
			MemoryImage& operator = (const MemoryImage&) = delete;

		public:
			MemoryImage(int w, int h);
			~MemoryImage();
			void Build(int w, int h, const int* c);
			QImage* Image();
		};

		class RenderImplement : public IRender
		{
			QPainter* m_Painter;

			MemoryImage m_ImageBuildFromMemory;

		public:

			RenderImplement();

			void SetPainter(QPainter* painter);

			virtual void SetPenColor(int rgb) override;
			virtual void SetBrushColor(int rgb) override;
			virtual void DrawString(int x, int y, const char* str) override;
			virtual void DrawLine(int x1, int y1, int x2, int y2) override;
			virtual void DrawRectangle(int x1, int y1, int x2, int y2) override;
			virtual void DrawEllipse(int x1, int y1, int x2, int y2) override;
			virtual void DrawARGB(int dx, int dy, const int* argb, int w, int h) override;
			virtual void DrawARGB(int dx, int dy, int dw, int dh, const int* argb, int w, int h) override;
		};

		//用户应用相关
		IUserApp* m_UserApp;
		QString m_Title;
		int m_PixelWidth;
		int m_PixelHeight;
		int m_loopIntervalMilliseconds;
		bool m_activeInMinimized;

		//定时器
		int m_TimerID;

		//渲染器
		RenderImplement m_RenderImplement;

		//事件响应
		void changeEvent(QEvent* event);
		void timerEvent(QTimerEvent* event);
		void paintEvent(QPaintEvent* event);
		void keyPressEvent(QKeyEvent* event);
		void keyReleaseEvent(QKeyEvent* event);
		void mousePressEvent(QMouseEvent* event);
		void mouseReleaseEvent(QMouseEvent* event);
		void mouseDoubleClickEvent(QMouseEvent* event);
		void mouseMoveEvent(QMouseEvent* event);

	public:

		Qt5Window(QWidget* parent);

		~Qt5Window();

		void SetUserApp(IUserApp* user_app);
	};

	Qt5Window::MemoryImage::MemoryImage(int w, int h)
		: m_Used(w, h)
		, m_Real(w, h)
		, m_Image(new QImage(w, h, QImage::Format_ARGB32))
	{
	}

	Qt5Window::MemoryImage::~MemoryImage()
	{
		delete m_Image;
	}

	void Qt5Window::MemoryImage::Build(int w, int h, const int* c)
	{
		m_Used.setWidth(w);
		m_Used.setHeight(h);

		if (m_Real.width() < m_Used.width() ||
			m_Real.height() < m_Used.height())
		{
			m_Real.setWidth(m_Used.width());
			m_Real.setHeight(m_Used.height());

			delete m_Image;

			m_Image = new QImage(
				m_Real.width(),
				m_Real.height(),
				QImage::Format_ARGB32);
		}

		int uw = m_Used.width();
		int uh = m_Used.height();
		for (int y = 0; y < uh; ++y)
		{
			QRgb* rgb = (QRgb*)m_Image->scanLine(y);
			for (int x = 0; x < uw; ++x)
				rgb[x] = c[x + y * uw];
		}
	}

	QImage* Qt5Window::MemoryImage::Image()
	{
		return m_Image;
	}

	Qt5Window::RenderImplement::RenderImplement()
		: m_Painter(nullptr)
		, m_ImageBuildFromMemory(1, 1)
	{}

	void Qt5Window::RenderImplement::SetPainter(QPainter* painter)
	{
		m_Painter = painter;
	}

	void Qt5Window::RenderImplement::SetPenColor(int rgb)
	{
		if (_RENDER_NIL == rgb)
			m_Painter->setPen(Qt::NoPen);
		else
			m_Painter->setPen(QColor(QRgb(rgb)));
	}

	void Qt5Window::RenderImplement::SetBrushColor(int rgb)
	{
		if (_RENDER_NIL == rgb)
			m_Painter->setBrush(Qt::NoBrush);
		else
			m_Painter->setBrush(QBrush(QColor(QRgb(rgb))));
	}

	void Qt5Window::RenderImplement::DrawString(int x, int y, const char* str)
	{
		//文字底部略有突出
		m_Painter->drawText(x, y, QString::fromUtf8(str));
	}

	void Qt5Window::RenderImplement::DrawLine(int x1, int y1, int x2, int y2)
	{
		m_Painter->drawLine(x1, y1, x2, y2);
	}

	void Qt5Window::RenderImplement::DrawRectangle(int x1, int y1, int x2, int y2)
	{
		m_Painter->drawRect(x1, y1, x2, y2);
	}

	void Qt5Window::RenderImplement::DrawEllipse(int x1, int y1, int x2, int y2)
	{
		m_Painter->drawEllipse(x1, y1, x2, y2);
	}

	void Qt5Window::RenderImplement::DrawARGB(int dx, int dy, const int* argb, int w, int h)
	{
		m_ImageBuildFromMemory.Build(w, h, argb);

		m_Painter->drawImage(
			QPoint(dx, dy),
			*m_ImageBuildFromMemory.Image(),
			QRect(0, 0, w, h));
	}

	void Qt5Window::RenderImplement::DrawARGB(int dx, int dy, int dw, int dh, const int* argb, int w, int h)
	{
		m_ImageBuildFromMemory.Build(w, h, argb);

		m_Painter->drawImage(
			QRect(dx, dy, dw, dh),
			*m_ImageBuildFromMemory.Image(),
			QRect(0, 0, w, h));
	}

	Qt5Window::Qt5Window(QWidget* parent)
		: QMainWindow(parent)
		, m_UserApp(Q_NULLPTR)
		, m_Title("")
		, m_PixelWidth(0)
		, m_PixelHeight(0)
		, m_loopIntervalMilliseconds(0)
		, m_activeInMinimized(false)
		, m_TimerID(0)
		, m_RenderImplement()
	{
	}

	Qt5Window::~Qt5Window()
	{
		m_UserApp->OnEnd();
	}

	void Qt5Window::SetUserApp(IUserApp* user_app)
	{
		//设置本对象名字
		if (objectName().isEmpty())
			setObjectName(QString::fromUtf8("MainWindow"));

		//添加中心组件
		QWidget* central_widget = new QWidget(this);
		central_widget->setObjectName(QString::fromUtf8("central_widget"));
		setCentralWidget(central_widget);

		//信号槽关联
		QMetaObject::connectSlotsByName(this);

		//得到用户应用
		m_UserApp = user_app;
		std::string title = m_UserApp->getTitle();
		m_Title = QString::fromLocal8Bit(title.c_str());
		m_PixelWidth = m_UserApp->getPixelWidth();
		m_PixelHeight = m_UserApp->getPixelHeight();
		m_loopIntervalMilliseconds = m_UserApp->getLoopIntervalMilliseconds();
		m_activeInMinimized = m_UserApp->getActiveInMinimized();

		//设置窗口标题栏
		setWindowTitle(m_Title);

		//设置窗口尺寸
		setFixedSize(m_PixelWidth, m_PixelHeight);

		//启动定时器
		m_TimerID = startTimer(m_loopIntervalMilliseconds);

		//用户应用初始化
		m_UserApp->OnInit();
	}

	void Qt5Window::changeEvent(QEvent* event)
	{
		if (QEvent::WindowStateChange == event->type())
		{
			QWindowStateChangeEvent* state_change_event =
				dynamic_cast<QWindowStateChangeEvent*>(event);

			if (Q_NULLPTR != state_change_event)
			{
				if (!m_activeInMinimized)
				{
					//之前窗口状态
					//state_change_event->oldState()

					//目前窗口状态
					//windowState()

					//窗口状态判断
					// if (Qt::WindowNoState == window_state) 0x00000000
					//     ;
					// else
					// {
					//     
					//     if (Qt::WindowMinimized & window_state) 0x00000001
					//         ;
					//     if (Qt::WindowMaximized & window_state) 0x00000002
					//         ;
					//     if (Qt::WindowFullScreen & window_state) 0x00000004
					//         ;
					//     if (Qt::WindowActive & window_state) 0x00000008
					//         ;
					// }

					//目前窗口状态
					switch (windowState())
					{
					case Qt::WindowNoState:
					{
						m_TimerID = startTimer(m_loopIntervalMilliseconds);
						break;
					}
					case Qt::WindowMinimized:
					{
						killTimer(m_TimerID);
						break;
					}
					}
				}
			}
		}
	}

	void Qt5Window::timerEvent(QTimerEvent* event)
	{
		if (m_TimerID != event->timerId())
			return;

		//更新逻辑，返回为真则更新渲染
		if (m_UserApp->OnUpdateLogic())
			update();

		QMainWindow::timerEvent(event);
	}

	void Qt5Window::paintEvent(QPaintEvent* event)
	{
		QPainter painter(this);

		//用户更新渲染
		m_RenderImplement.SetPainter(&painter);
		m_UserApp->OnUpdateRender(&m_RenderImplement);

		QMainWindow::paintEvent(event);
	}

	void Qt5Window::keyPressEvent(QKeyEvent* event)
	{
		int param = event->key();
		m_UserApp->OnInput(_INPUT_KEY_PRESS, &param);

		QMainWindow::keyPressEvent(event);
	}

	void Qt5Window::keyReleaseEvent(QKeyEvent* event)
	{
		int param = event->key();
		m_UserApp->OnInput(_INPUT_KEY_RELEASE, &param);

		QMainWindow::keyReleaseEvent(event);
	}

	void Qt5Window::mousePressEvent(QMouseEvent* event)
	{
		int param[] = { _MOUSE_BUTTON(event), event->x(), event->y() };
		m_UserApp->OnInput(_INPUT_MOUSE_PRESS, param);

		QMainWindow::mousePressEvent(event);
	}

	void Qt5Window::mouseReleaseEvent(QMouseEvent* event)
	{
		int param[] = { _MOUSE_BUTTON(event), event->x(), event->y() };
		m_UserApp->OnInput(_INPUT_MOUSE_RELEASE, param);

		QMainWindow::mouseReleaseEvent(event);
	}

	void Qt5Window::mouseDoubleClickEvent(QMouseEvent* event)
	{
		int param[] = { _MOUSE_BUTTON(event), event->x(), event->y() };
		m_UserApp->OnInput(_INPUT_MOUSE_DOUBLE_CLICK, param);

		QMainWindow::mouseDoubleClickEvent(event);
	}

	void Qt5Window::mouseMoveEvent(QMouseEvent* event)
	{
		int param[] = { _MOUSE_BUTTON(event), event->x(), event->y() };
		m_UserApp->OnInput(_INPUT_MOUSE_MOVE, param);

		QMainWindow::mouseMoveEvent(event);
	}

	int Launch(int argc, char* argv[], IUserApp* user_app)
	{
		QApplication application(argc, argv);
		Qt5Window qt5_window(nullptr);
		qt5_window.SetUserApp(user_app);
		qt5_window.show();
		return application.exec();
	}

#undef _MOUSE_BUTTON

}

