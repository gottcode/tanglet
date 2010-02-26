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
#include <QPainterPath>
#include <QTime>
#include <QTimer>

//-----------------------------------------------------------------------------

class Clock::Timer {
public:
	Timer();
	virtual ~Timer();

	virtual bool addWord(int score)=0;
	virtual bool addIncorrectWord(int score);
	virtual QColor color();
	virtual bool isFinished();
	virtual void start()=0;
	virtual void stop();
	virtual int type() const=0;
	virtual QString update();
	virtual int width() const;

protected:
	int m_time;
};

Clock::Timer::Timer()
	: m_time(0) {
}

Clock::Timer::~Timer() {
}

bool Clock::Timer::addIncorrectWord(int) {
	return false;
}

QColor Clock::Timer::color() {
	QColor c;
	if (m_time > 20) {
		c = "#37a42b";
	} else if (m_time > 10) {
		c = "#ffaa00";
	} else if (m_time > 0) {
		c = "#bf0000";
	} else {
		c = "#373737";
	}
	return c;
}

bool Clock::Timer::isFinished() {
	return m_time == 0;
}

void Clock::Timer::stop() {
	m_time = 0;
}

QString Clock::Timer::update() {
	m_time = qMax(m_time - 1, 0);
	return QTime(0,0,0).addSecs(m_time).toString(tr("m:ss"));
}

int Clock::Timer::width() const {
	return m_time;
}

//-----------------------------------------------------------------------------

class Clock::AllotmentTimer : public Clock::Timer {
public:
	bool addWord(int score);
	bool addIncorrectWord(int score);
	void start();
	int type() const;
	QString update();
	int width() const;
};

bool Clock::AllotmentTimer::addWord(int) {
	m_time = qMax(m_time - 1, 0);
	return true;
}

bool Clock::AllotmentTimer::addIncorrectWord(int) {
	m_time = qMax(m_time - 1, 0);
	return true;
}

void Clock::AllotmentTimer::start() {
	m_time = 30;
}

int Clock::AllotmentTimer::type() const {
	return Clock::Allotment;
}

QString Clock::AllotmentTimer::update() {
	return tr("%n guesses(s)", "", m_time);
}

int Clock::AllotmentTimer::width() const {
	return m_time * 6;
}

//-----------------------------------------------------------------------------

class Clock::ClassicTimer : public Clock::Timer {
public:
	bool addWord(int score);
	void start();
	int type() const;
};

bool Clock::ClassicTimer::addWord(int) {
	return false;
}

void Clock::ClassicTimer::start() {
	m_time = 181;
}

int Clock::ClassicTimer::type() const {
	return Clock::Classic;
}

//-----------------------------------------------------------------------------

class Clock::RefillTimer : public Clock::Timer {
public:
	bool addWord(int score);
	void start();
	int type() const;
	int width() const;
};

bool Clock::RefillTimer::addWord(int) {
	m_time = 31;
	return true;
}

void Clock::RefillTimer::start() {
	m_time = 31;
}

int Clock::RefillTimer::type() const {
	return Clock::Refill;
}

int Clock::RefillTimer::width() const {
	return m_time * 6;
}

//-----------------------------------------------------------------------------

class Clock::StaminaTimer : public Clock::Timer {
public:
	bool addWord(int score);
	QColor color();
	void start();
	int type() const;
	QString update();
	int width() const;

private:
	int m_freeze;
};

bool Clock::StaminaTimer::addWord(int) {
	m_freeze = 6;
	return true;
}

QColor Clock::StaminaTimer::color() {
	return (m_freeze) ? "#3389ea" : Timer::color();
}

void Clock::StaminaTimer::start() {
	m_time = 46;
	m_freeze = 0;
}

int Clock::StaminaTimer::type() const {
	return Clock::Stamina;
}

QString Clock::StaminaTimer::update() {
	m_freeze = qMax(0, m_freeze - 1);
	if (m_freeze) {
		return tr("+%1").arg(m_freeze);
	} else {
		m_time = qMax(m_time - 1, 0);
		return tr("0:%1").arg(m_time, 2, 10, QLatin1Char('0'));
	}
}

int Clock::StaminaTimer::width() const {
	if (m_freeze) {
		return 180;
	} else {
		return m_time * 4;
	}
}

//-----------------------------------------------------------------------------

class Clock::StrikeoutTimer : public Clock::Timer {
public:
	bool addWord(int score);
	bool addIncorrectWord(int score);
	void start();
	int type() const;
	QString update();
	int width() const;

private:
	int m_strikes;
};

