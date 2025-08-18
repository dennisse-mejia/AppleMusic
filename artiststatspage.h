// ArtistStatsPage.h
#ifndef ARTISTSTATSPAGE_H
#define ARTISTSTATSPAGE_H

#include <QWidget>

struct Artist;

class ArtistStatsPage : public QWidget {
    Q_OBJECT
public:
    explicit ArtistStatsPage(const Artist &artist, QWidget *parent = nullptr);
};

#endif // ARTISTSTATSPAGE_H  // :contentReference[oaicite:0]{index=0}
