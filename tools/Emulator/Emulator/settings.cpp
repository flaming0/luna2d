//-----------------------------------------------------------------------------
// luna2d Emulator
// This is part of luna2d engine
// Copyright 2014-2015 Stepan Prokofjev
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

#include "settings.h"
#include <QSettings>

// Default values of settings
QStringList Settings::recentGames = QStringList();
bool Settings::openLastGame = true;
bool Settings::gameWasOpened = false;
QVector<Resolution> Settings::resolutions = QVector<Resolution>();
int Settings::curResolution = 0;
int Settings::bullitinCount = 0;
bool Settings::openLogWhenError = false;
bool Settings::showFps = true;

void Settings::Load()
{
	QSettings settings(ORGANIZATION_NAME, APP_NAME);

	openLastGame = settings.value("openLastGame", openLastGame).toBool();
	gameWasOpened = settings.value("gameWasOpened", gameWasOpened).toBool();
	curResolution = settings.value("curResolution", curResolution).toInt();
	openLogWhenError = settings.value("openLogWhenError", openLogWhenError).toBool();
	showFps = settings.value("showFps", showFps).toBool();

	// Load recent games
	int recentGamesCount = settings.beginReadArray("recentGames");
	for(int i = 0; i < recentGamesCount; i++)
	{
		settings.setArrayIndex(i);
		recentGames.push_back(settings.value("path").toString());
	}
	settings.endArray();

	// Add bullit-in resolutions
	resolutions.push_back(Resolution(480, 320, "HVGA", true));
	resolutions.push_back(Resolution(800, 480, "WVGA", true));
	resolutions.push_back(Resolution(1280, 720, "HD", true));
	resolutions.push_back(Resolution(1024, 768, "iPad", true));
	resolutions.push_back(Resolution(1920, 1080, "FullHD", true));
	resolutions.push_back(Resolution(2048, 1536, "Retina iPad", true));
	bullitinCount = resolutions.size();

	// Load user-defined resolutions
	int resolutionsCount = settings.beginReadArray("resolutions");
	for(int i = 0; i < resolutionsCount; i++)
	{
		settings.setArrayIndex(i);

		int width = settings.value("width").toInt();
		int height = settings.value("height").toInt();
		QString name = settings.value("name").toString();

		resolutions.push_back(Resolution(width, height, name));
	}
	settings.endArray();
}

void Settings::Save()
{
	QSettings settings(ORGANIZATION_NAME, APP_NAME);

	settings.setValue("openLastGame", openLastGame);
	settings.setValue("gameWasOpened", gameWasOpened);
	settings.setValue("curResolution", curResolution);
	settings.setValue("openLogWhenError", openLogWhenError);
	settings.setValue("showFps", showFps);

	// Save recent games
	settings.beginWriteArray("recentGames");
	for(int i = 0; i < recentGames.size(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("path", recentGames.at(i));
	}
	settings.endArray();

	// Save resolutions
	settings.beginWriteArray("resolutions");
	for(int i = bullitinCount; i < resolutions.size(); i++)
	{
		settings.setArrayIndex(i - bullitinCount);
		settings.setValue("width", resolutions.at(i).width);
		settings.setValue("height", resolutions.at(i).height);
		settings.setValue("name", resolutions.at(i).name);
	}
	settings.endArray();
}

void Settings::AddRecentGame(const QString& gamePath)
{
	int index = recentGames.indexOf(gamePath);

	// If given game already exists, move it to top,
	// else just add
	if(index != -1) recentGames.move(index, 0);
	else
	{
		recentGames.push_front(gamePath);

		// Limit max games count
		if(recentGames.size() > MAX_RECENT_GAMES) recentGames.removeLast();
	}
}
