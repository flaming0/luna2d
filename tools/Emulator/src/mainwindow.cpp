//-----------------------------------------------------------------------------
// luna2d Emulator
// This is part of luna2d engine
// Copyright 2014-2016 Stepan Prokofjev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QProcess>
#include <QDateTime>
#include <QStandardPaths>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	logStorage(new LogStorage()),
	logDlg(nullptr),
	curGamePath(QString::null)
{
	ui->setupUi(this);

	SetupRecentGames();
	SetupResolutionMenu();
	SetResolution(Settings::curResolution);

	ui->actionRestart_game->setEnabled(false);
	ui->actionClose_game->setEnabled(false);
	ui->actionRun_project->setEnabled(false);
	ui->actionRun_project_restart->setEnabled(false);
	ui->actionOpen_in_Pipeline->setEnabled(false);
	ui->actionSet_project->setEnabled(false);
	ui->actionPipelineProject->setText(MENU_NO_PIPELINE_PROJECT);
	ui->actionTake_screenshot->setEnabled(false);

	// Set placeholder image to engine widget
	ui->centralWidget->SetPlaceholderImage(QImage(":/images/placeholder_image.png"));

	connect(ui->centralWidget, &luna2d::LUNAQtWidget::glSurfaceInitialized, this, &MainWindow::OnGlSurfaceInitialized);
	connect(ui->centralWidget, &luna2d::LUNAQtWidget::gameLoopIteration, this, &MainWindow::OnGameLoopIteration);
	connect(ui->centralWidget, &luna2d::LUNAQtWidget::logInfo, logStorage, &LogStorage::OnLogInfo);
	connect(ui->centralWidget, &luna2d::LUNAQtWidget::logWarning, logStorage, &LogStorage::OnLogWarning);
	connect(ui->centralWidget, &luna2d::LUNAQtWidget::logError, logStorage, &LogStorage::OnLogError);
	connect(ui->centralWidget, &luna2d::LUNAQtWidget::logError, this, &MainWindow::OnLogError);
	connect(ui->actionOpen_game, &QAction::triggered, this, &MainWindow::OnActionOpen);
	connect(ui->actionRestart_game, &QAction::triggered, this, &MainWindow::OnActionRestart);
	connect(ui->actionClose_game, &QAction::triggered, this, &MainWindow::OnActionClose);
	connect(ui->actionLog, &QAction::triggered, this, &MainWindow::OnActionLog);
	connect(ui->actionWatcher, &QAction::triggered, this, &MainWindow::OnActionWatcher);
	connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::OnActionSettings);
	connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::OnAbout);
	connect(ui->actionRun_project, &QAction::triggered, this, &MainWindow::OnRunPipelineProject);
	connect(ui->actionRun_project_restart, &QAction::triggered, this, &MainWindow::OnRunPipelineProjectAndRestart);
	connect(ui->actionOpen_in_Pipeline, &QAction::triggered, this, &MainWindow::OnOpenInPipeline);
	connect(ui->actionSet_project, &QAction::triggered, this, &MainWindow::OnSetPipelineProject);
	connect(ui->actionTake_screenshot, &QAction::triggered, this, &MainWindow::OnTakeScreenshot);
	connect(ui->actionOpen_screenshots_folder, &QAction::triggered, this, &MainWindow::OnOpenScreenshotsFolder);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete logStorage;
}

// Setup recent games menu
void MainWindow::SetupRecentGames()
{
	// Remove previous items
	ui->menuRecent->clear();

	// Disable recent games menu if list of recent games is empty
	ui->menuRecent->setEnabled(!Settings::recentGames.empty());

	// Add all recent games from settings
	for(QString game : Settings::recentGames)
	{
		QAction* action = new QAction(game, this);
		ui->menuRecent->addAction(action);
		connect(action, &QAction::triggered, this, &MainWindow::OnRecentGame);
	}
}

