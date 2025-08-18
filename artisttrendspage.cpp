// ArtistTrendsPage.cpp
#include "artisttrendspage.h"
#include <QVBoxLayout>
#include <QLabel>

ArtistTrendsPage::ArtistTrendsPage(const Artist &a, QWidget *parent)
    : QWidget(parent)
{
    // Solo layout y título
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(20, 20, 20, 0);
    lay->setSpacing(20);

}


