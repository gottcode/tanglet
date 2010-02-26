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

		bool isFinished() const;

		void addWord(int score);
		void addIncorrectWord(int score);
		void setMode(int mode);
		void setPaused(bool paused);
		void setText(const QString& text);
		void start();
		void stop();

		enum Mode {
			TangletMode,
			ClassicMode,
			RefillMode,
			StaminaMode,
			StrikeoutMode,
			AllotmentMode,
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
		QString m_text;
		QTimer* m_timer;

		class Type {
		public:
			Type();
			virtual ~Type();

			virtual bool isFinished() {
				return m_time == 0;
			}

			virtual int time() const {
				return m_time;
			}

			virtual bool addWord(int score)=0;
			virtual bool addIncorrectWord(int score);
			virtual void start()=0;
			virtual void stop();
			virtual QString update();

			virtual QColor color();
			virtual int type() const=0;

		protected:
			int m_time;
		};
		Type* m_type;

		class AllotmentType : public Type {
		public:
			bool addWord(int score);
			bool addIncorrectWord(int score);
			void start();
			int time() const;
			int type() const;
			QString update();
		};

		class ClassicType : public Type {
		public:
			bool addWord(int score);
			void start();
			int type() const;
		};

		class RefillType : public Type {
		public:
			bool addWord(int score);
			void start();
			int time() const;
			int type() const;
		};

		class StaminaType : public Type {
		public:
			bool addWord(int score);
			void start();
			int time() const;
			int type() const;
			QString update();
			QColor color();

		private:
			int m_freeze;
		};

		class StrikeoutType : public Type {
		public:
			bool addWord(int score);
			bool addIncorrectWord(int score);
			void start();
			int time() const;
			int type() const;
			QString update();

		private:
			int m_strikes;
		};

		class TangletType : public Type {
		public:
			bool addWord(int score);
			void start();
			int type() const;
		};
};

#endif
