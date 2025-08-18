// ArtistPerfilPage.cpp
#include "artistperfilpage.h"
#include "Artist.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QWidget>
#include <QComboBox>
#include <QFileDialog>
#include <QEvent>
#include <QPixmap>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QDate>
#include <QHideEvent>
#include <QStyleFactory>

static void showMsg(QWidget *parent, QMessageBox::Icon icon,
                    const QString &title, const QString &text) {
    QMessageBox box(parent);
    box.setIcon(icon);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::NoButton); // pondremos nuestro OK

    // (Opcional pero útil en macOS) fuerza un estilo Qt para respetar el stylesheet
    // comenta esta línea si no quieres cambiar el estilo del box:
    box.setStyle(QStyleFactory::create("Fusion"));

    // Estilo: mismo look que tus botones del diálogo (#F9F9F9 / #fa2c43)
    box.setStyleSheet(
        "QMessageBox {"
        "  background-color: #ffffff;"
        "}"
        "QLabel#qt_msgbox_label {"
        "  qproperty-alignment: AlignCenter;"
        "  color: #000;"
        "  font: bold 10pt 'Arial';"
        "}"
        "QPushButton {"
        "  background-color: #F9F9F9;"
        "  color: #fa2c43;"
        "  font: 10pt 'Arial';"
        "  border: 1px solid #fa2c43;"
        "  border-radius: 4px;"
        "  min-width: 105px;"
        "  min-height: 24px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #ededed;"
        "}"
        );

    QPushButton *ok = box.addButton(QObject::tr("OK"), QMessageBox::AcceptRole);
    ok->setMinimumWidth(105);
    ok->setMinimumHeight(24);

    box.exec();
}

static inline void showWarn(QWidget *p, const QString &t, const QString &x) {
    showMsg(p, QMessageBox::Warning, t, x);
}
static inline void showError(QWidget *p, const QString &t, const QString &x) {
    showMsg(p, QMessageBox::Critical, t, x);
}
static inline void showInfo(QWidget *p, const QString &t, const QString &x) {
    showMsg(p, QMessageBox::Information, t, x);
}


