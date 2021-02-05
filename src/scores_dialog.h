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

#ifndef SCORES_DIALOG
#define SCORES_DIALOG

#include <QDateTime>
#include <QDialog>
class QAbstractButton;
class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QLineEdit;

class ScoresDialog : public QDialog
{
	Q_OBJECT

public:
	ScoresDialog(QWidget* parent = nullptr);

	bool addScore(int score);
	static int isHighScore(int score);

signals:
	void scoresReset();

protected:
	void hideEvent(QHideEvent* event);
	void keyPressEvent(QKeyEvent* event);

private slots:
	void editingFinished();
	void resetClicked(QAbstractButton* button);

private:
	int addScore(const QString& name, int score, const QDateTime& date, int timer);
	void load();
	void updateItems();

private:
	QDialogButtonBox* m_buttons;

	struct Score
	{
		QString name;
		int score;
		QDateTime date;
		int timer;

		bool operator<(const Score& s) const
		{
			return score < s.score;
		}
	};
	QList<Score> m_scores;
	QString m_default_name;

	QLabel* m_score_labels[10][5];
	QGridLayout* m_scores_layout;
	QLineEdit* m_username;
	int m_row;

	static int m_max;
	static int m_min;
};

#endif
