/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2011 Graeme Gott <graeme@gottcode.org>
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

#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
class QSettings;
class QTimer;

class Clock : public QWidget
{
	Q_OBJECT

public:
	Clock(QWidget* parent = 0);
	~Clock();

	virtual QSize sizeHint() const;

	bool isFinished() const;

	void addWord(int score);
	void addIncorrectWord(int score);
	void setPaused(bool paused);
	void setText(const QString& text);
	void start();
	void stop();

	void load(const QSettings& game);
	void save(QSettings& game);

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
	void setTimer(int timer);
	int timer() const;
	static QString timerToString(int timer);
	static QString timerDescription(int timer);

signals:
	void finished();

protected:
	virtual void paintEvent(QPaintEvent* event);

private slots:
	void updateTime();

private:
	QString m_text;
	QTimer* m_update;

	class Timer;
	class AllotmentTimer;
	class ClassicTimer;
	class DisciplineTimer;
	class RefillTimer;
	class StaminaTimer;
	class StrikeoutTimer;
	class TangletTimer;
	Timer* m_timer;
};

#endif