bool Clock::StrikeoutTimer::addWord(int) {
	return false;
}

bool Clock::StrikeoutTimer::addIncorrectWord(int) {
	m_strikes = qMin(m_strikes + 1, 3);
	m_time = (3 - m_strikes) * 10;
	return true;
}

void Clock::StrikeoutTimer::start() {
	m_time = 30;
	m_strikes = 0;
}

int Clock::StrikeoutTimer::type() const {
	return Clock::Strikeout;
}

QString Clock::StrikeoutTimer::update() {
	return tr("%n strike(s)", "", m_strikes);
}

int Clock::StrikeoutTimer::width() const {
	return (3 - m_strikes) * 60;
}

//-----------------------------------------------------------------------------

class Clock::TangletTimer : public Clock::Timer {
public:
	bool addWord(int score);
	void start();
	int type() const;
};

bool Clock::TangletTimer::addWord(int score) {
	m_time += score + 8;
	return true;
}

void Clock::TangletTimer::start() {
	m_time = 31;
}

int Clock::TangletTimer::type() const {
	return Clock::Tanglet;
}

//-----------------------------------------------------------------------------

Clock::Clock(QWidget* parent)
: QWidget(parent), m_timer(0) {
	setTimer(Tanglet);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_update = new QTimer(this);
	m_update->setInterval(1000);
	connect(m_update, SIGNAL(timeout()), this, SLOT(updateTime()));

	QFont f = font();
	f.setBold(true);
	setFont(f);
}

//-----------------------------------------------------------------------------

Clock::~Clock() {
	delete m_timer;
}

//-----------------------------------------------------------------------------

QSize Clock::sizeHint() const {
	return QSize(186, fontMetrics().height() + 6);
}

//-----------------------------------------------------------------------------

bool Clock::isFinished() const {
	return m_timer->isFinished();
}

//-----------------------------------------------------------------------------

void Clock::addWord(int score) {
	if (m_timer->addWord(score)) {
		updateTime();
	}
}

//-----------------------------------------------------------------------------

void Clock::addIncorrectWord(int score) {
	if (m_timer->addIncorrectWord(score)) {
		updateTime();
	}
}

//-----------------------------------------------------------------------------

void Clock::setPaused(bool paused) {
	if (paused) {
		m_update->stop();
	} else {
		m_update->start();
	}
}

//-----------------------------------------------------------------------------

void Clock::setText(const QString& text) {
	m_text = text;
	update();
}

//-----------------------------------------------------------------------------

void Clock::start() {
	m_timer->start();
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::stop() {
	m_timer->stop();
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::setTimer(int timer) {
	if (!m_timer || m_timer->type() != timer) {
		delete m_timer;

		switch (timer) {
		case Tanglet:
			m_timer = new TangletTimer;
			break;
		case Classic:
			m_timer = new ClassicTimer;
			break;
		case Refill:
			m_timer = new RefillTimer;
			break;
		case Stamina:
			m_timer = new StaminaTimer;
			break;
		case Strikeout:
			m_timer = new StrikeoutTimer;
			break;
		case Allotment:
		default:
			m_timer = new AllotmentTimer;
			break;
		}
	}
}

//-----------------------------------------------------------------------------

QString Clock::timerToString(int timer) {
	static QStringList timers = QStringList() <<
		tr("Tanglet") <<
		tr("Classic") <<
		tr("Refill") <<
		tr("Stamina") <<
		tr("Strikeout") <<
		tr("Allotment");
	return timers.at(qBound(0, timer, TotalTimers - 1));
}

//-----------------------------------------------------------------------------

void Clock::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QColor color = m_timer->color();

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setPen(Qt::NoPen);

	painter.setBrush(color);
	painter.drawRoundedRect(rect(), 5, 5);

	int x = m_timer->width();
	int width = 180 - qMin(180, x);
	if (width < 180) {
		painter.setBrush(QColor(255, 255, 255, 160));
		painter.drawRoundedRect(x + 3, 3, width, rect().height() - 6, 2.5, 2.5);
	}

	QPainterPath path;
	path.addText(93 - (fontMetrics().width(m_text) / 2), fontMetrics().ascent() + 3, font(), m_text);
	painter.setBrush(Qt::white);
	painter.setPen(QPen(Qt::black, 3));
	painter.drawPath(path);
	painter.fillPath(path, Qt::white);
}

//-----------------------------------------------------------------------------

void Clock::updateTime() {
	m_text = m_timer->update();
	update();

	if (!isFinished()) {
		m_update->start();
	} else {
		m_update->stop();
		emit finished();
	}
}

//-----------------------------------------------------------------------------
