//#include "AddMusicDialog.h"
#include "addmusicdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QScrollArea>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QVBoxLayout>
#include <QDialogButtonBox>  // por si no lo tienes



AddMusicDialog::AddMusicDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Agregar música"));
    setFixedSize(400, 800);

    applyStyles();
    setupUi();
}

QString AddMusicDialog::title() const       { return m_titleEdit->text(); }
QString AddMusicDialog::type() const        { return m_typeBox->currentText(); }
QString AddMusicDialog::coverPath() const   { return m_coverPath; }
QString AddMusicDialog::audioPath() const   { return m_audioPath; }
QString AddMusicDialog::genre() const { return m_genreBox->currentText(); }
QString AddMusicDialog::category() const { return m_categoryBox->currentText(); }



QStringList AddMusicDialog::trackTitles() const {
    QStringList list;
    for (auto &t : m_tracks) list << t.titleEdit->text();
    return list;
}
QStringList AddMusicDialog::trackPaths() const {
    QStringList list;
    for (auto &t : m_tracks) list << t.audioPath;
    return list;
}

void AddMusicDialog::applyStyles() {
    setStyleSheet(R"(
        QDialog {
            background-color: #F9F9F9;
            border-radius: 12px;
        }

        /* Etiquetas con fondo F9F9F9 */
        QLabel {
            font: bold 10pt Arial;
            color: #000;
            background-color: #F9F9F9;
        }

        /* Campos de texto transparentes */
        QLineEdit {
            background: transparent;
            color: #7D8286;
            font: 10pt Arial;
            border: 1px solid #7D8286;
            border-radius: 4px;
            padding-left: 8px;
        }

        /* Estilo completo para QComboBox */
        QComboBox {
            background-color: #F9F9F9;
            color: #7D8286;
            font: 10pt Arial;
            border: 1px solid #7D8286;
            border-radius: 4px;
            padding-left: 8px;
            padding-right: 25px;  /* espacio para la flecha */
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border: none;
            background: transparent;
            border-top-right-radius: 4px;
            border-bottom-right-radius: 4px;
        }
        QComboBox::down-arrow {
            image: url(:/Resources/down_arrow.svg);
            width: 12px;
            height: 12px;
        }
        QComboBox QAbstractItemView {
            background-color: rgba(60, 60, 60, 200);
            color: #000;
            font: 10pt Arial;
            selection-background-color: #fa2c43;
            outline: none;
        }

        /* Botones con fondo F9F9F9 */
        QPushButton {
            background-color: #F9F9F9;
            color: #fa2c43;
            font: 10pt Arial;
            border: 1px solid #fa2c43;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #ededed;
        }
        /* Cancel/Guardar conservan su estilo pero fondo F9F9F9 */
        QPushButton#CancelButton {
            background-color: #F9F9F9;
            color: #fa2c43;
            border: 1px solid #fa2c43;
            border-radius: 4px;
        }
        QPushButton#CancelButton:hover {
            background-color: #ededed;
        }

        /* Botones pequeños de pistas: tamaño exacto (Añadir / Importar) */
        QPushButton#AddTrackButton,
        QPushButton#TrackFolderButton {
            min-width: 135px;
            max-width: 135px;
            min-height: 25px;
            max-height: 25px;
            padding: 2px 8px;  /* evita que el padding suba la altura */
        }


        /* ScrollArea transparente mantiene fondo del dialog */
        QScrollArea {
            background: transparent;
        }
    )");
}



