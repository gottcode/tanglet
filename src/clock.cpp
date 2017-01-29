/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2011, 2014 Graeme Gott <graeme@gottcode.org>
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
#include <QSettings>
#include <QTime>
#include <QTimer>

#include <algorithm>

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

	void load(const QSettings& game);
	void save(QSettings& game);

protected:
	int m_time;

private:
	virtual void loadDetails(const QSettings& game);
	virtual void saveDetails(QSettings& game);
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
	m_time = std::max(m_time - 1, 0);
	return QTime(0,0,0).addSecs(m_time).toString(tr("m:ss"));
}

int Clock::Timer::width() const {
	return m_time;
}

void Clock::Timer::load(const QSettings& game) {
	m_time = std::max(0, game.value("TimerDetails/Time", m_time).toInt()) + 1;
	loadDetails(game);
}

void Clock::Timer::save(QSettings& game) {
	if (!isFinished()) {
		game.setValue("TimerDetails/Time", m_time);
		saveDetails(game);
	} else {
		stop();
	}
}

void Clock::Timer::loadDetails(const QSettings&) {
}

void Clock::Timer::saveDetails(QSettings&) {
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

private:
	void loadDetails(const QSettings&);
};

bool Clock::AllotmentTimer::addWord(int) {
	m_time = std::max(m_time - 1, 0);
	return true;
}