void MainWindow::SetupResolutionMenu()
{
	ui->menuResolution->clear();

	QString format = "%1 (%2x%3)";
	QActionGroup *group = new QActionGroup(this);

	for(int i = 0; i < Settings::resolutions.size(); i++)
	{
		auto& res = Settings::resolutions.at(i);

		QAction *action = new QAction(format.arg(res.name).arg(res.width).arg(res.height), this);
		action->setCheckable(true);
		action->setActionGroup(group);
		action->setData(i);
		ui->menuResolution->addAction(action);
		connect(action, &QAction::triggered, this, &MainWindow::OnResolutionChanged);

		// Check current selected resolution
		if(i == Settings::curResolution) action->setChecked(true);

		// Add separator after bullit-in resolutions
		if(i == Settings::bullitinCount - 1) ui->menuResolution->addSeparator();
	}
}

// Check given directory for it's valid game directory
QString MainWindow::CheckGameDirectory(const QString &path)
{
	QDir dir(path);
	if(!dir.exists("scripts/")) return "directory \"scripts\" not found";

	dir.cd("scripts/");
	if(!dir.exists("main.lua")) return "\"main.lua\" not found";

	return QString::Null();
}

// Launch game from given path
void MainWindow::OpenGame(const QString &gamePath)
{
	QString error = CheckGameDirectory(gamePath);

	if(!error.isNull())
	{
		QString errorMessage = "Cannot open game: " + error;
		QMessageBox::critical(this, "Error opening game", errorMessage);
		return;
	}

	// Update recent games list
	Settings::AddRecentGame(gamePath);
	Settings::gameWasOpened = true;
	SetupRecentGames();

	curGamePath = gamePath;
	ui->actionRestart_game->setEnabled(true);
	ui->actionClose_game->setEnabled(true);

	// Clear log
	if(Settings::clearLogOnStart)
	{
		logStorage->Clear();
		if(logDlg) logDlg->Clear();
	}

	// Launch game
	auto& resolution = Settings::resolutions.at(Settings::curResolution);
	ui->centralWidget->DeinitializeEngine();
	ui->centralWidget->InitializeEngine(gamePath, resolution.width, resolution.height);

	// Update window title
	curGameName = ui->centralWidget->GetGameName();
	if(!Settings::showFps) setWindowTitle(WINDOW_TITLE_NAME.arg(curGameName));

	// Update "Tools/Pipeline" menu
	ui->actionSet_project->setEnabled(true);
	UpdatePipelineMenu();

	// Resize pixmap for screenshots
	screenshotsPixmap = QPixmap(ui->centralWidget->size());
	ui->actionTake_screenshot->setEnabled(true);
}

void MainWindow::SetResolution(int resolutionIndex)
{
	const Resolution &resolution = Settings::resolutions.at(resolutionIndex);

	int wndWidth = resolution.width;
	int wndHeight = resolution.height;
	int screenWidth = QApplication::desktop()->availableGeometry().width() - SCREEN_MARGIN;
	int screenHeight = QApplication::desktop()->availableGeometry().height() - SCREEN_MARGIN;

	// Downscale window if emulator resolution large than screen resolution
	if(wndWidth > screenWidth || wndHeight + ui->menuBar->height() > screenHeight)
	{
		float scaleWidth = screenWidth / (float)wndWidth;
		float scaleHeight = screenHeight / (float)wndHeight;
		float scale = std::min(scaleWidth, scaleHeight);

		wndWidth = std::round((float)wndWidth * scale);
		wndHeight = std::round((float)wndHeight * scale);
	}

	setFixedSize(wndWidth, wndHeight + ui->menuBar->height()); // Consider menubar height
	MoveToCenter();

	Settings::curResolution = resolutionIndex;
}

