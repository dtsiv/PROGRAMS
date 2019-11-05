#ifndef QTARGETMARKER_H
#define QTARGETMARKER_H

#include <QtCore>
#include <QtGlobal>
#include <QObject>

class QFormular;

class QTargetMarker : public QObject
{
    Q_OBJECT
public:
    explicit QTargetMarker(double dX, double dY, QString qsMesg = QString(), QObject *parent = 0);
    ~QTargetMarker();
    double x();
    double y();
    bool hasFormular();
    void setFormular(QFormular *pFormular);
    QString &mesgString();

signals:

public slots:
    void onFormularClosed(QObject*);

private:
    QFormular *m_pFormular;
    // physical coordinates of target
    double m_dTarDPhys; // physical coordinate D (m) of target
    double m_dTarVPhys; // physical coordinate V (m/s) of target
    QString m_qsMesg;
};

#endif // QTARGETMARKER_H