bool Clock::AllotmentTimer::addIncorrectWord(int) {
	m_time = std::max(m_time - 1, 0);
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

void Clock::AllotmentTimer::loadDetails(const QSettings&) {
	m_time--;
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

class Clock::DisciplineTimer : public Clock::Timer {
public:
	bool addWord(int score);
	bool addIncorrectWord(int score);
	void start();
	int type() const;
};

bool Clock::DisciplineTimer::addWord(int score) {
	m_time += score + 8;
	return true;
}

bool Clock::DisciplineTimer::addIncorrectWord(int score) {
	m_time = std::max(0, m_time - (score + 8));
	return true;
}

void Clock::DisciplineTimer::start() {
	m_time = 31;
}

int Clock::DisciplineTimer::type() const {
	return Clock::Discipline;
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
	void loadDetails(const QSettings& game);
	void saveDetails(QSettings& game);

private:
	int m_freeze;
};

bool Clock::StaminaTimer::addWord(int) {
	m_freeze = 6;
	return true;
}

QColor Clock::StaminaTimer::color() {
	return (m_time && m_freeze) ? "#3389ea" : Timer::color();
}

void Clock::StaminaTimer::start() {
	m_time = 46;
	m_freeze = 0;
}

int Clock::StaminaTimer::type() const {
	return Clock::Stamina;
}

QString Clock::StaminaTimer::update() {
	m_freeze = std::max(0, m_freeze - 1);
	return (m_freeze) ? tr("+%1").arg(m_freeze) : Timer::update();
}

int Clock::StaminaTimer::width() const {
	if (m_freeze) {
		return 180;
	} else {
		return m_time * 4;
	}
}

void Clock::StaminaTimer::loadDetails(const QSettings& game) {
	m_freeze = qBound(0, game.value("TimerDetails/Freeze").toInt(), 5) + 1;
}

void Clock::StaminaTimer::saveDetails(QSettings& game) {
	game.setValue("TimerDetails/Freeze", m_freeze);
}

//-----------------------------------------------------------------------------

class Clock::StrikeoutTimer : public Clock::Timer {
public:
	bool addWord(int score);
	bool addIncorrectWord(int score);
	void start();
	void stop();
	int type() const;
	QString update();
	int width() const;

private:
	void loadDetails(const QSettings& game);
	void saveDetails(QSettings& game);

private:
	int m_strikes;
};

bool Clock::StrikeoutTimer::addWord(int) {
	return false;
}

bool Clock::StrikeoutTimer::addIncorrectWord(int) {
	m_strikes = std::min(m_strikes + 1, 3);
	m_time = (3 - m_strikes) * 10;
	return true;
}

void Clock::StrikeoutTimer::start() {
	m_time = 30;
	m_strikes = 0;
}

void Clock::StrikeoutTimer::stop() {
	m_time = 0;
	m_strikes = 3;
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

void Clock::StrikeoutTimer::loadDetails(const QSettings& game) {
	m_strikes = qBound(0, game.value("TimerDetails/Strikes").toInt(), 3);
	m_time = (3 - m_strikes) * 10;
}

void Clock::StrikeoutTimer::saveDetails(QSettings& game) {
	game.setValue("TimerDetails/Strikes", m_strikes);
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

// Lets the user keep adding words until game is manually ended
// TODO: Should end automatically if user has found all the possible words
class Clock::UntimedTimer : public Clock::Timer {
public:
	bool addWord(int score);
	QColor color();
	bool isFinished();
	void start();
	void stop();
	int type() const;
	QString update();
	int width() const;
};

bool Clock::UntimedTimer::addWord(int) {
	return false;
}

QColor Clock::UntimedTimer::color() {
	return "#3389ea";
}

bool Clock::UntimedTimer::isFinished() {
	return !m_time;
}

void Clock::UntimedTimer::start() {
	m_time = true;
}

void Clock::UntimedTimer::stop() {
	m_time = false;
}

int Clock::UntimedTimer::type() const {
	return Clock::Untimed;
}

QString Clock::UntimedTimer::update() {
	return tr("Untimed");
}

int Clock::UntimedTimer::width() const {
	return 180; // full width
}

//-----------------------------------------------------------------------------

Clock::Clock(QWidget* parent)
: QWidget(parent), m_timer(0) {
	setTimer(Tanglet);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_update = new QTimer(this);
	m_update->setInterval(1000);
	connect(m_update, &QTimer::timeout, this, &Clock::updateTime);

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
	return QSize(186, fontMetrics().height() + 8);
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

void Clock::load(const QSettings& game) {
	m_timer->load(game);
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::save(QSettings& game) {
	m_timer->save(game);
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
			m_timer = new AllotmentTimer;
			break;
		case Untimed:
			m_timer = new UntimedTimer;
			break;
		case Discipline:
		default:
			m_timer = new DisciplineTimer;
			break;
		}
	}
}

//-----------------------------------------------------------------------------

int Clock::timer() const {
	return m_timer->type();
}

//-----------------------------------------------------------------------------

QString Clock::timerToString(int timer) {
	static QStringList timers = QStringList() <<
		tr("Tanglet") <<
		tr("Classic") <<
		tr("Refill") <<
		tr("Stamina") <<
		tr("Strikeout") <<
		tr("Allotment") <<
		tr("Discipline") <<
		tr("Untimed");
	return timers.at(qBound(0, timer, TotalTimers - 1));
}

//-----------------------------------------------------------------------------

QString Clock::timerDescription(int timer) {
	static QStringList timers = QStringList() <<
		tr("Counts down from 30 seconds and increases on correct guesses.") <<
		tr("Counts down from 3 minutes.") <<
		tr("Counts down from 30 seconds and refills on correct guesses.") <<
		tr("Counts down from 45 seconds and pauses on correct guesses.") <<
		tr("Game ends after 3 incorrect guesses.") <<
		tr("Game ends after 30 guesses.") <<
		tr("Counts down from 30 seconds and increases or decreases on guesses.") <<
		tr("Unlimited time (press Game > End to finish).");
	return timers.at(qBound(0, timer, TotalTimers - 1));
}

//-----------------------------------------------------------------------------

void Clock::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QColor color = m_timer->color();

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setPen(Qt::NoPen);

	// Draw indent
	QLinearGradient indent_gradient(0, 0, 0, height());
	QColor shadow = palette().color(QPalette::Shadow);
	indent_gradient.setColorAt(0, QColor(shadow.red(),shadow.green(),shadow.blue(),64));
	indent_gradient.setColorAt(1, QColor(shadow.red(),shadow.green(),shadow.blue(),0));
	painter.setBrush(indent_gradient);
	painter.drawRoundedRect(rect(), 4, 4);

	// Draw outside border
	painter.setBrush(color.darker());
	painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 3, 3);

	// Draw filled background
	QLinearGradient gradient(0, 2, 0, height() - 2);
	gradient.setColorAt(0, color.lighter(115));
	gradient.setColorAt(0.49, color.darker(125));
	gradient.setColorAt(0.5, color.darker(150));
	gradient.setColorAt(1, color.lighter(105));
	painter.setBrush(gradient);
	painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 3, 3);

	int x = m_timer->width();
	int width = 180 - std::min(180, x);
	if ((width < 180) && (width > 0)) {
		// Draw empty space
		painter.setBrush(QColor(255,255,255,160));
		painter.drawRoundedRect(x + 3, 2, width + 1, rect().height() - 4, 2, 2);

		// Draw dividing line
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.setPen(color.darker(150));
		painter.drawLine(x + 3, 2, x + 3, rect().height() - 2);
		painter.setRenderHint(QPainter::Antialiasing, true);
	}

	// Draw text shadow
	QPainterPath path;
	path.addText(93 - (fontMetrics().width(m_text) / 2), fontMetrics().ascent() + 3, font(), m_text);
	painter.setBrush(Qt::black);
	painter.translate(0.6, 0.6);
	painter.setPen(QPen(QColor(0,0,0,32), 3));
	painter.drawPath(path);
	painter.translate(-0.2, -0.2);
	painter.setPen(QPen(QColor(0,0,0,64), 2));
	painter.drawPath(path);
	painter.translate(-0.2, -0.2);
	painter.setPen(QPen(QColor(0,0,0,192), 1));
	painter.drawPath(path);
	painter.translate(-0.2, -0.2);

	// Draw text
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::white);
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