void MainWindow::OpenLogDialog()
{
	logDlg = new LogDialog(logStorage, this);
	logDlg->show();
	connect(logDlg, &QDialog::rejected, this, &MainWindow::OnLogClosed);

	connect(ui->centralWidget, &luna2d::LUNAQtWidget::logInfo, logDlg, &LogDialog::OnLogInfo);
	connect(ui->centralWidget, &luna2d::LUNAQtWidget::logWarning, logDlg, &LogDialog::OnLogWarning);
	connect(ui->centralWidget, &luna2d::LUNAQtWidget::logError, logDlg, &LogDialog::OnLogError);
}

// Update "Tools/Pipeline" menu
void MainWindow::UpdatePipelineMenu()
{
	QString pipelineProject = Settings::GetPipelineProject(curGameName);
	bool hasProject = !pipelineProject.isEmpty();

	ui->actionRun_project->setEnabled(hasProject);
	ui->actionRun_project_restart->setEnabled(hasProject);
	ui->actionOpen_in_Pipeline->setEnabled(hasProject);

	if(hasProject) ui->actionPipelineProject->setText(pipelineProject);
	else ui->actionPipelineProject->setText(MENU_NO_PIPELINE_PROJECT);
}

QString MainWindow::MakeScreenhotsFolder()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/luna2d/Emulator/";
	path += ui->centralWidget->GetGameName() + "/";

	QDir dir(path);
	if(!dir.exists()) dir.mkpath(".");

	return path;
}

void MainWindow::OnGlSurfaceInitialized()
{
	// Try open game from command line
	QCommandLineParser cmdParser;
	cmdParser.process(*QApplication::instance());

	QStringList args = cmdParser.positionalArguments();
	if(!args.empty())
	{
		QString path = args.first();
		path.remove("\""); // Remove quotes from argument
		OpenGame(path);
		return;
	}

	// Launch last recent game on startup
	if(Settings::openLastGame && Settings::gameWasOpened && !Settings::recentGames.empty())
	{
		OpenGame(Settings::recentGames.first());
	}
}

void MainWindow::OnGameLoopIteration()
{
	if(Settings::showFps) setWindowTitle(WINDOW_TITLE_FPS.arg(curGameName).arg(ui->centralWidget->GetFps()));
}

void MainWindow::OnActionOpen()
{
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::DirectoryOnly);
	dialog.setOption(QFileDialog::ShowDirsOnly);
	dialog.setWindowTitle("Select game directory");

	if(dialog.exec())
	{
		QString gamePath = dialog.selectedFiles()[0];
		OpenGame(gamePath);
	}
}

void MainWindow::OnActionRestart()
{
	if(curGamePath != QString::null) OpenGame(curGamePath);
}

void MainWindow::OnActionClose()
{
	ui->centralWidget->DeinitializeEngine();
	curGamePath = QString::null;
	ui->actionRestart_game->setEnabled(false);
	ui->actionClose_game->setEnabled(false);
	ui->actionRun_project->setEnabled(false);
	ui->actionRun_project_restart->setEnabled(false);
	ui->actionOpen_in_Pipeline->setEnabled(false);
	ui->actionSet_project->setEnabled(false);
	ui->actionTake_screenshot->setEnabled(false);
	Settings::gameWasOpened = false;

	setWindowTitle(WINDOW_TITLE);
	ui->actionPipelineProject->setText(MENU_NO_PIPELINE_PROJECT);
}

void MainWindow::OnRecentGame()
{
	QString gamePath = qobject_cast<QAction*>(sender())->text();
	OpenGame(gamePath);
}

void MainWindow::OnActionLog()
{
	if(ui->actionLog->isChecked()) OpenLogDialog();
	else
	{
		delete logDlg;
		logDlg = nullptr;
	}
}

void MainWindow::OnActionWatcher()
{
	if(ui->actionWatcher->isChecked())
	{
		watcherDlg = new WatcherDialog(ui->centralWidget, this);
		watcherDlg->show();
		connect(watcherDlg, &QDialog::rejected, this, &MainWindow::OnWatcherClosed);
	}
	else
	{
		delete watcherDlg;
		watcherDlg = nullptr;
	}
}