void AddMusicDialog::setupUi() {
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(20,20,20,20);
    mainLay->setSpacing(20);


    // Campos comunes
    m_titleEdit   = new QLineEdit(this);
    m_genreBox    = new QComboBox(this);
    m_categoryBox = new QComboBox(this);
    m_typeBox     = new QComboBox(this);


    // *** FIJAR TAMAÑO DE TODOS LOS FIELDS AQUÍ ***
    const QSize fieldSize(275,35);
    m_titleEdit  ->setFixedSize(fieldSize);
    m_genreBox   ->setFixedSize(fieldSize);
    m_categoryBox->setFixedSize(fieldSize);
    m_typeBox    ->setFixedSize(fieldSize);

    m_coverBtn = new QPushButton(tr("Seleccionar portada"), this);
    m_coverBtn->setObjectName("ActionButton");
    m_coverBtn->setFixedSize(fieldSize);
    connect(m_coverBtn, &QPushButton::clicked, this, &AddMusicDialog::browseCover);

    m_audioBtn = new QPushButton(tr("Seleccionar archivo de audio"), this);
    m_audioBtn->setObjectName("ActionButton");
    m_audioBtn->setFixedSize(fieldSize);
    connect(m_audioBtn, &QPushButton::clicked, this, &AddMusicDialog::browseAudio);

    m_genreBox->addItems({tr("Pop"), tr("Corridos"), tr("Cristianos"),
                          tr("Electrónica"), tr("Reguetón"), tr("Rock"), tr("Clásicas")});
    m_categoryBox->addItems({tr("Playlist"), tr("Recomendado"), tr("Favorito"),
                             tr("Infantil"), tr("Instrumental")});
    m_typeBox->addItems({tr("Single"), tr("EP"), tr("Álbum")});
    m_typeBox->setFixedHeight(35);
    connect(m_typeBox, &QComboBox::currentTextChanged,
            this, &AddMusicDialog::onTypeChanged);

    // Helper para insertar campo + label
    auto addField = [&](const QString &lbl, QWidget *w, bool highlightBackground){
        auto *cont = new QWidget(this);
        if (highlightBackground) {
            cont->setStyleSheet("background-color: #F9F9F9;");  // <<< fondo para los primeros 4
        }
        auto *lay  = new QVBoxLayout(cont);
        lay->setContentsMargins(0,0,0,0);
        lay->setSpacing(14);   // <<< 14px entre label y widget

        auto *label = new QLabel(lbl, cont);
        label->setStyleSheet("background-color: transparent;");
        lay->addWidget(label);
        lay->addWidget(w);
        mainLay->addWidget(cont, /*stretch=*/0, Qt::AlignHCenter);
    };

    // Aplica fondo sólo a los primeros 4:
    addField(tr("Título"),    m_titleEdit,   true);
    addField(tr("Género"),    m_genreBox,    true);
    addField(tr("Categoría"), m_categoryBox, true);
    addField(tr("Tipo"),      m_typeBox,     true);
    // Los demás igual que antes, pero sin fondo extra
    addField(tr("Portada"),        m_coverBtn, true);
    addField(tr("Audio (Single)"), m_audioBtn, true);

    // Contenedor dinámico de pistas
    m_tracksContainer = new QWidget(this);
    m_tracksContainer->setStyleSheet("background: transparent;");

    // Etiqueta Pistas con ancho fijo
    auto *labelTracks = new QLabel(tr("Pistas (EP/Álbum)"), this);
    labelTracks->setFixedWidth(fieldSize.width());
    mainLay->addWidget(labelTracks, /*stretch=*/0, Qt::AlignHCenter);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(m_tracksContainer);
    //scroll->setFixedWidth(fieldSize.width());
    scroll->setFixedWidth(275);
    scroll->setStyleSheet(R"(
    QScrollArea {
        background-color: #f9f9f9;      /* fondo del área de pistas */
        border: 1px solid #DDD;
        border-radius: 6px;
    }
    /* Barra vertical del scroll */
    QScrollBar:vertical {
        background: transparent;     /* deja ver el fondo del area */
        width: 8px;
        margin: 0px;
    }
    /* Oculta las flechas */
    QScrollBar::sub-line:vertical,
    QScrollBar::add-line:vertical {
        height: 0px;
    }
    /* Relleno vacío */
    QScrollBar::sub-page:vertical,
    QScrollBar::add-page:vertical {
        background: none;
    }
    /* “Thumb” que arrastras */
    QScrollBar::handle:vertical {
        background: #C0C0C0;
        min-height: 20px;
        border-radius: 4px;
    }
    /* Hover sobre el handle */
    QScrollBar::handle:vertical:hover {
        background: #A0A0A0;
    }
)");

    mainLay->addWidget(scroll, /*stretch=*/0, Qt::AlignHCenter);

    m_tracksLayout = new QVBoxLayout(m_tracksContainer);
    m_tracksLayout->setContentsMargins(0,0,0,0);
    m_tracksLayout->setSpacing(10);

    // Botón "+ Añadir pista"
    m_addTrackBtn = new QPushButton(tr("+ Añadir pista"), this);
    m_addTrackBtn->setObjectName("AddTrackButton");
    connect(m_addTrackBtn, &QPushButton::clicked, this, &AddMusicDialog::addTrackRow);

    // Botón "Importar carpeta"
    m_folderBtn = new QPushButton(tr("Importar carpeta"), this);
m_folderBtn->setObjectName("TrackFolderButton");
    connect(m_folderBtn, &QPushButton::clicked, this, &AddMusicDialog::browseTracksFolder);

    // === Forzar tamaño EXACTO 180x35 y que NADA lo cambie ===
    m_addTrackBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_folderBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_addTrackBtn->setMinimumSize(135, 25);
    m_addTrackBtn->setMaximumSize(135, 25);
    m_addTrackBtn->setFixedSize(135, 25);

    m_folderBtn->setMinimumSize(135, 25);
    m_folderBtn->setMaximumSize(135, 25);
    m_folderBtn->setFixedSize(135, 25);

    // (Opcional) neutraliza estilos que impongan min-height/padding grandes
    m_addTrackBtn->setStyleSheet(
        "QPushButton#AddTrackButton{min-height:0; padding:4px 10px;}"
        );
    m_folderBtn->setStyleSheet(
        "QPushButton#ActionButton{min-height:0; padding:4px 10px;}"
        );


    // Los dos en una fila
    auto *tracksBtnsRow = new QHBoxLayout;
    tracksBtnsRow->setSpacing(8);
    tracksBtnsRow->addStretch();
    tracksBtnsRow->addWidget(m_addTrackBtn);
    tracksBtnsRow->addWidget(m_folderBtn);
    tracksBtnsRow->addStretch();
    mainLay->addLayout(tracksBtnsRow);

    // Botones Cancel / Guardar
    m_cancelBtn = new QPushButton(tr("Cancelar"), this);
    m_cancelBtn->setObjectName("CancelButton");
    m_cancelBtn->setFixedSize(100,30);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_okBtn = new QPushButton(tr("Guardar"), this);
    m_okBtn->setObjectName("ActionButton");
    m_okBtn->setFixedSize(100,30);

    // Helper para mostrar mensaje con OK grande y texto centrado (sin capturas)
    auto showMsg = [](QWidget *parent, const QString &title, const QString &text){
        QMessageBox box(parent);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle(title);
        box.setText(text);
        box.setStandardButtons(QMessageBox::NoButton); // pondremos nuestro OK
        // Centra el texto del mensaje y agranda OK
        box.setStyleSheet(
            "QLabel#qt_msgbox_label { qproperty-alignment: AlignCenter; }"
            "QPushButton { min-width: 105px; min-height: 24px; "
            "  font: 11pt 'Arial'; border-radius: 6px; }"
            );
        QPushButton *ok = box.addButton(QObject::tr("OK"), QMessageBox::AcceptRole);
        ok->setMinimumWidth(105);
        ok->setMinimumHeight(24);
        box.exec();
    };


    connect(m_okBtn, &QPushButton::clicked, this, [this, showMsg](){
        const QString titulo    = m_titleEdit->text().trimmed();
        const QString genero    = m_genreBox->currentText().trimmed();
        const QString categoria = m_categoryBox->currentText().trimmed();
        const QString tipo      = m_typeBox->currentText().trimmed();

        // 1) Todos vacíos → un solo mensaje
        if (titulo.isEmpty() && genero.isEmpty() && categoria.isEmpty() && tipo.isEmpty() && m_coverPath.isEmpty()) {
            showMsg(this, tr("Campos incompletos"), tr("Todos los campos son obligatorios"));
            return;
        }

        // 2) Campos obligatorios
        if (titulo.isEmpty())      { showMsg(this, tr("Campos incompletos"), tr("El título es obligatorio")); return; }
        if (genero.isEmpty())      { showMsg(this, tr("Campos incompletos"), tr("El género es obligatorio")); return; }
        if (categoria.isEmpty())   { showMsg(this, tr("Campos incompletos"), tr("La categoría es obligatoria")); return; }
        if (tipo.isEmpty())        { showMsg(this, tr("Campos incompletos"), tr("El tipo es obligatorio")); return; }
        if (m_coverPath.isEmpty()) { showMsg(this, tr("Campos incompletos"), tr("La portada es obligatoria")); return; }

        // 3) Validación por tipo
        if (tipo.compare(tr("Single"), Qt::CaseInsensitive) == 0) {
            // Single: audio obligatorio
            if (m_audioPath.isEmpty()) {
                showMsg(this, tr("Campos incompletos"), tr("Debes seleccionar el archivo de audio del Single"));
                return;
            }
            emit songAdded(title(), type(), coverPath(), audioPath());
            accept();
            return;
        }

        // 4) EP/Álbum: todas las pistas deben estar completas (título + audio)
        // 4a) Detectar filas incompletas (tienen algo pero no todo)
        for (const auto &t : m_tracks) {
            const bool tieneTitulo = !t.titleEdit->text().trimmed().isEmpty();
            const bool tieneAudio  = !t.audioPath.trimmed().isEmpty();
            if (tieneTitulo ^ tieneAudio) {
                showMsg(this, tr("Pistas incompletas"),
                        tr("Cada pista debe tener título y archivo de audio. Completa o elimina las pistas incompletas."));
                return;
            }
        }

        // 4b) Reunir pistas completas
        QStringList titulos; titulos.reserve(m_tracks.size());
        QStringList paths;   paths.reserve(m_tracks.size());
        for (const auto &t : m_tracks) {
            const QString tt = t.titleEdit->text().trimmed();
            const QString ap = t.audioPath.trimmed();
            if (!tt.isEmpty() && !ap.isEmpty()) {
                titulos << tt;
                paths   << ap;
            }
        }

        if (titulos.isEmpty()) {
            showMsg(this, tr("Faltan pistas"),
                    tr("Para EP/Álbum debes añadir al menos 2 pistas (cada una con título y archivo de audio)."));
            return;
        }
        if (titulos.size() == 1) {
            showMsg(this, tr("Solo 1 pista"),
                    tr("Has agregado solo 1 pista. Cambia el tipo a 'Single' si deseas subir una sola pista."));
            return; // no guardamos ni cambiamos el tipo automáticamente
        }

        // 5) OK: guardar EP/Álbum
        emit albumAdded(title(), type(), coverPath(), titulos, paths);
        accept();
    });



    auto *hl = new QHBoxLayout;
    hl->addStretch();
    hl->addWidget(m_cancelBtn);
    hl->addWidget(m_okBtn);
    hl->addStretch();
    mainLay->addLayout(hl);

    // Estado inicial
    onTypeChanged(m_typeBox->currentText());

}

