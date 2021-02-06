/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
class QSettings;
class QTimer;

class Clock : public QWidget
{
	Q_OBJECT

public:
	Clock(QWidget* parent = nullptr);
	~Clock();

	QSize sizeHint() const override;

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
	void paintEvent(QPaintEvent* event) override;

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
