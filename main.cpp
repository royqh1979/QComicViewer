/*
 * Copyright (C) 2025 Roy Qu (royqh1979@gmail.com)
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>
#include "settings.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QComicViewer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    QStringList appLocations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    QDir dataDir{appLocations.first()};
    auto settings = std::make_shared<Settings>(dataDir.filePath("config.ini"));
    pSettings = settings.get();
    settings->load();
    MainWindow w;
    w.show();
    if (argc==2) {
        w.openBook(QString::fromLocal8Bit(argv[1]));
    }
    int code = a.exec();
    settings->save();
    return code;
}
