/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "clock.h"

#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include <QTime>
#include <QTimer>

#include <algorithm>

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::Timer class defines how the timer updates for gameplay.
 */
class Clock::Timer
{
public:
	/**
	 * Constructs a timer instance.
	 */
	Timer();

	/**
	 * Destroys the timer.
	 */
	virtual ~Timer();

	/**
	 * Updates timer based on adding a correctly spelled word.
	 * @param score how much the word is worth
	 * @return @c true if the time remaining has changed by adding the word
	 */
	virtual bool addWord(int score) = 0;

	/**
	 * Updates timer based on adding an incorrectly spelled word.
	 * @param score how much the word is worth
	 * @return @c true if the time remaining has changed by adding the word
	 */
	virtual bool addIncorrectWord(int score);

	/**
	 * @return the current display color of the timer
	 */
	virtual QColor color();

	/**
	 * @return whether the timer has reached its end
	 */
	virtual bool isFinished();

	/**
	 * Resets the timer to its default amount.
	 */
	virtual void start() = 0;

	/**
	 * Stops the timer and resets to @c 0.
	 */
	virtual void stop();

	/**
	 * @return the timer unique ID
	 */
	virtual int type() const = 0;

	/**
	 * Updates the amount of time remaining and the display string of the timer.
	 * @return the current display string
	 */
	virtual QString update();

	/**
	 * @return how full the timer should be out of a maximum of 180
	 */
	virtual int width() const;

	/**
	 * Loads the timer values.
	 * @param game where to load the timer values from
	 */
	void load(const QSettings& game);

	/**
	 * Stores the timer values.
	 * @param game where to save the timer values
	 */
	void save(QSettings& game);

protected:
	int m_time; /**< The time remaining on the clock */

private:
	/**
	 * Implementation specific handling of loading timer details.
	 * @param game where to load the timer values from
	 */
	virtual void loadDetails(const QSettings& game);

	/**
	 * Implementation specific handling of saving timer details.
	 * @param game where to save the timer values
	 */
	virtual void saveDetails(QSettings& game);
};

Clock::Timer::Timer()
	: m_time(0)
{
}

Clock::Timer::~Timer()
{
}

bool Clock::Timer::addIncorrectWord(int)
{
	return false;
}

QColor Clock::Timer::color()
{
	QColor c;
	if (m_time > 20) {
		c = QColor(0x37, 0xa4, 0x2b);
	} else if (m_time > 10) {
		c = QColor(0xff, 0xaa, 0);
	} else if (m_time > 0) {
		c = QColor(0xbf, 0, 0);
	} else {
		c = QColor(0x37, 0x37, 0x37);
	}
	return c;
}

bool Clock::Timer::isFinished()
{
	return m_time == 0;
}

void Clock::Timer::stop()
{
	m_time = 0;
}

QString Clock::Timer::update()
{
	m_time = std::max(m_time - 1, 0);
	return QTime(0, 0, 0).addSecs(m_time).toString(tr("m:ss"));
}

int Clock::Timer::width() const
{
	return m_time;
}

void Clock::Timer::load(const QSettings& game)
{
	m_time = std::max(0, game.value("TimerDetails/Time", m_time).toInt()) + 1;
	loadDetails(game);
}

void Clock::Timer::save(QSettings& game)
{
	if (!isFinished()) {
		game.setValue("TimerDetails/Time", m_time);
		saveDetails(game);
	} else {
		stop();
	}
}

void Clock::Timer::loadDetails(const QSettings&)
{
}

void Clock::Timer::saveDetails(QSettings&)
{
}

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::AllotmentTimer class is a timer that allows 30 guesses.
 */
class Clock::AllotmentTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Reduce guesses remaining when a correct guess is made.
	 * @return @c true to update display
	 */
	bool addWord(int) override;

	/**
	 * Reduce guesses remaining when an incorrect guess is made.
	 * @return @c true to update display
	 */
	bool addIncorrectWord(int) override;

	/**
	 * Start with 30 guesses.
	 */
	void start() override;

	/**
	 * @return the time as "X guesses".
	 */
	QString update() override;

	/**
	 * @return the time multiplied by 6 to make it fill the timer display
	 */
	int width() const override;

private:
	void loadDetails(const QSettings&) override;
};

bool Clock::AllotmentTimer::addWord(int)
{
	m_time = std::max(m_time - 1, 0);
	return true;
}

bool Clock::AllotmentTimer::addIncorrectWord(int)
{
	m_time = std::max(m_time - 1, 0);
	return true;
}

void Clock::AllotmentTimer::start()
{
	m_time = 30;
}

int Clock::AllotmentTimer::type() const
{
	return Clock::Allotment;
}

