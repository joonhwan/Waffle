#include <QApplication>
#include "packet/WSynchronousFtp.h"
#include <QDebug>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

	WSynchronousFtp ftp;
	ftp.setHost("skysc.is-a-geek.net");
	ftp.setUser("skysc");
	ftp.setPassword("RkRndUFO");

	bool done = false;

	do
	{
		qDebug() << "ftp connecting...";
		if (!ftp.open()) {
			qDebug() << "unable to open ftp site!";
			break;
		}
		qDebug() << "ftp connected.";

		qDebug() << "file listing...";
		QTextCodec* codec= QTextCodec::codecForName("euc-kr");
		if (!ftp.fileListAt(codec->toUnicode("/My Movie/영화/!!평점순위별 TOP 300/271위.[평점9.00] 미션"))) {
			qDebug() << "unable to list files!";
			break;
		}
		foreach(const QString& s, ftp.lastFileList()) {
			qDebug() << s;
		}

		bool found = ftp.fileExists(codec->toUnicode("/My Movie/영화/!!평점순위별 TOP 300/271위.[평점9.00] 미션/미션 (The Mission, 1986) XviD.AC3.CD1-WAF.avi"));
		qDebug() << "file find check... found ? " << found;

		found = ftp.dirExists(codec->toUnicode("/My Movie/영화/!!평점순위별 TOP 300/271위.[평점9.00] 미션"));
		qDebug() << "dir find check... found? " << found;

		if (!ftp.close()) {
			qDebug() << "unable to close ftp!";
		}
	} while (0);

	return 0;
}
