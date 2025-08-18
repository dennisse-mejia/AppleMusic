//#include "ArtistMusicPage.h"
#include "artistmusicpage.h"
#include "Artist.h"
#include "HoverButton.h"

//#include "AddMusicDialog.h"
#include "addmusicdialog.h"
#include "musicdetailpage.h"
#include "SongRecord.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QWidget>
#include <QScrollBar>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QIcon>
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QHoverEvent>
#include <QDir>
#include <QFile>

#include <QString>
#include <QStringList>
#include <QPixmap>


void ArtistMusicPage::loadSongsFromFile(const Artist& a)
{
    // Busca los layouts por nombre (se los asignamos en addSection)
    auto *layoutSingles = this->findChild<QHBoxLayout*>("layoutSingles");
    auto *layoutEP      = this->findChild<QHBoxLayout*>("layoutEP");
    auto *layoutAlbums  = this->findChild<QHBoxLayout*>("layoutAlbums");

    if (!layoutSingles || !layoutEP || !layoutAlbums) {
        // Si aún no existen, no hacemos nada (constructor los crea antes de llamar a esto)
        return;
    }

    QString dir   = QDir::homePath() + "/AppleMusic";
    QString fileP = dir + "/canciones.dat";
    QFile file(fileP);
    if (!file.open(QIODevice::ReadOnly)) return;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_5);

    auto toQString = [](const QByteArray &ba) {
        int nul = ba.indexOf('\0');
        QByteArray trimmed = (nul >= 0 ? ba.left(nul) : ba);
        return QString::fromUtf8(trimmed);
    };

    while (!in.atEnd()) {
        // Lee EXACTAMENTE el formato que guardas:
        qint32 id = 0, artistId = -1;
        QByteArray titleBA, genreBA, categoryBA, audioBA, coverBA;
        quint32 duration = 0;
        bool active = false;
        qint64 ts = 0;

        in >> id
            >> artistId
            >> titleBA
            >> genreBA
            >> categoryBA
            >> duration
            >> audioBA
            >> coverBA
            >> active
            >> ts;

        // Filtra por artista y solo activos
        if (!active || artistId != a.id) continue;

        const QString title = toQString(titleBA);
        const QString genre = toQString(genreBA);

        const QString tipo  = toQString(categoryBA);   // "Single"/"EP"/"Álbum"
        const QString audio = toQString(audioBA);
        const QString cover = toQString(coverBA);

        // Portada redondeada (mismo look & feel que al agregar)
        QPixmap pix;
        if (!pix.load(cover)) continue;
        pix = pix.scaled(131,131, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap rounded(131,131);
        rounded.fill(Qt::transparent);
        {
            QPainter p(&rounded);
            p.setRenderHint(QPainter::Antialiasing);
            QPainterPath path; path.addRoundedRect(rounded.rect(), 8, 8);
            p.setClipPath(path);
            p.drawPixmap(0,0,pix);
        }
        pix = rounded;

        // Miniatura idéntica a la creada al guardar
        QWidget *itemW = new QWidget;
        auto *itemLay  = new QVBoxLayout(itemW);
        itemLay->setContentsMargins(0,0,0,0);
        itemLay->setSpacing(4);

        auto *thumbBtn = new HoverButton(itemW);
        thumbBtn->setIcon(QIcon(pix));
        thumbBtn->setIconSize(QSize(131,131));
        thumbBtn->setFixedSize(131,131);
        thumbBtn->setStyleSheet("border-radius:8px; background:#ececec; border:none;");
        itemLay->addWidget(thumbBtn, 0, Qt::AlignLeft);

        QStringList trackTitles;
        QStringList trackPaths;

        // 1) Intentar leer pistas del archivo secundario
        {
            const QString tracksPath = QDir::homePath() + "/AppleMusic/canciones_tracks.dat";
            QFile tfile(tracksPath);
            if (tfile.open(QIODevice::ReadOnly)) {
                QDataStream tin(&tfile);
                tin.setVersion(QDataStream::Qt_6_5);

                while (!tin.atEnd()) {
                    qint32 parentId = 0;
                    QByteArray tTitleBA, tPathBA;
                    tin >> parentId >> tTitleBA >> tPathBA;
                    if (parentId == id) {
                        // misma utilidad toQString que usas arriba
                        trackTitles << toQString(tTitleBA);
                        trackPaths  << toQString(tPathBA);
                    }
                }
                tfile.close();
            }
        }

        // 2) Fallback: si no había registros, tratar como single
        if (trackTitles.isEmpty()) {
            trackTitles << title;
            trackPaths  << audio;
        }

        connect(thumbBtn, &QPushButton::clicked, this, [=](){
            emit itemSelected(
                id,          // qint32 id
                tipo,        // type
                title,       // title
                cover,       // coverPath (QString)
                pix,         // cover (QPixmap)
                trackTitles,
                trackPaths,
                genre
                );
        });



        auto *titleLbl = new QLabel(title + " – " + tipo, itemW);
        titleLbl->setStyleSheet("font:9pt Arial; color:#333;");
        itemLay->addWidget(titleLbl);

        auto *yearLbl = new QLabel(QString::number(QDate::currentDate().year()), itemW);
        yearLbl->setStyleSheet("font:8pt Arial; color:#777;");
        itemLay->addWidget(yearLbl);

        const QString tipoNorm = tipo.toLower();
        QHBoxLayout *dest = nullptr;
        if (tipoNorm == "single")      dest = layoutSingles;
        else if (tipoNorm == "ep")     dest = layoutEP;
        else /* álbum / album */       dest = layoutAlbums;

        dest->addWidget(itemW);
    }

    file.close();
}

