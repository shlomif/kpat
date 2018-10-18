#include <QTest>
#include <QDate>

class testDate: public QObject
{
    Q_OBJECT
private slots:
    void testValidity();
    void testMonth();
};

void testDate::testValidity()
{
    // 11 March 1967
    QDate date( 1967, 3, 11 );
    QVERIFY( date.isValid() );
}

void testDate::testMonth()
{
    // 11 March 1967
    QDate date;
    date.setDate( 1967, 3, 11 );
    QCOMPARE( date.month(), 3 );
    QCOMPARE( QDate::longMonthName(date.month()),
              QString("March") );
}


QTEST_MAIN(testDate)
#include "tutorial1.moc"
