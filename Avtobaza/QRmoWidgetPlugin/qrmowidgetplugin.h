#ifndef QRMOWIDGETPLUGIN_H
#define QRMOWIDGETPLUGIN_H

#include <QDesignerCustomWidgetInterface>

//==========================================================================
//
// Проект Автобаза. Коллекция виджетов, встраиваемых в QtDesigner
//
//==========================================================================


//======================================================================================
//  QFIndicatorWidget
//======================================================================================
class QFIndicatorInterface : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    QFIndicatorInterface(QObject *parent = 0);
    ~QFIndicatorInterface();

    bool isContainer() const;
    bool isInitialized() const;
    QIcon icon() const;
    QString domXml() const;
    QString group() const;
    QString includeFile() const;
    QString name() const;
    QString toolTip() const;
    QString whatsThis() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);

private:
    bool initialized;
};

//======================================================================================
//  QConsoleWidget
//======================================================================================
class QConsoleInterface : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    QConsoleInterface(QObject *parent = 0);
    ~QConsoleInterface();

    bool isContainer() const;
    bool isInitialized() const;
    QIcon icon() const;
    QString domXml() const;
    QString group() const;
    QString includeFile() const;
    QString name() const;
    QString toolTip() const;
    QString whatsThis() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);

private:
    bool initialized;
};

//======================================================================================
//  QLocalControlQMLWidget
//======================================================================================
class QLocalControlQMLInterface : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    QLocalControlQMLInterface(QObject *parent = 0);
    ~QLocalControlQMLInterface();

    bool isContainer() const;
    bool isInitialized() const;
    QIcon icon() const;
    QString domXml() const;
    QString group() const;
    QString includeFile() const;
    QString name() const;
    QString toolTip() const;
    QString whatsThis() const;
    QWidget *createWidget(QWidget *parent);
    void initialize(QDesignerFormEditorInterface *core);

private:
    bool initialized;
};

//===================================================================================
//    QRmoWidgetPlugin - collection of RMO plugins
//===================================================================================
class QRmoWidgetPlugin: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    QRmoWidgetPlugin(QObject * parent = 0);
    QList<QDesignerCustomWidgetInterface *> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface *> widgets;
};

#endif // QRMOWIDGETPLUGIN_H