ArtistMusicPage::ArtistMusicPage(const Artist &a, QWidget *parent)
    : QWidget(parent)
{
    // 1) Layout principal con scroll vertical
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    auto *outerScroll = new QScrollArea(this);
    outerScroll->setWidgetResizable(true);
    outerScroll->setFrameShape(QFrame::NoFrame);
    outerScroll->setStyleSheet(R"(
        QScrollBar:vertical { background: transparent; width: 8px; margin: 0; }
        QScrollBar::handle:vertical { background: #c0c0c0; min-height: 20px; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; height: 0; }
    )");
    mainLay->addWidget(outerScroll);

    // 2) Contenedor interno
    auto *container = new QWidget;
    auto *vlay = new QVBoxLayout(container);
    vlay->setContentsMargins(20, 20, 20, 20);
    vlay->setSpacing(20);

    // ——— Botón “Agregar” ———
    QPushButton *btnAgregar = new QPushButton(tr("+  Agregar"), container);
    btnAgregar->setFixedSize(160, 40);
    btnAgregar->setStyleSheet(R"(
        QPushButton { background-color: #f1f1f1; border: none; border-radius: 20px; color: #ec2339; font: bold 14pt Arial; }
        QPushButton:hover { background-color: #ececec; }
        QPushButton:pressed { background-color: #dcdcdc; }
    )");
    {
        auto *hBtnLay = new QHBoxLayout;
        hBtnLay->setContentsMargins(0, 0, 50, 0);
        hBtnLay->addStretch();
        hBtnLay->addWidget(btnAgregar, 0, Qt::AlignTop);
        vlay->addLayout(hBtnLay);
    }

    // Secciones y layouts para miniaturas
    QHBoxLayout *layoutSingles = nullptr;
    QHBoxLayout *layoutEP      = nullptr;
    QHBoxLayout *layoutAlbums  = nullptr;

    // Helper: crea sección de scroll horizontal
    auto addSection = [&](const QString &sectionName, QHBoxLayout *&outLayout) {
        QLabel *lbl = new QLabel(sectionName, container);
        lbl->setStyleSheet("font: bold 10pt Arial; color: black;");
        vlay->addWidget(lbl, 0, Qt::AlignLeft);

        QScrollArea *scr = new QScrollArea(container);
        scr->setFixedSize(1074, 170);
        scr->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scr->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scr->setWidgetResizable(true);
        scr->setFrameShape(QFrame::NoFrame);
        scr->setStyleSheet(R"(
    QScrollArea { background: transparent; }
    QScrollBar:horizontal { background: transparent; height: 8px; margin: 0; }
    QScrollBar::handle:horizontal { background: transparent; min-width: 20px; border-radius: 4px; }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
    QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; width: 0; height: 0; }
)");

        // 2) Contenido interno del scroll
        QWidget *inner = new QWidget;
        auto *innerLay = new QHBoxLayout(inner);
        innerLay->setContentsMargins(0,0,0,0);
        innerLay->setSpacing(53);
        innerLay->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        inner->setLayout(innerLay);
        scr->setWidget(inner);

        // —— Ponemos nombres para encontrarlos desde loadSongsFromFile ——
        if (sectionName == tr("Singles"))      innerLay->setObjectName("layoutSingles");
        else if (sectionName == tr("EP"))      innerLay->setObjectName("layoutEP");
        else if (sectionName == tr("Albums"))  innerLay->setObjectName("layoutAlbums");

        // 3) Wrapper que contiene flechas + scroll
        auto *wrapper = new QWidget(container);
        auto *wrapperLay = new QHBoxLayout(wrapper);
        wrapperLay->setContentsMargins(0,0,0,0);
        wrapperLay->setSpacing(0);

        // Flecha izquierda (fuera del scroll)
        auto *btnLeft = new QPushButton(wrapper);
        btnLeft->setIcon(QIcon(":/Resources/left_arrow.png"));
        btnLeft->setIconSize(QSize(24,24));
        btnLeft->setFixedSize(30, scr->height());
        btnLeft->setStyleSheet("border:none; background:transparent;");
        wrapperLay->addWidget(btnLeft);

        // El propio scroll (solo covers dentro)
        wrapperLay->addWidget(scr);

        // Flecha derecha (fuera del scroll)
        auto *btnRight = new QPushButton(wrapper);
        btnRight->setIcon(QIcon(":/Resources/right_arrow.png"));
        btnRight->setIconSize(QSize(24,24));
        btnRight->setFixedSize(30, scr->height());
        btnRight->setStyleSheet("border:none; background:transparent;");
        wrapperLay->addWidget(btnRight);

        // 4) Añade el wrapper centrado
        vlay->addWidget(wrapper, 0, Qt::AlignHCenter);

        // 5) Conecta las flechas a la acción estándar de scroll
        auto *bar = scr->horizontalScrollBar();
        btnLeft ->setVisible(false);
        btnRight->setVisible(false);

        connect(bar, &QScrollBar::rangeChanged, this,
                [btnLeft, btnRight, bar](int min, int max){
                    bool hasOverflow = (max > min);
                    btnLeft ->setVisible(hasOverflow);
                    btnRight->setVisible(hasOverflow);
                });

        connect(btnLeft,  &QPushButton::clicked, this, [bar](){
            bar->triggerAction(QAbstractSlider::SliderToMinimum);
        });
        connect(btnRight, &QPushButton::clicked, this, [bar](){
            bar->triggerAction(QAbstractSlider::SliderToMaximum);
        });

        outLayout = innerLay;
    };

    // Crear las secciones
    addSection(tr("Singles"), layoutSingles);
    addSection(tr("EP"),      layoutEP);
    addSection(tr("Albums"),  layoutAlbums);



    // 3) Finaliza el scroll vertical
    outerScroll->setWidget(container);
    loadSongsFromFile(a);

    // Conexión para añadir miniatura (y listener) al guardar canción
    connect(btnAgregar, &QPushButton::clicked, this, [this, layoutSingles, layoutEP, layoutAlbums, artistId = a.id]() {
        AddMusicDialog dlg(this);
        dlg.setWindowTitle(tr("Agregar música"));
        if (dlg.exec() != QDialog::Accepted)
            return;

        // 1) Recoger datos del diálogo
        QString tipo         = dlg.type();
        QString portada      = dlg.coverPath();
        QString titulo       = dlg.title();
        QString genero  = dlg.genre();
        int     year         = QDateTime::currentDateTime().date().year();
        QStringList trackTitles = dlg.trackTitles();
        QStringList trackPaths  = dlg.trackPaths();

        if (tipo.compare("Single", Qt::CaseInsensitive) == 0) {
            trackTitles = { titulo };
            trackPaths  = { dlg.audioPath() };
        }

        // Normaliza para decidir carrusel destino (maneja "Álbum"/"album")
        const QString tipoNorm = tipo.toLower();
        QHBoxLayout *dest = nullptr;
        if (tipoNorm == "single")      dest = layoutSingles;
        else if (tipoNorm == "ep")     dest = layoutEP;
        else /* álbum / album */       dest = layoutAlbums;

        // 2) Cargar y recortar redondeado la portada
        QPixmap pix;
        if (!pix.load(portada))
            return;
        pix = pix.scaled(131, 131, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap rounded(131, 131);
        rounded.fill(Qt::transparent);
        {
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path; path.addRoundedRect(rounded.rect(), 8, 8);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, pix);
        }
        pix = rounded;
        qint32 releaseId = 0;

        // 3) Persistir en disco usando QDataStream
        {
            QString projectRoot = QDir::homePath() + "/AppleMusic";
            QDir().mkpath(projectRoot);
            QString dataPath = projectRoot + "/canciones.dat";

            QFile file(dataPath);
            if (!file.open(QIODevice::Append))
                return;

            QDataStream out(&file);
            out.setVersion(QDataStream::Qt_6_5);

            SongRecord rec;
            rec.artistId   = artistId;
            rec.id         = static_cast<int32_t>(QDateTime::currentSecsSinceEpoch());

            // Limpia buffers antes de copiar (evita basura)
            memset(rec.title,     0, sizeof(rec.title));
            memset(rec.genre,     0, sizeof(rec.genre));
            memset(rec.category,  0, sizeof(rec.category));
            memset(rec.audioPath, 0, sizeof(rec.audioPath));
            memset(rec.coverPath, 0, sizeof(rec.coverPath));

            // Copia datos
            strncpy(rec.title,    titulo.toUtf8().constData(), sizeof(rec.title)-1);
            strncpy(rec.genre,   genero.toUtf8().constData(),  sizeof(rec.genre)-1);     // ← AQUÍ
            strncpy(rec.category, tipo.toUtf8().constData(),   sizeof(rec.category)-1); // guarda la categoría real
            rec.duration   = 0;
            strncpy(rec.audioPath, dlg.audioPath().toUtf8().constData(), sizeof(rec.audioPath)-1);
            strncpy(rec.coverPath, portada.toUtf8().constData(),          sizeof(rec.coverPath)-1);

            rec.active     = true;
            rec.timestamp  = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            // Escribir en el orden y tipo exactos
            out << rec.id
                << rec.artistId
                << QByteArray(rec.title,     sizeof(rec.title))
                << QByteArray(rec.genre,     sizeof(rec.genre))
                << QByteArray(rec.category,  sizeof(rec.category))
                << quint32(rec.duration)
                << QByteArray(rec.audioPath, sizeof(rec.audioPath))
                << QByteArray(rec.coverPath, sizeof(rec.coverPath))
                << rec.active
                << rec.timestamp;
            releaseId = static_cast<qint32>(rec.id);

            file.close();
        }

        // === GUARDAR PISTAS EN canciones_tracks.dat ===
        {
            const QString tracksPath = QDir::homePath() + "/AppleMusic/canciones_tracks.dat";
            QFile tfile(tracksPath);
            if (tfile.open(QIODevice::Append)) {
                QDataStream tout(&tfile);
                tout.setVersion(QDataStream::Qt_6_5);

                auto writeTrack = [&](const QString &tt, const QString &pp){
                    tout << releaseId
                         << QByteArray(tt.toUtf8())
                         << QByteArray(pp.toUtf8());
                };

                if (tipo.compare("Single", Qt::CaseInsensitive) == 0) {
                    writeTrack(titulo, dlg.audioPath());
                } else {
                    for (int i = 0; i < trackTitles.size(); ++i) {
                        writeTrack(trackTitles.at(i),
                                   (i < trackPaths.size() ? trackPaths.at(i) : QString()));
                    }
                }
                tfile.close();
            }
        }



        // 4) Construir el widget de miniatura (igual que arriba)
        QWidget    *itemW    = new QWidget;
        QVBoxLayout *itemLay = new QVBoxLayout(itemW);
        itemLay->setContentsMargins(0,0,0,0);
        itemLay->setSpacing(4);

        auto *thumbBtn = new HoverButton(itemW);
        thumbBtn->setIcon(QIcon(pix));
        thumbBtn->setIconSize(QSize(131,131));
        thumbBtn->setFixedSize(131,131);
        thumbBtn->setStyleSheet("border-radius: 8px; background-color: #ececec; border: none;");
        itemLay->addWidget(thumbBtn, 0, Qt::AlignLeft);

        connect(thumbBtn, &QPushButton::clicked, this, [=](){
            emit itemSelected(
                releaseId,   // qint32 id
                tipo,        // type
                titulo,      // title
                portada,     // coverPath (QString)
                pix,         // cover (QPixmap)
                trackTitles,
                trackPaths,
                genero       // genre
                );
        });





        auto *titleLbl = new QLabel(titulo + " – " + tipo, itemW);
        titleLbl->setAlignment(Qt::AlignLeft);
        titleLbl->setStyleSheet("font:9pt Arial; color:#333;");
        itemLay->addWidget(titleLbl);

        auto *yearLbl = new QLabel(QString::number(year), itemW);
        yearLbl->setAlignment(Qt::AlignLeft);
        yearLbl->setStyleSheet("font:8pt Arial; color:#777;");
        itemLay->addWidget(yearLbl);

        dest->addWidget(itemW);

    });


}


void ArtistMusicPage::reload(const Artist& a) {
    auto clear = [](QHBoxLayout* lay){
        if (!lay) return;
        while (auto *item = lay->takeAt(0)) {
            if (auto *w = item->widget()) { w->deleteLater(); }
            delete item;
        }
    };
    clear(this->findChild<QHBoxLayout*>("layoutSingles"));
    clear(this->findChild<QHBoxLayout*>("layoutEP"));
    clear(this->findChild<QHBoxLayout*>("layoutAlbums"));
    loadSongsFromFile(a);
}