// void AddMusicDialog::onTypeChanged(const QString &tipo) {
//     bool single = (tipo == tr("Single"));

//     // En vez de setVisible, solo habilitamos o deshabilitamos
//     m_audioBtn->setEnabled(single);

//     if (!single) {
//         // Limpiar el estado del audio de Single
//         m_audioPath.clear();
//         m_audioBtn->setText(tr("Seleccionar archivo de audio"));
//     }


//     // Para las pistas mantenemos la lógica de visibilidad
//     m_addTrackBtn->setVisible(!single);
//     m_tracksContainer->setVisible(!single);

//     // Si volvemos a Single, limpiamos las pistas existentes
//     if (single) {
//         QLayoutItem *child;
//         while ((child = m_tracksLayout->takeAt(0)) != nullptr) {
//             if (auto w = child->widget()) w->deleteLater();
//             delete child;
//         }
//         m_tracks.clear();
//     }
// }

void AddMusicDialog::onTypeChanged(const QString &tipo) {
    const bool single = (tipo == tr("Single"));

    // Habilita/inhabilita el botón de audio de Single
    m_audioBtn->setEnabled(single);

    if (single) {
        // Vamos de EP/Álbum → Single
        // Si había exactamente 1 pista con audio, úsala para el campo de Single
        int countWithAudio = 0;
        QString pickedAudio;
        for (const auto &t : m_tracks) {
            if (!t.audioPath.isEmpty()) {
                ++countWithAudio;
                pickedAudio = t.audioPath;
            }
        }

        if (countWithAudio == 1) {
            m_audioPath = pickedAudio;
            m_audioBtn->setText(tr("Audio cargado correctamente"));
        } else {
            // No hay o hay más de una: deja el field de Single limpio
            m_audioPath.clear();
            m_audioBtn->setText(tr("Seleccionar archivo de audio"));
        }

        // Limpia las pistas del modo EP/Álbum
        QLayoutItem *child;
        while ((child = m_tracksLayout->takeAt(0)) != nullptr) {
            if (auto w = child->widget()) w->deleteLater();
            delete child;
        }
        m_tracks.clear();

    } else {
        // Vamos de Single → EP/Álbum: limpia el audio de Single (obligará a subir pistas)
        m_audioPath.clear();
        m_audioBtn->setText(tr("Seleccionar archivo de audio"));
    }

    // Visibilidad de controles de pistas
    m_addTrackBtn->setVisible(!single);
    m_folderBtn->setVisible(!single);
    m_tracksContainer->setVisible(!single);
}




