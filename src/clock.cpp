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

Clock::Clock(QWidget* parent)
: QWidget(parent), m_type(0) {
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

bool Clock::isFinished() const {
	return m_type->isFinished();
}

//-----------------------------------------------------------------------------

void Clock::addWord(int score) {
	if (m_type->addWord(score)) {
		updateTime();
	}
}

//-----------------------------------------------------------------------------

void Clock::addIncorrectWord(int score) {
	if (m_type->addIncorrectWord(score)) {
		updateTime();
	}
}

//-----------------------------------------------------------------------------

void Clock::setMode(int mode) {
	if (!m_type || m_type->type() != mode) {
		delete m_type;

		switch (mode) {
		case TangletMode:
			m_type = new TangletType;
			break;
		case ClassicMode:
			m_type = new ClassicType;
			break;
		case RefillMode:
			m_type = new RefillType;
			break;
		case StaminaMode:
			m_type = new StaminaType;
			break;
		case StrikeoutMode:
			m_type = new StrikeoutType;
			break;
		case AllotmentMode:
		default:
			m_type = new AllotmentType;
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
	m_type->stop();
	updateTime();
}

//-----------------------------------------------------------------------------

QString Clock::modeString(int mode) {
	static QStringList timers = QStringList() <<
		tr("Tanglet") <<
		tr("Classic") <<
		tr("Refill") <<
		tr("Stamina") <<
		tr("Strikeout") <<
		tr("Allotment");
	return timers.at(qBound(0, mode, timers.count() - 1));
}

//-----------------------------------------------------------------------------

void Clock::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QColor color = m_type->color();

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setPen(Qt::NoPen);

	painter.setBrush(color);
	painter.drawRoundedRect(rect(), 5, 5);

	int secs = qMin(180, m_type->time());
	int width = 180 - secs;
	if (width < 180) {
		painter.setBrush(QColor(255, 255, 255, 160));
		painter.drawRoundedRect(secs + 3, 3, width, rect().height() - 6, 2.5, 2.5);
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
	m_text = m_type->update();
	update();

	if (!isFinished()) {
		m_timer->start();
	} else {
		m_timer->stop();
		emit finished();
	}
}

//-----------------------------------------------------------------------------

Clock::Type::Type()
	: m_time(0) {
}

Clock::Type::~Type() {
}

bool Clock::Type::addIncorrectWord(int score) {
	Q_UNUSED(score);
	return false;
}

void Clock::Type::stop() {
	m_time = 0;
}

QString Clock::Type::update() {
	m_time = qMax(m_time - 1, 0);
	//: Format time for the clock
	return QTime(0,0,0).addSecs(m_time).toString(tr("m:ss"));
}

QColor Clock::Type::color() {
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

//-----------------------------------------------------------------------------

bool Clock::AllotmentType::addWord(int score) {
	Q_UNUSED(score);
	m_time = qMax(m_time - 1, 0);
	return true;
}

bool Clock::AllotmentType::addIncorrectWord(int score) {
	Q_UNUSED(score);
	m_time = qMax(m_time - 1, 0);
	return true;
}

void Clock::AllotmentType::start() {
	m_time = 30;
}

int Clock::AllotmentType::time() const {
	return m_time * 6;
}

int Clock::AllotmentType::type() const {
	return Clock::AllotmentMode;
}

QString Clock::AllotmentType::update() {
	return tr("%n guesses(s)", "", m_time);
}

//-----------------------------------------------------------------------------

bool Clock::ClassicType::addWord(int score) {
	Q_UNUSED(score);
	return false;
}

void Clock::ClassicType::start() {
	m_time = 181;
}

int Clock::ClassicType::type() const {
	return Clock::ClassicMode;
}

//-----------------------------------------------------------------------------

bool Clock::RefillType::addWord(int score) {
	Q_UNUSED(score);
	m_time = 31;
	return true;
}

void Clock::RefillType::start() {
	m_time = 31;
}

int Clock::RefillType::time() const {
	return m_time * 6;
}

int Clock::RefillType::type() const {
	return Clock::RefillMode;
}

//-----------------------------------------------------------------------------

bool Clock::StaminaType::addWord(int score) {
	Q_UNUSED(score);
	m_freeze = 6;
	return true;
}

void Clock::StaminaType::start() {
	m_time = 46;
	m_freeze = 0;
}

int Clock::StaminaType::time() const {
	if (m_freeze) {
		return 180;
	} else {
		return m_time * 4;
	}
}

int Clock::StaminaType::type() const {
	return Clock::StaminaMode;
}

QString Clock::StaminaType::update() {
	m_freeze = qMax(0, m_freeze - 1);
	if (m_freeze) {
		return tr("+%1").arg(m_freeze);
	} else {
		m_time = qMax(m_time - 1, 0);
		return tr("0:%1").arg(m_time, 2, 10, QLatin1Char('0'));
	}
}

QColor Clock::StaminaType::color() {
	return (m_freeze) ? "#3389ea" : Type::color();
}

//-----------------------------------------------------------------------------

bool Clock::StrikeoutType::addWord(int score) {
	Q_UNUSED(score);
	return false;
}

bool Clock::StrikeoutType::addIncorrectWord(int score) {
	Q_UNUSED(score);
	m_strikes = qMin(m_strikes + 1, 3);
	m_time = (3 - m_strikes) * 10;
	return true;
}

void Clock::StrikeoutType::start() {
	m_time = 30;
	m_strikes = 0;
}

int Clock::StrikeoutType::time() const {
	return (3 - m_strikes) * 60;
}

int Clock::StrikeoutType::type() const {
	return Clock::StrikeoutMode;
}

QString Clock::StrikeoutType::update() {
	return tr("%n strike(s)", "", m_strikes);
}

//-----------------------------------------------------------------------------

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