void MainWindow::OnResolutionChanged()
{
	// Set new resolution
	int resolutionIndex = qobject_cast<QAction*>(sender())->data().toInt();
	SetResolution(resolutionIndex);

	// Relaunch game with new resolution
	OnActionRestart();
}

void MainWindow::OnActionSettings()
{
	QDialog dialog(this);
	Ui::SettingsDialog uiDialog;
	uiDialog.setupUi(&dialog);

	uiDialog.openLastGame->setChecked(Settings::openLastGame);
	uiDialog.openLogWhenError->setChecked(Settings::openLogWhenError);
	uiDialog.showFps->setChecked(Settings::showFps);

	if(dialog.exec())
	{
		Settings::openLastGame = uiDialog.openLastGame->isChecked();
		Settings::openLogWhenError = uiDialog.openLogWhenError->isChecked();
		Settings::showFps = uiDialog.showFps->isChecked();
		Settings::Save();

		if(!Settings::showFps) setWindowTitle(WINDOW_TITLE_NAME.arg(curGameName));
	}
}

void MainWindow::OnAbout()
{
	QMessageBox::about(this, "About", "luna2d Emulator\nThis is part of luna2d engine\nCopyright 2014-2016 Stepan Prokofjev");
}

void MainWindow::OnLogClosed()
{
	logDlg = nullptr;
	ui->actionLog->setChecked(false);
}

void MainWindow::OnWatcherClosed()
{
	watcherDlg = nullptr;
	ui->actionWatcher->setChecked(false);
}

void MainWindow::OnLogError()
{
	if(!Settings::openLogWhenError) return;

	// Open log window when occurs log message with error
	if(logDlg) logDlg->activateWindow();
	else OpenLogDialog();
}

void MainWindow::OnRunPipelineProject()
{
	QString projectPath = Settings::GetPipelineProject(curGameName);
	if(projectPath.isEmpty()) return;

	QString pipelinePath = QApplication::applicationDirPath() + "/../Pipeline/Pipeline";
	QProcess pipeline;

	pipeline.start(pipelinePath, { projectPath, "-u" });
	pipeline.waitForFinished();
}

void MainWindow::OnRunPipelineProjectAndRestart()
{
	OnRunPipelineProject();
	OnActionRestart();
}

void MainWindow::OnOpenInPipeline()
{
	QString projectPath = Settings::GetPipelineProject(curGameName);
	if(projectPath.isEmpty()) return;

	QString pipelinePath = QApplication::applicationDirPath() + "/../Pipeline/Pipeline";
	QProcess pipeline;

	pipeline.startDetached(pipelinePath, { projectPath });
}

void MainWindow::OnSetPipelineProject()
{
	QFileDialog dialog;
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setNameFilter("Pipeline project (*.pipeline)");

	if(dialog.exec())
	{
		QString projectPath = dialog.selectedFiles().first();
		Settings::SetPipelineProject(curGameName, projectPath);
		UpdatePipelineMenu();
	}
}

void MainWindow::OnTakeScreenshot()
{
	if(!ui->centralWidget->IsEngineInitialized()) return;

	ui->centralWidget->render(&screenshotsPixmap);

	QDateTime now = QDateTime::currentDateTime();
	QString nowStr = now.toString("hh-mm-ss dd.MM.yyyy");
	QString path = MakeScreenhotsFolder();

	screenshotsPixmap.save(path + SCREENSHOT_NAME.arg(nowStr));
}

void MainWindow::OnOpenScreenshotsFolder()
{
	QString path = MakeScreenhotsFolder();
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::closeEvent(QCloseEvent*)
{
	Settings::Save();
}

// Move window to screen center
void MainWindow::MoveToCenter()
{
	QRect screenRect = QApplication::desktop()->availableGeometry();
	QRect wndRect = frameGeometry();
	move((screenRect.width() - wndRect.width()) / 2, (screenRect.height() - wndRect.height()) / 2);
}
