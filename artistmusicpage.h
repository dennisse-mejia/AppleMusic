#ifndef ARTISTMUSICPAGE_H
#define ARTISTMUSICPAGE_H

#include <QWidget>

class QHBoxLayout;
class QPushButton;
class Artist;

class ArtistMusicPage : public QWidget {
    Q_OBJECT

public:
    explicit ArtistMusicPage(const Artist &a, QWidget *parent = nullptr);
    void reload(const Artist& a);


private:
    QHBoxLayout *layoutSingles;
    QHBoxLayout *layoutEP;
    QHBoxLayout *layoutAlbums;
    void loadSongsFromFile(const Artist &a);


signals:
    void itemSelected(qint32 id,
                      const QString &type,
                      const QString &title,
                      const QString &coverPath,
                      const QPixmap  &cover,
                      const QStringList &trackTitles,
                      const QStringList &trackPaths,
                      const QString &genre);




};

#endif // ARTISTMUSICPAGE_H