void AddMusicDialog::browseCover() {
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Seleccionar portada"), {}, tr("Imágenes (*.png *.jpg *.jpeg)"));
    if (file.isEmpty()) return;
    m_coverPath = file;
    m_coverBtn->setText(tr("Portada cargada"));
}

void AddMusicDialog::browseAudio() {
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Seleccionar archivo de audio"), {}, tr("Audio (*.mp3 *.wav *.ogg)"));
    if (file.isEmpty()) return;
    m_audioPath = file;
    m_audioBtn->setText(tr("Audio cargado correctamente"));
}

void AddMusicDialog::addTrackRow() {
    const QSize titleSize(136, 28);
    const QSize audioBtnSize(80, 28);
    const QSize removeSize(28, 28);

    QWidget *row = new QWidget(m_tracksContainer);
    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0,0,0,0);
    hl->setSpacing(8);

    auto *le = new QLineEdit(row);
    le->setPlaceholderText(tr("Título de pista"));
    le->setFixedSize(titleSize);

    auto *btn = new QPushButton(tr("Agregar audio"), row);
    btn->setObjectName("ActionButton");
    btn->setFixedSize(audioBtnSize);
    connect(btn, &QPushButton::clicked, this, &AddMusicDialog::browseTrackAudio);

    auto *rm = new QPushButton("×", row);
    rm->setObjectName("CancelButton");
    rm->setFixedSize(removeSize);
    connect(rm, &QPushButton::clicked, row, [=](){
        row->deleteLater();
        m_tracks.erase(std::remove_if(
                           m_tracks.begin(), m_tracks.end(),
                           [&](const Track &t){ return t.audioBtn == btn; }),
                       m_tracks.end());
    });

    hl->addWidget(le);
    hl->addWidget(btn);
    hl->addWidget(rm);
    m_tracksLayout->addWidget(row);
    m_tracks.append({le, btn, QString()});
}


