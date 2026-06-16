#include "launcherapplication.h"
#include "launcherwindow.h"
#include "gameconfig.h"
#include "patchconfig.h"
#include "controllermanager.h"
#include "globals.h"
#include <QFile>
#include <QTextStream>
#include <QStyleFactory>
#include <QDir>
#include <QFont>
#include <QFontDatabase>
#include "Windows.h"
#include "Shlobj.h"

LauncherApplication::LauncherApplication(int& argc, char** argv)
    : QApplication(argc, argv) {
    QDir(gameConfigDirectory()).mkpath(".");

    gc = new GameConfig(this);
    pc = new PatchConfig(this);
    cm = new ControllerManager(this);
    w = new LauncherWindow(0);

    // I would *like* to apply the style to the whole application
    // (for unparented MessageBoxes), but that breaks the X button image...
    w->setStyle(QStyleFactory::create("windows"));

    // Bundle the Prompt font (Regular + Bold) -- better Thai coverage than
    // Tahoma, with a real bold face. Override the kofuna theme default without
    // editing the submodule; scale heading/log box with the base to keep the
    // original hierarchy (heading = base + 1, log box = base - 1).
    QFontDatabase::addApplicationFont(":/assets/Prompt-Regular.ttf");
    QFontDatabase::addApplicationFont(":/assets/Prompt-Bold.ttf");

    QString uiFontFamily = "Prompt";
    int uiFontPx = 15;
    if (!QFontDatabase().families().contains(uiFontFamily)) {
        uiFontFamily = "Tahoma";  // fall back if the font failed to load
    }

    QFile qssFile(":/kofuna/style.qss");
    qssFile.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&qssFile);
    QFile qssFile2(":/assets/realboot.qss");
    qssFile2.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts2(&qssFile2);
    QString styleSheet = ts.readAll() + ts2.readAll();
    // Override the theme font without editing the submodule. Scale the heading
    // and log box with the base so the original kofuna hierarchy holds
    // (heading = base + 1, log box = base - 1).
    styleSheet += QStringLiteral(
                      "\nQWidget { font: %1px \"%2\"; }"
                      "\nQLabel#pageHeading { font-size: %3px; }"
                      "\nQAbstractScrollArea#logBox { font-size: %4px; }\n")
                      .arg(uiFontPx)
                      .arg(uiFontFamily)
                      .arg(uiFontPx + 1)
                      .arg(uiFontPx - 1);
    w->setStyleSheet(styleSheet);

    w->show();
    w->afterShow();
}

LauncherApplication::~LauncherApplication() { delete w; }

QString LauncherApplication::gameConfigDirectory() const {
#if defined(GAME_CHAOSHEADLCC) || defined(GAME_CHAOSCHILDLCC)
    return "./" + game_PatchConfPath;
#else
    PWSTR myDocumentsPath;
    SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &myDocumentsPath);
    QString myGamesPath =
        QString::fromWCharArray(myDocumentsPath) + "/My Games/";
    CoTaskMemFree(myDocumentsPath);
    return myGamesPath + game_GameConfPath;
#endif
}

QString LauncherApplication::patchConfigDirectory() const {
#if defined(GAME_CHAOSHEADLCC) || defined(GAME_CHAOSCHILDLCC)
    return "./" + game_PatchConfPath;
#else
    PWSTR localAppDataPath;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &localAppDataPath);
    QString patchConfigPath =
        QString::fromWCharArray(localAppDataPath) + "/" + game_PatchConfPath;
    CoTaskMemFree(localAppDataPath);
    return patchConfigPath;
#endif
}

void LauncherApplication::showWindow() { w->show(); }