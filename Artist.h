#ifndef ARTIST_H
#define ARTIST_H

#include <QString>
#include <QDate>

struct Artist {
    int      id;
    QString  email;
    QString  password;
    QString  artistName;
    QString  realName;
    QString  country;
    QString  genre;
    QString  bio;
    QString  photoPath;
    QDate    registerDate;
    bool     active;
};

#endif // ARTIST_H
