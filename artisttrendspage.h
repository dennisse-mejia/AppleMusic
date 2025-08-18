#ifndef ARTISTTRENDSPAGE_H
#define ARTISTTRENDSPAGE_H

#include <QWidget>
struct Artist;

class ArtistTrendsPage : public QWidget {
    Q_OBJECT
public:
    explicit ArtistTrendsPage(const Artist &artist, QWidget *parent = nullptr);
};

#endif // ARTISTTRENDSPAGE_H