ArtistPerfilPage::ArtistPerfilPage(const Artist &a, QWidget *parent)
    : QWidget(parent)
{
    // 1) Layout principal con scroll
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *container = new QWidget;
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(30);


    // --- Avatar centrado + Fecha de Registro debajo ---
    // --- Avatar centrado + Fecha de Registro debajo ---
    {
        auto *topLay = new QVBoxLayout;
        topLay->setSpacing(8);
        topLay->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

        m_artistId = a.id;  // guarda el id para eventFilter/persistencia

        // CREA avatarLbl (no uses un QLabel local distinto)
        // CREA avatarLbl (no uses un QLabel local distinto)
        avatarLbl = new QLabel(container);
        QPixmap pic(a.photoPath.isEmpty()
                        ? ":/Resources/profilepicbig.png"
                        : a.photoPath);
        avatarLbl->setPixmap(pic.scaled(129, 125, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        avatarLbl->setFixedSize(129, 125);
        avatarLbl->setObjectName("profilePic");
        avatarLbl->setCursor(Qt::PointingHandCursor);
        avatarLbl->installEventFilter(this);
        topLay->addWidget(avatarLbl, 0, Qt::AlignHCenter);


        QLabel *lblRegTitle = new QLabel(tr("Fecha de Registro"), container);
        lblRegTitle->setStyleSheet("font: bold 12pt Arial; color: black;");
        topLay->addWidget(lblRegTitle, 0, Qt::AlignHCenter);

        QLabel *lblRegDate = new QLabel(a.registerDate.toString(Qt::ISODate), container);
        lblRegDate->setStyleSheet("font: 12pt Arial; color: black;");
        topLay->addWidget(lblRegDate, 0, Qt::AlignHCenter);

        lay->addLayout(topLay);
    }


    // --- Grid de 6 campos (3 columnas × 2 filas) ---
    {
        QWidget *fieldsWidget = new QWidget(container);
        auto *grid = new QGridLayout(fieldsWidget);
        grid->setHorizontalSpacing(40);
        grid->setVerticalSpacing(20);

        const QString labels[6] = {
            tr("Email o username"), tr("Nombre Artístico"), tr("País de Origen"),
            tr("Contraseña"),        tr("Nombre Real"),       tr("Género Musical")
        };

        // Reusar miembros
        leEmail        = new QLineEdit(fieldsWidget);
        leArtisticName = new QLineEdit(fieldsWidget);

        lePassword     = new QLineEdit(fieldsWidget);
        leRealName     = new QLineEdit(fieldsWidget);

        cbCountry = new QComboBox(fieldsWidget);
        cbCountry->addItems({
                             "Afganistán", "Alemania", "Argentina", "Australia", "Bolivia", "Brasil", "Canadá", "Chile", "China", "Colombia", "Costa Rica",
                             "Cuba", "Ecuador", "Egipto", "El Salvador", "España", "Estados Unidos", "Francia", "Guatemala", "Honduras", "India", "Italia",
                             "Japón", "México", "Nicaragua", "Panamá", "Paraguay", "Perú", "Portugal", "Reino Unido", "República Dominicana", "Rusia",
                             "Uruguay", "Venezuela"
        });
        cbCountry->setCurrentText(a.country);

        cbGenre = new QComboBox(fieldsWidget);
        cbGenre->addItems({ tr("Pop"), tr("Corridos"), tr("Cristianos"),
                           tr("Electrónica"), tr("Reguetón"), tr("Rock"), tr("Clásicas") });
        cbGenre->setCurrentText(a.genre);


        QWidget* edits[6] = {
            leEmail, leArtisticName, cbCountry,
            lePassword, leRealName, cbGenre
        };

        for (int i = 0; i < 6; ++i) {
            QLabel *lbl = new QLabel(labels[i], fieldsWidget);
            lbl->setStyleSheet("font: bold 12pt Arial; color: black;");

            edits[i]->setFixedSize(270, 45);

            // ⬇ aplica estilo según el tipo de widget
            if (auto le = qobject_cast<QLineEdit*>(edits[i])) {
                le->setStyleSheet(
                    "QLineEdit {"
                    " background: white;"
                    " color: #7d8286;"
                    " font: 12pt Arial;"
                    " border:1px solid #7d8286;"
                    " border-radius:4px;"
                    " padding-left:8px;"
                    "}"
                    );
            } else if (auto cb = qobject_cast<QComboBox*>(edits[i])) {
                cb->setStyleSheet(R"(
            QComboBox {
                background: white;
                color: #7d8286;
                font: 12pt Arial;
                border: 1px solid #7d8286;
                border-radius: 4px;
                padding-left: 8px;
                padding-right: 36px; /* espacio para la flecha */
            }
            QComboBox::drop-down {
                subcontrol-origin: padding;
                subcontrol-position: top right;
                width: 36px;
                border: none;                 /* evita “cuadro” a la derecha */
                background: transparent;      /* conserva el borde redondeado */
                border-top-right-radius: 4px;
                border-bottom-right-radius: 4px;
            }
            QComboBox::down-arrow {
                image: url(:/Resources/down_arrow.svg); /* tu PNG */
                width: 14px;
                height: 14px;
                margin-right: 10px;
            }
            QComboBox QAbstractItemView {
                background: white;
                color: #000;
                font: 10pt Arial;
                selection-background-color: #fa2c43;
                selection-color: #fff;
                outline: none;
                border: 1px solid #ddd;
            }
        )");
            }

            int row = (i / 3) * 2;
            int col = i % 3;
            grid->addWidget(lbl, row, col);
            grid->addWidget(edits[i], row + 1, col);
        }

        lay->addSpacing(20);
        lay->addWidget(fieldsWidget);
    }

    // --- Biografía centrada debajo del grid ---
    {
        auto *bioContainer = new QWidget(container);
        auto *bioLay = new QVBoxLayout(bioContainer);
        bioLay->setSpacing(8);
        bioLay->setContentsMargins(0, 0, 0, 0);
        bioLay->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

        QLabel *bioTitle = new QLabel(tr("Biografía"), bioContainer);
        bioTitle->setStyleSheet("font: bold 12pt Arial; color: black;");
        bioLay->addWidget(bioTitle, 0, Qt::AlignHCenter);

        bioEdit = new QPlainTextEdit(bioContainer);
        bioEdit->setPlainText(a.bio);
        bioEdit->setFixedSize(394, 100);
        bioEdit->setStyleSheet(
            "QPlainTextEdit {"
            " border: 1px solid #7d8286;"
            " border-radius: 4px;"
            " padding: 4px;"
            " font: 12pt Arial;"
            " color: #7d8286; "
            "}"
            );
        bioLay->addWidget(bioEdit, 0, Qt::AlignHCenter);

        lay->addWidget(bioContainer);
    }

    // --- Botones Cancelar / Guardar centrados ---
    {
        auto *btnLay = new QHBoxLayout;
        btnLay->setSpacing(30);
        btnLay->addStretch();

        auto makeBtn = [&](const QString &txt) {
            auto *b = new QPushButton(txt, container);
            b->setFixedSize(172, 45);
            b->setStyleSheet(
                "QPushButton {"
                " border:2px solid #ec2339;"
                " color:#ec2339;"
                " font: 12pt Arial;"
                " border-radius:4px;"
                " background:transparent;"
                "}"
                "QPushButton:hover { background: #fcecec; }"
                );
            return b;
        };

        btnCancel = makeBtn(tr("Cancelar"));
        btnCancel->setObjectName("perfilBtnCancel");
        btnSave   = makeBtn(tr("Guardar"));

        btnLay->addWidget(btnCancel);
        btnLay->addWidget(btnSave);
        btnLay->addStretch();
        lay->addLayout(btnLay);

        connect(btnCancel, &QPushButton::clicked, this, [this](){
            resetUiToOrig();
            emit requestTrends();
        });

        // ⬇⬇⬇ AQUÍ pega tu connect(btnSave, ...) ⬇⬇⬇
        connect(btnSave, &QPushButton::clicked, this, [this](){
            // 1) Recoger valores UI
            const QString email   = leEmail->text().trimmed();
            const QString artist  = leArtisticName->text().trimmed();
            const QString pass    = lePassword->text();
            const QString real    = leRealName->text().trimmed();
            const QString country = cbCountry ? cbCountry->currentText().trimmed() : QString();
            const QString genre   = cbGenre   ? cbGenre  ->currentText().trimmed() : QString();
            const QString bio     = bioEdit->toPlainText().trimmed();

            const QString root = QDir::homePath() + "/AppleMusic";

            // 2.1) Valida contra usuarios.dat
            {
                QFile ufile(root + "/usuarios.dat");
                if (ufile.open(QIODevice::ReadOnly)) {
                    QDataStream in(&ufile);
                    while (!in.atEnd()) {
                        int uid; QString uUser,uPwd,uNameReal,uMail,uFoto;
                        int y1,m1,d1,y2,m2,d2; bool uActive;
                        in >> uid >> uUser >> uPwd >> uNameReal >> uMail
                            >> y1 >> m1 >> d1 >> y2 >> m2 >> d2
                            >> uFoto >> uActive;
                        if (uUser.compare(email, Qt::CaseInsensitive)==0 ||
                            uMail.compare(email, Qt::CaseInsensitive)==0) {
                            showWarn(this, tr("Usuario duplicado"),
                                     tr("Ese email/usuario ya está registrado como usuario"));
                            resetUiToOrig();

                            ufile.close();
                            return;
                        }
                    }
                    ufile.close();
                }
            }

            // 2.2) Valida contra artistas.dat (excepto mi propio id)
            struct Row { int id; QString e,p,a,r,c,g,b,fp; int y,m,d; bool active; };
            QList<Row> rows;
            {
                QFile file(root + "/artistas.dat");
                if (file.open(QIODevice::ReadOnly)) {
                    QDataStream in(&file);
                    while (!in.atEnd()) {
                        Row r;
                        in >> r.id >> r.e >> r.p >> r.a >> r.r >> r.c >> r.g >> r.b >> r.fp
                            >> r.y >> r.m >> r.d >> r.active;
                        rows.push_back(r);
                    }
                    file.close();
                }
            }
            for (const Row &r : rows) {
                if (r.id == m_artistId) continue;
                if (r.e.compare(email,  Qt::CaseInsensitive)==0) {
                    showWarn(this, tr("Email duplicado"),
                             tr("Ese email/usuario ya está registrado"));
                    return;
                }
                if (r.a.compare(artist, Qt::CaseInsensitive)==0) {
                    showWarn(this, tr("Nombre artístico duplicado"),
                             tr("Ese nombre artístico ya está en uso"));
                    resetUiToOrig();

                    return;
                }
            }

            // 3) Actualiza mi fila y reescribe artistas.dat
            for (Row &r : rows) {
                if (r.id == m_artistId) {
                    r.e  = email;
                    r.p  = pass;
                    r.a  = artist;
                    r.r  = real;
                    r.c  = country;
                    r.g  = genre;
                    r.b  = bio;
                    if (!m_photoPath.isEmpty()) r.fp = m_photoPath;
                    break;
                }
            }
            {
                QFile file(root + "/artistas.dat");
                if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    showError(this, tr("Error"),
                              tr("No se pudo abrir artistas.dat para escribir"));
                    return;
                }
                QDataStream out(&file);
                for (const Row &r : std::as_const(rows)) {
                    out << r.id << r.e << r.p << r.a << r.r << r.c << r.g << r.b << r.fp
                        << r.y << r.m << r.d << r.active;
                }
                file.close();
            }

            // 4) Emitir datos actualizados y volver a Trends
            Artist updated;
            updated.id           = m_artistId;
            updated.email        = email;
            updated.password     = pass;
            updated.artistName   = artist;
            updated.realName     = real;
            updated.country      = country;
            updated.genre        = genre;
            updated.bio          = bio;
            updated.photoPath    = m_photoPath;
            // conservar fecha del archivo (si quieres exactitud, guarda la tuya en Artist)
            if (!rows.isEmpty()) {
                auto &r0 = rows.first();
                updated.registerDate = QDate(r0.y, r0.m, r0.d);
            }

            // ←← INSERTA AQUÍ: refrescar el snapshot para futuros resets
            m_orig.email     = leEmail->text();
            m_orig.artist    = leArtisticName->text();
            m_orig.pass      = lePassword->text();
            m_orig.real      = leRealName->text();
            m_orig.country   = cbCountry ? cbCountry->currentText() : QString();
            m_orig.genre     = cbGenre   ? cbGenre  ->currentText() : QString();
            m_orig.bio       = bioEdit->toPlainText();
            m_orig.photoPath = m_photoPath;

            emit profileUpdated(updated);
            showInfo(this, tr("Perfil"), tr("Perfil actualizado exitosamente"));
                    });
    }

    // Ponemos el contenido en el scroll
    scroll->setWidget(container);
    // Estilizamos la barra vertical
    QScrollBar *vbar = scroll->verticalScrollBar();
    vbar->setStyleSheet(R"(
    QScrollBar:vertical {
        background: transparent;
        width: 8px;
        margin: 0px;
    }
    QScrollBar::handle:vertical {
        background: #c0c0c0;
        min-height: 20px;
        border-radius: 4px;
    }
    QScrollBar::handle:vertical:hover {
        background: #a0a0a0;
    }
    QScrollBar::add-line, QScrollBar::sub-line {
        height: 0px;
    }
    QScrollBar::add-page, QScrollBar::sub-page {
        background: transparent;
    }
    )");

    mainLay->addWidget(scroll);

    // ——— ¡Por fin poblamos! ———
    leEmail        ->setText(a.email);
    leArtisticName ->setText(a.artistName);
    lePassword     ->setText(a.password);
    leRealName     ->setText(a.realName);
    if (cbCountry) cbCountry->setCurrentText(a.country);
    if (cbGenre)   cbGenre->setCurrentText(a.genre);
    bioEdit        ->setPlainText(a.bio);
    m_photoPath = a.photoPath;

    // <<< PÉGALO AQUÍ: snapshot del estado original >>>
    m_orig.email     = leEmail->text();
    m_orig.artist    = leArtisticName->text();
    m_orig.pass      = lePassword->text();
    m_orig.real      = leRealName->text();
    m_orig.country   = cbCountry ? cbCountry->currentText() : QString();
    m_orig.genre     = cbGenre   ? cbGenre  ->currentText() : QString();
    m_orig.bio       = bioEdit->toPlainText();
    m_orig.photoPath = m_photoPath;

}

bool ArtistPerfilPage::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == avatarLbl && ev->type() == QEvent::MouseButtonRelease) {
        const QString fn = QFileDialog::getOpenFileName(
            this, tr("Selecciona tu foto"),
            QString(), tr("Imágenes (*.png *.jpg *.jpeg)"));
        if (fn.isEmpty()) return true;

        QPixmap p(fn);
        if (p.isNull()) return true;

        avatarLbl->setPixmap(p.scaled(129,125, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_photoPath = fn;

        // Persistir en artistas.dat (ajusta si ya tienes helper)
        // Formato coincide con tu registro (id,email,pass,artist,real,country,genre,bio,foto,fecha,active)
        QString path = QDir::homePath() + "/AppleMusic/artistas.dat";
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return true;

        struct Row {
            int id; QString email, pass, artist, real, country, genre, bio, foto;
            int y,m,d; bool active;
        };
        QList<Row> rows;
        QDataStream in(&f);
        while (!in.atEnd()) {
            Row r;
            in >> r.id >> r.email >> r.pass >> r.artist >> r.real
                >> r.country >> r.genre >> r.bio >> r.foto
                >> r.y >> r.m >> r.d >> r.active;
            rows.push_back(r);
        }
        f.close();

        for (auto &r : rows) if (r.id == m_artistId) { r.foto = fn; break; }

        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QDataStream out(&f);
            for (const auto &r : rows) {
                out << r.id << r.email << r.pass << r.artist << r.real
                    << r.country << r.genre << r.bio << r.foto
                    << r.y << r.m << r.d << r.active;
            }
            f.close();
        }
        return true; // ya consumimos el evento
    }
    return QWidget::eventFilter(obj, ev);
}

void ArtistPerfilPage::resetUiToOrig()
{
    leEmail->setText(m_orig.email);
    leArtisticName->setText(m_orig.artist);
    lePassword->setText(m_orig.pass);
    leRealName->setText(m_orig.real);
    if (cbCountry) cbCountry->setCurrentText(m_orig.country);
    if (cbGenre)   cbGenre  ->setCurrentText(m_orig.genre);
    bioEdit->setPlainText(m_orig.bio);
    // imagen:
    if (!m_orig.photoPath.isEmpty()) {
        QPixmap p(m_orig.photoPath);
        if (!p.isNull()) avatarLbl->setPixmap(p.scaled(129,125, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void ArtistPerfilPage::hideEvent(QHideEvent *e)
{
    // Si sales de esta página (cambio de pestaña/vista), restaura el snapshot
    resetUiToOrig();
    QWidget::hideEvent(e);
}



