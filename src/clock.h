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

#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
class QTimer;

class Clock : public QWidget {
	Q_OBJECT

	public:
		Clock(QWidget* parent = 0);
		~Clock();

		virtual QSize sizeHint() const;

		bool isFinished() const {
			return m_time == 0;
		}

		void addWord(int score);
		void setMode(int mode);
		void setPaused(bool paused);
		void setText(const QString& text);
		void start();
		void stop();

		enum Mode {
			TangletMode,
			ClassicMode,
			RefillMode,
			TotalModes
		};
		static QString modeString(int mode);

	signals:
		void finished();

	protected:
		virtual void paintEvent(QPaintEvent* event);

	private slots:
		void updateTime();

	private:
		int m_time;
		QString m_text;
		QTimer* m_timer;

		class Type {
		public:
			Type(int& time);
			virtual ~Type();
			virtual bool addWord(int score)=0;
			virtual void start()=0;
			virtual int type() const=0;

		protected:
			int& m_time;
		};
		Type* m_type;

		class ClassicType : public Type {
		public:
			ClassicType(int& time);
			bool addWord(int score);
			void start();
			int type() const;
		};

		class RefillType : public Type {
		public:
			RefillType(int& time);
			bool addWord(int score);
			void start();
			int type() const;
		};

		class TangletType : public Type {
		public:
			TangletType(int& time);
			bool addWord(int score);
			void start();
			int type() const;
		};
};

#endif
