/***********************************************************************
 *
 * Copyright (C) 2009 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "clock.h"

#include <QPainter>
#include <QTime>
#include <QTimer>

//-----------------------------------------------------------------------------

Clock::Clock(QWidget* parent)
: QWidget(parent), m_time(0) {
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_timer = new QTimer(this);
	m_timer->setInterval(1000);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));

	QFont f = font();
	f.setBold(true);
	setFont(f);
}

//-----------------------------------------------------------------------------

QSize Clock::sizeHint() const {
	return QSize(186, fontMetrics().height() + 6);
}

//-----------------------------------------------------------------------------

void Clock::addTime(int time) {
	m_time += qMax(time, 0);
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::setPaused(bool paused) {
	if (paused) {
		m_timer->stop();
	} else {
		m_timer->start();
	}
}

//-----------------------------------------------------------------------------

void Clock::setText(const QString& text) {
	m_text = text;
	update();
}

//-----------------------------------------------------------------------------

void Clock::start() {
	m_time = 31;
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::stop() {
	m_time = 0;
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QColor back, front, text;
	if (m_time > 20) {
		back = "#37a42b";
		front = Qt::white;
		text = Qt::black;
	} else if (m_time > 10) {
		back = "#f3c300";
		front = Qt::white;
		text = Qt::black;
	} else if (m_time > 0) {
		back = text = "#bf0000";
		front = "#ffbfbf";
	} else {
		back = "#373737";
		front = Qt::transparent;
		text = Qt::white;
	}

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setPen(Qt::NoPen);

	painter.setBrush(back);
	painter.drawRoundedRect(rect(), 5, 5);

	int secs = qMin(180, m_time);
	int width = 180 - secs;
	painter.setBrush(front);
	painter.drawRoundedRect(secs + 3, 3, width, rect().height() - 6, 2.5, 2.5);

	painter.setPen(text);
	painter.drawText(rect(), Qt::AlignCenter, m_text);
}

//-----------------------------------------------------------------------------

void Clock::updateTime() {
	m_time = qMax(m_time - 1, 0);
	//: Format time for the clock
	m_text = QTime(0,0,0).addSecs(m_time).toString(tr("m:ss"));
	update();

	if (!isFinished()) {
		m_timer->start();
	} else {
		m_timer->stop();
		emit finished();
	}
}

//-----------------------------------------------------------------------------
