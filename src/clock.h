/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
class QSettings;
class QTimer;

/**
 * @brief The Clock class manages and displays how much time is left.
 */
class Clock : public QWidget
{
	Q_OBJECT

	class Timer;
	class AllotmentTimer;
	class ClassicTimer;
	class DisciplineTimer;
	class RefillTimer;
	class StaminaTimer;
	class StrikeoutTimer;
	class TangletTimer;

public:
	/** The different timer modes that control the clock. */
	enum Mode
	{
		Tanglet,
		Classic,
		Refill,
		Stamina,
		Strikeout,
		Allotment,
		Discipline,
		TotalTimers
	};

	/**
	 * Constructs a clock instance.
	 * @param parent the QWidget that manages the clock
	 */
	explicit Clock(QWidget* parent = nullptr);

	/**
	 * Destroys the clock.
	 */
	~Clock();

	/**
	 * @return the size required to show the clock
	 */
	QSize sizeHint() const override;

	/**
	 * @return whether the current timer has finished
	 */
	bool isFinished() const;

	/**
	 * Updates the clock based on a correct guess.
	 * @param score how many points the guess was worth
	 */
	void addWord(int score);

	/**
	 * Updates the clock based on an incorrect guess.
	 * @param score how many points the guess was worth
	 */
	void addIncorrectWord(int score);

	/**
	 * Pauses or resumes the clock.
	 * @param paused pauses the clock if @c true
	 */
	void setPaused(bool paused);

	/**
	 * Override the text display of the clock. Used to inform the player a game has ended.
	 * @param text the text to display
	 */
	void setText(const QString& text);

	/**
	 * Starts the clock.
	 */
	void start();

	/**
	 * Stops the clock and resets it to @c 0.
	 */
	void stop();

	/**
	 * Loads the clock details.
	 * @param game where to read the clock details
	 */
	void load(const QSettings& game);

	/**
	 * Saves the clock details.
	 * @param game where to store the clock details
	 */
	void save(QSettings& game);

	/**
	 * Configures how the clock tracks time.
	 * @param timer the timer mode
	 */
	void setTimer(int timer);

	/**
	 * @return the current timer mode
	 */
	int timer() const;

	/**
	 * Fetch the translated name of a timer.
	 * @param timer the timer mode
	 * @return translated name of timer
	 */
	static QString timerToString(int timer);

	/**
	 * Fetch the translated description of a timer.
	 * @param timer the timer mode
	 * @return translated description of timer
	 */
	static QString timerDescription(int timer);

signals:
	/**
	 * Emitted when the clock reaches @c 0.
	 */
	void finished();

protected:
	/**
	 * Draws the clock.
	 * @param event the QWidget paint event details
	 */
	void paintEvent(QPaintEvent* event) override;

private slots:
	/**
	 * Calculates the current time remaining. If finished, it calls stop() to clear the time and
	 * emits the finished() signal to end the game.
	 */
	void updateTime();

private:
	QString m_text; /**< the string to display */
	QTimer* m_update; /**< the tick used to update the timer */

	Timer* m_timer; /**< the current timer */
};

#endif