void AddMusicDialog::browseTrackAudio() {
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Seleccionar archivo de audio"), {}, tr("Audio (*.mp3 *.wav *.ogg)"));
    if (file.isEmpty()) return;
    for (auto &t : m_tracks) {
        if (t.audioBtn == btn) {
            t.audioPath = file;
            btn->setText(QFileInfo(file).fileName());
            break;
        }
    }
}

void AddMusicDialog::preloadForEdit(const QString &title,
                                    const QString &genre,
                                    const QString &category,
                                    const QString &type,
                                    const QString &coverPath,
                                    const QStringList &trackTitles,
                                    const QStringList &trackPaths,
                                    const QString &singleAudioPath)
{
    m_titleEdit->setText(title);
    m_genreBox->setCurrentText(genre);
    m_categoryBox->setCurrentText(category);
    m_typeBox->setCurrentText(type);

    // portada
    m_coverPath = coverPath;
    m_coverBtn->setText(QFileInfo(coverPath).fileName());

    // limpiar estado dinámico
    QLayoutItem *child;
    while ((child = m_tracksLayout->takeAt(0)) != nullptr) {
        if (auto w = child->widget()) w->deleteLater();
        delete child;
    }
    m_tracks.clear();

    if (type.compare("Single", Qt::CaseInsensitive)==0) {
        m_audioPath = singleAudioPath;
        m_audioBtn->setText(QFileInfo(singleAudioPath).fileName());
    } else {
        onTypeChanged(type); // muestra sección de pistas
        for (int i=0;i<trackTitles.size();++i) {
            addTrackRow();
            m_tracks[i].titleEdit->setText(trackTitles.at(i));
            const QString p = (i<trackPaths.size()? trackPaths.at(i) : QString());
            m_tracks[i].audioPath = p;
            if (!p.isEmpty()) m_tracks[i].audioBtn->setText(QFileInfo(p).fileName());
        }
    }
}

