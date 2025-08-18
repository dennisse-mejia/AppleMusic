#ifndef ALBUMSPAGE_H
#define ALBUMSPAGE_H

#include <QWidget>

class FavoritesStore;
struct FavEntry;

class AlbumsPage : public QWidget {
    Q_OBJECT
public:
    explicit AlbumsPage(QWidget *parent = nullptr);
    void setFavoritesStore(FavoritesStore* s);
signals:
    void albumClicked(const QString& album,
                      const QString& artist,
                      const QString& coverPath,
                      const QString& titleForSingles);

private:
    FavoritesStore* m_store = nullptr;
    QWidget*        m_body  = nullptr;

    QWidget* makeCard(const FavEntry& e, QWidget* parent);
    void rebuild();
};

#endif // ALBUMSPAGE_H
