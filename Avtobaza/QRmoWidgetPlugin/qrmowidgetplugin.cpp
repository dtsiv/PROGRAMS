#include "qrmowidgetplugin.h"

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>

#ifdef __linux
    #include "qavtctrl_linux.h"
    #include "codograms_linux.h"
#else
    #include "qavtctrl.h"
    #include "codograms.h"
#endif

#include "qlocalcontrolqmlwidget.h"
#include "qfindicatorwidget.h"
#include "qconsolewidget.h"


//================================================================================
//  QRmoWidgetPlugin
//================================================================================
QRmoWidgetPlugin::QRmoWidgetPlugin(QObject * parent) : QObject(parent) {
    widgets.append(new QLocalControlQMLInterface(this));
    widgets.append(new QFIndicatorInterface(this));
    widgets.append(new QConsoleInterface(this));
}

QList<QDesignerCustomWidgetInterface *> QRmoWidgetPlugin::customWidgets() const {
    return(widgets);
}

//================================================================================
//  QConsoleInterface
//================================================================================
QConsoleInterface::QConsoleInterface(QObject *parent)
    : QObject(parent) {
    initialized = false;
}

QConsoleInterface::~QConsoleInterface() {
}

void QConsoleInterface::initialize(QDesignerFormEditorInterface * /* core */) {
    if (initialized)
        return;

    initialized = true;
}

bool QConsoleInterface::isInitialized() const {
    return initialized;
}

QWidget *QConsoleInterface::createWidget(QWidget *parent) {
    return new QConsoleWidget(parent);
}

QString QConsoleInterface::name() const {
    return "QConsoleWidget";
}

QString QConsoleInterface::group() const {
    return "RMO Plugins";
}

QIcon QConsoleInterface::icon() const {
    return QIcon();
}

QString QConsoleInterface::toolTip() const {
    return "Console";
}

QString QConsoleInterface::whatsThis() const {
    return "Console";
}

bool QConsoleInterface::isContainer() const {
    return false;
}

QString QConsoleInterface::domXml() const {
    return "<ui language=\"c++\">\n"
           " <widget class=\"QConsoleWidget\" name=\"qConsoleWidget\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>100</width>\n"
           "    <height>100</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string>Console</string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string>Console</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString QConsoleInterface::includeFile() const {
    return "qconsolewidget.h";
}

//================================================================================
//  QFIndicatorInterface
//================================================================================
QFIndicatorInterface::QFIndicatorInterface(QObject *parent)
    : QObject(parent) {
    initialized = false;
}

QFIndicatorInterface::~QFIndicatorInterface() {
}

void QFIndicatorInterface::initialize(QDesignerFormEditorInterface * /* core */) {
    if (initialized)
        return;

    initialized = true;
}

bool QFIndicatorInterface::isInitialized() const {
    return initialized;
}

QWidget *QFIndicatorInterface::createWidget(QWidget *parent) {
    return new QFIndicatorWidget(parent);
}

QString QFIndicatorInterface::name() const {
    return "QFIndicatorWidget";
}

QString QFIndicatorInterface::group() const {
    return "RMO Plugins";
}

QIcon QFIndicatorInterface::icon() const {
    return QIcon();
}

QString QFIndicatorInterface::toolTip() const {
    return "F-Indicator";
}

QString QFIndicatorInterface::whatsThis() const {
    return "F-Indicator";
}

bool QFIndicatorInterface::isContainer() const {
    return false;
}

QString QFIndicatorInterface::domXml() const {
    return "<ui language=\"c++\">\n"
           " <widget class=\"QFIndicatorWidget\" name=\"qFIndicatorWidget\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>100</width>\n"
           "    <height>100</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string>F-Indicator</string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string>F-Indicator</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString QFIndicatorInterface::includeFile() const {
    return "qfindicatorwidget.h";
}

//================================================================================
//  QLocalControlQMLInterface
//================================================================================
QLocalControlQMLInterface::QLocalControlQMLInterface(QObject *parent)
    : QObject(parent) {
    initialized = false;
}

QLocalControlQMLInterface::~QLocalControlQMLInterface() {
}

void QLocalControlQMLInterface::initialize(QDesignerFormEditorInterface * /* core */) {
    if (initialized)
        return;

    initialized = true;
}

bool QLocalControlQMLInterface::isInitialized() const {
    return initialized;
}

QWidget *QLocalControlQMLInterface::createWidget(QWidget *parent) {
    return new QLocalControlQMLWidget(parent);
}

QString QLocalControlQMLInterface::name() const {
    return "QLocalControlQMLWidget";
}

QString QLocalControlQMLInterface::group() const {
    return "RMO Plugins";
}

QIcon QLocalControlQMLInterface::icon() const {
    return QIcon();
}

QString QLocalControlQMLInterface::toolTip() const {
    return "RMO local control QML panel";
}

QString QLocalControlQMLInterface::whatsThis() const {
    return "RMO local control QML panel";
}

bool QLocalControlQMLInterface::isContainer() const {
    return false;
}

QString QLocalControlQMLInterface::domXml() const {
    return "<ui language=\"c++\">\n"
           " <widget class=\"QLocalControlQMLWidget\" name=\"qLocalControlQMLWidget\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>100</width>\n"
           "    <height>100</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string>QML-based control panel</string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string>Modification of scan programs parameters.</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString QLocalControlQMLInterface::includeFile() const {
    return "qlocalcontrolqmlwidget.h";
}

Q_EXPORT_PLUGIN2(qrmowidgetplugin, QRmoWidgetPlugin)


