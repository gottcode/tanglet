/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

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
	explicit ScoresDialog(QWidget* parent = nullptr);

	bool addScore(int score);
	static int isHighScore(int score);

signals:
	void scoresReset();

protected:
	void hideEvent(QHideEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

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