void AddMusicDialog::browseTracksFolder() {
    // Elegir carpeta
    const QString dirPath = QFileDialog::getExistingDirectory(
        this, tr("Seleccionar carpeta con canciones")
        );
    if (dirPath.isEmpty()) return;

    // Extensiones soportadas
    QDir dir(dirPath);
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.ogg" << "*.flac" << "*.m4a";

    // Orden por nombre (natural para números si tu Qt lo respeta con IgnoreCase)
    const QFileInfoList files = dir.entryInfoList(
        filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase
        );
    if (files.isEmpty()) {
        QMessageBox::information(this, tr("Sin audios"),
                                 tr("No se encontraron archivos de audio en la carpeta."));
        return;
    }

    // Si ya hay filas, pregunta si deseas limpiar o agregar al final
    // if (!m_tracks.isEmpty()) {
    //     const auto r = QMessageBox::question(
    //         this, tr("Agregar pistas"),
    //         tr("Ya tienes pistas cargadas.\n\n¿Quieres reemplazarlas por las de la carpeta?\n"
    //            "Si eliges 'No', se agregarán al final."),
    //         QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    //         );
    //     if (r == QMessageBox::Yes) {
    //         // Limpia UI y vector
    //         QLayoutItem *child;
    //         while ((child = m_tracksLayout->takeAt(0)) != nullptr) {
    //             if (auto w = child->widget()) w->deleteLater();
    //             delete child;
    //         }
    //         m_tracks.clear();
    //     }
    // }

    // Si ya hay filas, pregunta si deseas limpiar o agregar al final
    if (!m_tracks.isEmpty()) {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("Agregar pistas"));
        box.setText(tr("Ya tienes pistas cargadas.\n\n"
                       "¿Quieres reemplazarlas por las de la carpeta?\n"
                       "Si eliges 'No', se agregarán al final."));
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton(QMessageBox::No);

        // --- Botones más altos (y ancho uniforme opcional) ---
        QAbstractButton *yesBtn = box.button(QMessageBox::Yes);
        QAbstractButton *noBtn  = box.button(QMessageBox::No);

        if (yesBtn) {
            yesBtn->setMinimumHeight(32);   // alto
            yesBtn->setMinimumWidth(105);   // opcional: ancho
            yesBtn->setStyleSheet("font: 11pt 'Arial'; border-radius: 6px;");
        }
        if (noBtn) {
            noBtn->setMinimumHeight(32);
            noBtn->setMinimumWidth(105);
            noBtn->setStyleSheet("font: 11pt 'Arial'; border-radius: 6px;");
        }

        // --- Centrar botones ---
        if (auto bb = box.findChild<QDialogButtonBox*>()) {
            bb->setCenterButtons(true);
        }

        int r = box.exec();
        if (r == QMessageBox::Yes) {
            // Limpia UI y vector
            QLayoutItem *child;
            while ((child = m_tracksLayout->takeAt(0)) != nullptr) {
                if (auto w = child->widget()) w->deleteLater();
                delete child;
            }
            m_tracks.clear();
        }
        // Si elige "No" o cierra con X/Esc, se continúa sin reemplazar
    }


    // Crear una fila por cada archivo y precargar título y path
    for (const QFileInfo &fi : files) {
        addTrackRow();
        Track &t = m_tracks.last();
        // Título desde el nombre de archivo (sin extensión)
        t.titleEdit->setText(fi.completeBaseName());
        // Ruta completa al audio
        t.audioPath = fi.absoluteFilePath();
        // Reflejar en el botón el nombre del archivo
        if (t.audioBtn) t.audioBtn->setText(fi.fileName());
    }

    // Si el tipo estuviera en "Single" por error, fuerza visibilidad correcta
    const bool single = (m_typeBox->currentText() == tr("Single"));
    m_addTrackBtn->setVisible(!single);
    if (m_folderBtn) m_folderBtn->setVisible(!single);
    m_tracksContainer->setVisible(!single);
}





