/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QMainWindow>
#include <QDesktopWidget>

#include "modelwindow.h"
#include "controlwidget.h"
#include "modelsimcore.h"

int main(int argc, char **argv)
{
    //Q_INIT_RESOURCE(ffans);

    QApplication app(argc, argv);

    // Model window
    // ------------
    ModelWindow modelWindow;
    int desktop_w = QApplication::desktop()->width();
    int desktop_h = QApplication::desktop()->height();

    modelWindow.resize(modelWindow.sizeHint() * 0.8);
    int desktopArea = desktop_w * desktop_h;
    int widgetArea = modelWindow.width() * modelWindow.height();
    if (((float)widgetArea / (float)desktopArea) < 0.75f)
        modelWindow.show();
    else
    {
        modelWindow.resize(desktop_w * 0.8, desktop_h * 0.8);
        modelWindow.show();
        //modelWindow.showMaximized();
    }
    // Model Data Analysis window
    // --------------
    /*ControlWidget controlWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    controlWidget.setStyle(arthurStyle);

    QList<QWidget *> widgets = controlWidget.findChildren<QWidget *>();
    foreach (QWidget *w, widgets) {
        w->setStyle(arthurStyle);
        w->setAttribute(Qt::WA_AcceptTouchEvents);
    }

    controlWidget.show();*/

    // Simultion thread controller init
    // --------------------------------
    SimController* simController = new SimController;
    simController->operate("stub");

    return app.exec();
}
