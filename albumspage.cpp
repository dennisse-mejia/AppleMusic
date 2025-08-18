// AlbumsPage.cpp
#include "albumspage.h"
#include "favorites_store.h"
#include "HoverButton.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QFrame>


static constexpr int CARD = 150;   // 150x150 cover
static constexpr int GAP  = 34;    // separación entre columnas
static constexpr int COLS = 5;     // fijo: 5 por fila

// Redondear el QPixmap a 10px
static QPixmap rounded10(const QPixmap& src){
    if (src.isNull()) return src;
    QPixmap dest(src.size());
    dest.fill(Qt::transparent);
    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path; path.addRoundedRect(src.rect(), 10, 10);
    p.setClipPath(path);
    p.drawPixmap(0, 0, src);
    return dest;
}

AlbumsPage::AlbumsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 0);
    root->setSpacing(12);

    auto *label = new QLabel(tr("Álbumes"), this);
    label->setStyleSheet("font: bold 30pt Arial; color: black;");
    root->addWidget(label, 0, Qt::AlignTop | Qt::AlignLeft);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_body = new QWidget; // sin padre; lo gestiona el scroll
    m_body->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    scroll->setWidget(m_body);

    // El contenedor scrolleable es el que se añade al layout raíz
    root->addWidget(scroll, 1);

}

void AlbumsPage::setFavoritesStore(FavoritesStore* s){
    m_store = s;
    rebuild();
    // repintar cuando cambien favoritos
    connect(m_store, &FavoritesStore::countChanged, this, [this](int){ rebuild(); });
}

QWidget* AlbumsPage::makeCard(const FavEntry& e, QWidget* parent){
    auto *card = new QWidget(parent);
    card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto *v = new QVBoxLayout(card);
    v->setContentsMargins(0,0,0,0);
    v->setSpacing(6);

    // cover 150x150
    QPixmap pm;
    if (!e.cover.isEmpty()) pm.load(e.cover);
    if (pm.isNull())        pm.load(":/Resources/note.png");
    pm = pm.scaled(CARD, CARD, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    pm = rounded10(pm);

    auto *btn = new HoverButton(card);     // ← hover
    btn->setFixedSize(CARD, CARD);
    btn->setIcon(QIcon(pm));
    btn->setIconSize(QSize(CARD, CARD));
    btn->setCursor(Qt::PointingHandCursor);
    // el icono ya viene redondeado; mantenemos el fondo transparente
    btn->setStyleSheet(
        "QPushButton{border:none; background:transparent; border-radius:10px;}"
        "QPushButton:hover{background:transparent;}"
        );
    v->addWidget(btn, 0, Qt::AlignLeft);
    v->addSpacing(10);

    // NUEVO: al click, avisar qué álbum/cover/artista fue clicado
    QObject::connect(btn, &QToolButton::clicked, this, [=](){
        emit albumClicked(e.album, e.artist, e.cover, e.title);
    });

    // título (single/EP/álbum)
   auto *ttl = new QLabel(e.album.toUpper(), card);
   ttl->setFixedWidth(CARD);
   ttl->setWordWrap(false);
   ttl->setStyleSheet("font: 11pt Arial; color: #111;");
   v->addWidget(ttl, 0, Qt::AlignLeft);

    // artista
    auto *art = new QLabel(e.artist, card);
    art->setFixedWidth(CARD);
    art->setWordWrap(false);
    art->setStyleSheet("font: 10pt Arial; color: #777;");
    v->addWidget(art, 0, Qt::AlignLeft);

    return card;
}

void AlbumsPage::rebuild(){
    if (!m_body) return;

    // limpiar grid anterior
    if (auto *old = m_body->layout()) {
        QLayoutItem *it;
        while ((it = old->takeAt(0))) {
            if (auto *w = it->widget()) w->deleteLater();
            delete it;
        }
        delete old;
    }

    // grid fijo
    auto *grid = new QGridLayout(m_body);
    grid->setContentsMargins(20, 12, 20, 24);
    grid->setHorizontalSpacing(GAP);
    grid->setVerticalSpacing(40);
    m_body->setLayout(grid);

    // leer favoritos
    const auto items = m_store ? m_store->list() : QVector<FavEntry>{};

    // ---- AGRUPAR POR (album + artist) para evitar duplicados ----
    QVector<FavEntry> unique;
    unique.reserve(items.size());
    QSet<QString> seen;

    for (const auto &e : items) {
        const QString albumNorm  = e.album.simplified().toLower();
        const QString artistNorm = e.artist.simplified().toLower();

        // fallback: si no hubiese album, agrupa por título (single)
        const QString base = albumNorm.isEmpty()
                                 ? e.title.simplified().toLower()
                                 : albumNorm;

        const QString key = base + '|' + artistNorm;
        if (seen.contains(key)) continue;

        seen.insert(key);
        unique.push_back(e);  // conserva el primer cover y el orden visual
    }

    // pintar únicos
    for (int i = 0; i < unique.size(); ++i){
        int r = i / COLS;
        int c = i % COLS;
        grid->addWidget(makeCard(unique[i], m_body), r, c,
                        Qt::AlignLeft | Qt::AlignTop);
    }
}

