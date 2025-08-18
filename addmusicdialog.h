#ifndef ADDMUSICDIALOG_H
#define ADDMUSICDIALOG_H

#include <QDialog>
#include <QStringList>

class QLineEdit;
class QComboBox;
class QPushButton;
class QWidget;
class QVBoxLayout;

struct Track {
    QLineEdit   *titleEdit;
    QPushButton *audioBtn;
    QString      audioPath;
};

class AddMusicDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddMusicDialog(QWidget *parent = nullptr);

    void preloadForEdit(const QString &title,
                        const QString &genre,
                        const QString &category,   // por ahora decorativo
                        const QString &type,
                        const QString &coverPath,
                        const QStringList &trackTitles,
                        const QStringList &trackPaths,
                        const QString &singleAudioPath);

    // Single
    QString title()     const;
    QString type()      const;
    QString coverPath() const;
    QString audioPath() const;
    QString genre() const;
    QString category() const;



    // EP / Álbum
    QStringList trackTitles() const;
    QStringList trackPaths()  const;

signals:
    void songAdded(const QString &title,
                   const QString &type,
                   const QString &coverPath,
                   const QString &audioPath);
    void albumAdded(const QString &albumTitle,
                    const QString &type,
                    const QString &coverPath,
                    const QStringList &titles,
                    const QStringList &paths);

private slots:
    void browseCover();
    void browseAudio();
    void onTypeChanged(const QString &tipo);
    void addTrackRow();
    void browseTrackAudio();
    void browseTracksFolder();

private:
    void setupUi();
    void applyStyles();

    // Campos comunes
    QLineEdit   *m_titleEdit;
    QComboBox   *m_genreBox;
    QComboBox   *m_categoryBox;
    QComboBox   *m_typeBox;
    QPushButton *m_coverBtn;
    QString      m_coverPath;

    // Single
    QPushButton *m_audioBtn;
    QString      m_audioPath;

    // EP / Álbum
    QWidget     *m_tracksContainer;
    QVBoxLayout *m_tracksLayout;
    QPushButton *m_addTrackBtn;
    QList<Track> m_tracks;
    QPushButton *m_folderBtn;

    // Botones de acción
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
};

#endif // ADDMUSICDIALOG_H