QString Clock::AllotmentTimer::update()
{
	return tr("%n guesses(s)", "", m_time);
}

int Clock::AllotmentTimer::width() const
{
	return m_time * 6;
}

void Clock::AllotmentTimer::loadDetails(const QSettings&)
{
	m_time--;
}

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::ClassicTimer class is a timer that only counts down from 3 minutes.
 */
class Clock::ClassicTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Ignore as guesses do not impact time remaining.
	 * @return always returns @c false
	 */
	bool addWord(int) override;

	/**
	 * Start with 3 minutes.
	 */
	void start() override;
};

bool Clock::ClassicTimer::addWord(int)
{
	return false;
}

void Clock::ClassicTimer::start()
{
	m_time = 181;
}

int Clock::ClassicTimer::type() const
{
	return Clock::Classic;
}

//-----------------------------------------------------------------------------

class Clock::DisciplineTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Increase time based on how much a correct guess is worth.
	 * @param score how much the word is worth
	 * @return @c true to update display
	 */
	bool addWord(int score) override;

	/**
	 * Decrease time based on how much an incorrect guess is worth.
	 * @param score how much the word is worth
	 * @return @c true to update display
	 */
	bool addIncorrectWord(int score) override;

	/**
	 * Start with 30 seconds.
	 */
	void start() override;
};

bool Clock::DisciplineTimer::addWord(int score)
{
	m_time += score + 8;
	return true;
}

bool Clock::DisciplineTimer::addIncorrectWord(int score)
{
	m_time = std::max(0, m_time - (score + 8));
	return true;
}

void Clock::DisciplineTimer::start()
{
	m_time = 31;
}

int Clock::DisciplineTimer::type() const
{
	return Clock::Discipline;
}

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::RefillTimer class is a timer that refills on correct guesses.
 */
class Clock::RefillTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Refills time back to 30 seconds on a correct guess.
	 * @return @c true to update display
	 */
	bool addWord(int) override;

	/**
	 * Start with 30 seconds.
	 */
	void start() override;

	/**
	 * @return the time multiplied by 6 to make it fill the timer display
	 */
	int width() const override;
};

bool Clock::RefillTimer::addWord(int)
{
	m_time = 31;
	return true;
}

void Clock::RefillTimer::start()
{
	m_time = 31;
}

int Clock::RefillTimer::type() const
{
	return Clock::Refill;
}

int Clock::RefillTimer::width() const
{
	return m_time * 6;
}

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::StaminaTimer class is a timer that pauses for 5 seconds on correct guesses.
 */
class Clock::StaminaTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Pauses the timer on a correct guess.
	 * @return @c true to update display
	 */
	bool addWord(int) override;

	/**
	 * @return blue if the timer is paused, otherwise returns the regular timer color
	 */
	QColor color() override;

	/**
	 * Start with 45 seconds.
	 */
	void start() override;

	/**
	 * Reduces the pause if it exists.
	 * @return the pause remaining or the regular display if the time is not paused
	 */
	QString update() override;

	/**
	 * @return full 180 if paused, or time multiplied by 4 to fill timer display
	 */
	int width() const override;

private:
	/**
	 * Load how much pause remains.
	 * @param game where to load how much pause remains
	 */
	void loadDetails(const QSettings& game) override;

	/**
	 * Store how much pause remains.
	 * @param game where to store how much pause remains
	 */
	void saveDetails(QSettings& game) override;

private:
	int m_freeze; /**< how much pause is left */
};

bool Clock::StaminaTimer::addWord(int)
{
	m_freeze = 6;
	return true;
}

QColor Clock::StaminaTimer::color()
{
	return (m_time && m_freeze) ? QColor(0x33, 0x89, 0xea) : Timer::color();
}

void Clock::StaminaTimer::start()
{
	m_time = 46;
	m_freeze = 0;
}

int Clock::StaminaTimer::type() const
{
	return Clock::Stamina;
}

QString Clock::StaminaTimer::update()
{
	m_freeze = std::max(0, m_freeze - 1);
	return (m_freeze) ? tr("+%1").arg(m_freeze) : Timer::update();
}

int Clock::StaminaTimer::width() const
{
	if (m_freeze) {
		return 180;
	} else {
		return m_time * 4;
	}
}

void Clock::StaminaTimer::loadDetails(const QSettings& game)
{
	m_freeze = qBound(0, game.value("TimerDetails/Freeze").toInt(), 5) + 1;
}

void Clock::StaminaTimer::saveDetails(QSettings& game)
{
	game.setValue("TimerDetails/Freeze", m_freeze);
}

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::StrikeoutTimer class is a timer that stops after 3 incorrect guesses.
 */
