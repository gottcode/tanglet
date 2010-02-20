/***********************************************************************
 *
 * Copyright (C) 2009, 2010 Graeme Gott <graeme@gottcode.org>
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
: QWidget(parent), m_time(0), m_type(0) {
	setMode(TangletMode);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_timer = new QTimer(this);
	m_timer->setInterval(1000);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));

	QFont f = font();
	f.setBold(true);
	setFont(f);
}

//-----------------------------------------------------------------------------

Clock::~Clock() {
	delete m_type;
}

//-----------------------------------------------------------------------------

QSize Clock::sizeHint() const {
	return QSize(186, fontMetrics().height() + 6);
}

//-----------------------------------------------------------------------------

void Clock::addWord(int score) {
	if (m_type->addWord(score)) {
		updateTime();
	}
}

//-----------------------------------------------------------------------------

void Clock::setMode(int mode) {
	if (!m_type || m_type->type() != mode) {
		delete m_type;

		switch (mode) {
		case TangletMode:
			m_type = new TangletType(m_time);
			break;
		case BoggleMode:
			m_type = new BoggleType(m_time);
			break;
		case RefillMode:
		default:
			m_type = new RefillType(m_time);
			break;
		}
	}
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
	m_type->start();
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::stop() {
	m_time = 0;
	updateTime();
}

//-----------------------------------------------------------------------------

QString Clock::modeString(int mode) {
	static QStringList timers = QStringList() <<
		tr("Tanglet") <<
		tr("Boggle") <<
		tr("Refill");
	return timers.at(qBound(0, mode, timers.count() - 1));
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

Clock::Type::Type(int& time)
	: m_time(time) {
}

Clock::Type::~Type() {
}

//-----------------------------------------------------------------------------

Clock::BoggleType::BoggleType(int& time)
	: Type(time) {
}

bool Clock::BoggleType::addWord(int score) {
	Q_UNUSED(score);
	return false;
}

void Clock::BoggleType::start() {
	m_time = 181;
}

int Clock::BoggleType::type() const {
	return Clock::BoggleMode;
}

//-----------------------------------------------------------------------------

Clock::RefillType::RefillType(int& time)
	: Type(time) {
}

bool Clock::RefillType::addWord(int score) {
	Q_UNUSED(score);
	m_time = 31;
	return true;
}

void Clock::RefillType::start() {
	m_time = 31;
}

int Clock::RefillType::type() const {
	return Clock::RefillMode;
}

//-----------------------------------------------------------------------------

Clock::TangletType::TangletType(int& time)
	: Type(time) {
}

bool Clock::TangletType::addWord(int score) {
	m_time += score + 8;
	return true;
}

void Clock::TangletType::start() {
	m_time = 31;
}

int Clock::TangletType::type() const {
	return Clock::TangletMode;
}

//-----------------------------------------------------------------------------
