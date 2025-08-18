// AppWindow.h
#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include "login.h"
#include "registro.h"
#include "registrousuarionormal.h"
#include "registrousuarionormal2.h"
#include "registroartista.h"
#include "registroartista2.h"
#include "panelusuario.h"
#include "User.h"
#include "Artist.h"
#include "panelartista.h"


class AppWindow : public QWidget {
    Q_OBJECT

public:
    explicit AppWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack;
    Login            *loginPage;
    Registro         *registroPage;
    UsuarioNormal    *userPage1;
    UsuarioNormal2   *userPage2;
    RegistroArtista  *artistPage1;
    RegistroArtista2 *artistPage2;
    PanelUsuario  *panelUsuario = nullptr;
    PanelArtista *panelArtista = nullptr;

private slots:
    void onShowLogin();
    void onShowRegistro();
    void onShowUser1();
    void onShowUser2();
    void onShowArtist1();
    void onShowArtist2();
    void onShowUserPanel(const User &user);
    void onShowArtistPanel(const Artist &artist);
};

#endif // APPWINDOW_H