class Clock::StrikeoutTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Ignore adding a correct word as guesses do not impact time remaining.
	 * @return always returns @c false
	 */
	bool addWord(int) override;

	/**
	 * Decreases amount of strikes remaining.
	 * @return @c true to update timer display
	 */
	bool addIncorrectWord(int score) override;

	/**
	 * Start with 0 incorrect guesses.
	 */
	void start() override;

	/**
	 * Resets count of incorrect guesses.
	 */
	void stop() override;

	/**
	 * @return how many guesses remain
	 */
	QString update() override;

	/**
	 * @return remaining guesses multipled by 60 to fill timer display
	 */
	int width() const override;

private:
	/**
	 * Load how many incorrect guesses have happened.
	 * @param game where to load the count of incorrect guesses
	 */
	void loadDetails(const QSettings& game) override;

	/**
	 * Store how many incorrect guesses have happened.
	 * @param game where to store count of incorrect guesses
	 */
	void saveDetails(QSettings& game) override;

private:
	int m_strikes; /**< how many incorrect guesses have been made */
};

bool Clock::StrikeoutTimer::addWord(int)
{
	return false;
}

bool Clock::StrikeoutTimer::addIncorrectWord(int)
{
	m_strikes = std::min(m_strikes + 1, 3);
	m_time = (3 - m_strikes) * 10;
	return true;
}

void Clock::StrikeoutTimer::start()
{
	m_time = 30;
	m_strikes = 0;
}

void Clock::StrikeoutTimer::stop()
{
	m_time = 0;
	m_strikes = 3;
}

int Clock::StrikeoutTimer::type() const
{
	return Clock::Strikeout;
}

QString Clock::StrikeoutTimer::update()
{
	return tr("%n strike(s)", "", m_strikes);
}

int Clock::StrikeoutTimer::width() const
{
	return (3 - m_strikes) * 60;
}

void Clock::StrikeoutTimer::loadDetails(const QSettings& game)
{
	m_strikes = qBound(0, game.value("TimerDetails/Strikes").toInt(), 3);
	m_time = (3 - m_strikes) * 10;
}

void Clock::StrikeoutTimer::saveDetails(QSettings& game)
{
	game.setValue("TimerDetails/Strikes", m_strikes);
}

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::TangletTimer class is a timer that rewards on correct guesses.
 */
class Clock::TangletTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Increase time based on how much a correct guess is worth.
	 * @param score how much the word is worth
	 * @return @c true to update display
	 */
	bool addWord(int score) override;

	/**
	 * Start with 30 seconds.
	 */
	void start() override;
};

bool Clock::TangletTimer::addWord(int score)
{
	m_time += score + 8;
	return true;
}

void Clock::TangletTimer::start()
{
	m_time = 31;
}

int Clock::TangletTimer::type() const
{
	return Clock::Tanglet;
}

//-----------------------------------------------------------------------------

/**
 * @brief The Clock::UnlimitedTimer class is a timer that does nothing.
 */
class Clock::UnlimitedTimer : public Clock::Timer
{
public:
	int type() const override;

	/**
	 * Ignore adding a correct word as guesses do not impact anything.
	 * @return always returns @c false
	 */
	bool addWord(int) override;

	/**
	 * Start with 180 seconds to fill display.
	 */
	void start() override;

	/**
	 * @return infinity symbol
	 */
	QString update() override;
};

bool Clock::UnlimitedTimer::addWord(int)
{
	return false;
}

void Clock::UnlimitedTimer::start()
{
	m_time = 180;
}

int Clock::UnlimitedTimer::type() const
{
	return Clock::Unlimited;
}

QString Clock::UnlimitedTimer::update()
{
	return "∞";
}

//-----------------------------------------------------------------------------

