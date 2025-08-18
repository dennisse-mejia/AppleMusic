// ArtistPerfilPage.h
#ifndef ARTISTPERFILPAGE_H
#define ARTISTPERFILPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>

struct Artist;
class QComboBox;
class QLabel;

class ArtistPerfilPage : public QWidget {
    Q_OBJECT
public:
    explicit ArtistPerfilPage(const Artist &artist, QWidget *parent = nullptr);

    void resetUiToOrig();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void hideEvent(QHideEvent *e) override;

signals:
    void requestTrends();                  // para volver a Trends en PanelArtista
    void profileUpdated(const Artist &a);

private:
    // 6 campos
    QLineEdit     *leEmail;
    QLineEdit     *leArtisticName;
    QLineEdit     *leCountry;
    QLineEdit     *lePassword;
    QLineEdit     *leRealName;
    QLineEdit     *leGenre;
    QComboBox *cbCountry = nullptr;
    QComboBox *cbGenre   = nullptr;
    QLabel    *avatarLbl = nullptr;  // usaremos eventFilter sobre el QLabel existente
    int        m_artistId = 0;

    // bio
    QPlainTextEdit *bioEdit;

    // avatar y fecha (si quieres mantener referencias)
    QLabel        *lblRegDate;

    QPushButton *btnCancel = nullptr;
    QPushButton *btnSave   = nullptr;
    QString      m_photoPath;

    struct Snapshot {
        QString email, artist, pass, real, country, genre, bio, photoPath;
    } m_orig;


};

#endif // ARTISTPERFILPAGE_H