Clock::Clock(QWidget* parent)
	: QWidget(parent)
	, m_timer(nullptr)
{
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

Clock::~Clock()
{
	delete m_timer;
}

//-----------------------------------------------------------------------------

QSize Clock::sizeHint() const
{
	return QSize(186, fontMetrics().height() + 8);
}

//-----------------------------------------------------------------------------

bool Clock::isFinished() const
{
	return m_timer->isFinished();
}

//-----------------------------------------------------------------------------

void Clock::addWord(int score)
{
	if (m_timer->addWord(score)) {
		updateTime();
	}
}

//-----------------------------------------------------------------------------

void Clock::addIncorrectWord(int score)
{
	if (m_timer->addIncorrectWord(score)) {
		updateTime();
	}
}

//-----------------------------------------------------------------------------

void Clock::setPaused(bool paused)
{
	if (paused) {
		m_update->stop();
	} else {
		m_update->start();
	}
}

//-----------------------------------------------------------------------------

void Clock::setText(const QString& text)
{
	m_text = text;
	update();
}

//-----------------------------------------------------------------------------

void Clock::start()
{
	m_timer->start();
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::stop()
{
	m_timer->stop();
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::load(const QSettings& game)
{
	m_timer->load(game);
	updateTime();
}

//-----------------------------------------------------------------------------

void Clock::save(QSettings& game)
{
	m_timer->save(game);
}

//-----------------------------------------------------------------------------

void Clock::setTimer(int timer)
{
	if (!m_timer || m_timer->type() != timer) {
		delete m_timer;

		switch (timer) {
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
		case Discipline:
			m_timer = new DisciplineTimer;
			break;
		case Unlimited:
			m_timer = new UnlimitedTimer;
			break;
		case Tanglet:
		default:
			m_timer = new TangletTimer;
			break;
		}
	}
}

//-----------------------------------------------------------------------------

int Clock::timer() const
{
	return m_timer->type();
}

//-----------------------------------------------------------------------------

QString Clock::timerToString(int timer)
{
	static QStringList timers = QStringList()
			<< tr("Tanglet")
			<< tr("Classic")
			<< tr("Refill")
			<< tr("Stamina")
			<< tr("Strikeout")
			<< tr("Allotment")
			<< tr("Discipline")
			<< tr("Unlimited");
	return timers.at(qBound(0, timer, TotalTimers - 1));
}

//-----------------------------------------------------------------------------

QString Clock::timerDescription(int timer)
{
	static QStringList timers = QStringList()
			<< tr("Counts down from 30 seconds and increases on correct guesses.")
			<< tr("Counts down from 3 minutes.")
			<< tr("Counts down from 30 seconds and refills on correct guesses.")
			<< tr("Counts down from 45 seconds and pauses on correct guesses.")
			<< tr("Game ends after 3 incorrect guesses.")
			<< tr("Game ends after 30 guesses.")
			<< tr("Counts down from 30 seconds and increases or decreases on guesses.")
			<< tr("Game ends when all words are found.");
	return timers.at(qBound(0, timer, TotalTimers - 1));
}

//-----------------------------------------------------------------------------

QString Clock::timerScoresGroup(int timer)
{
	static QStringList timers = QStringList()
			<< QStringLiteral("Scores_Tanglet")
			<< QStringLiteral("Scores_Classic")
			<< QStringLiteral("Scores_Refill")
			<< QStringLiteral("Scores_Stamina")
			<< QStringLiteral("Scores_Strikeout")
			<< QStringLiteral("Scores_Allotment")
			<< QStringLiteral("Scores_Discipline")
			<< QStringLiteral("Scores_Unlimited");
	return timers.at(qBound(0, timer, TotalTimers - 1));
}

//-----------------------------------------------------------------------------

void Clock::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);

	QColor color = m_timer->color();

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setPen(Qt::NoPen);

	// Draw indent
	QLinearGradient indent_gradient(0, 0, 0, height());
	QColor shadow = palette().color(QPalette::Shadow);
	indent_gradient.setColorAt(0, QColor(shadow.red(), shadow.green(), shadow.blue(), 64));
	indent_gradient.setColorAt(1, QColor(shadow.red(), shadow.green(), shadow.blue(), 0));
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
		painter.setBrush(QColor(255, 255, 255, 160));
		painter.drawRoundedRect(x + 3, 2, width + 1, rect().height() - 4, 2, 2);

		// Draw dividing line
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.setPen(color.darker(150));
		painter.drawLine(x + 3, 2, x + 3, rect().height() - 2);
		painter.setRenderHint(QPainter::Antialiasing, true);
	}

	// Draw text shadow
	QPainterPath path;
	path.addText(93 - (fontMetrics().boundingRect(m_text).width() / 2), fontMetrics().ascent() + 3, font(), m_text);
	painter.setBrush(Qt::black);
	painter.translate(0.6, 0.6);
	painter.setPen(QPen(QColor(0, 0, 0, 32), 3));
	painter.drawPath(path);
	painter.translate(-0.2, -0.2);
	painter.setPen(QPen(QColor(0, 0, 0, 64), 2));
	painter.drawPath(path);
	painter.translate(-0.2, -0.2);
	painter.setPen(QPen(QColor(0, 0, 0, 192), 1));
	painter.drawPath(path);
	painter.translate(-0.2, -0.2);

	// Draw text
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::white);
	painter.fillPath(path, Qt::white);
}

//-----------------------------------------------------------------------------

void Clock::updateTime()
{
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
